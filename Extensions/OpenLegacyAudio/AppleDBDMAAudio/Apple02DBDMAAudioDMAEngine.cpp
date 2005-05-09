#define DEBUGTIMESTAMPS		FALSE

#include "Apple02DBDMAAudioDMAEngine.h"
#include "Apple02Audio.h"

#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/audio/IOAudioDebug.h>

#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/IOWorkLoop.h>

#include "AudioHardwareUtilities.h"
#include "AppleiSubEngine.h"

#pragma mark ------------------------ 
#pragma mark еее Constants
#pragma mark ------------------------ 

#ifdef _TIME_CLIP_ROUTINE
#define kCallFrequency 10
#endif 

#define super IOAudioEngine

OSDefineMetaClassAndStructors(Apple02DBDMAAudioDMAEngine, super)

const int Apple02DBDMAAudioDMAEngine::kDBDMADeviceIndex	= 0;
const int Apple02DBDMAAudioDMAEngine::kDBDMAOutputIndex	= 1;
const int Apple02DBDMAAudioDMAEngine::kDBDMAInputIndex	= 2;

#pragma mark ------------------------ 
#pragma mark еее IOAudioEngine Methods
#pragma mark ------------------------ 

bool Apple02DBDMAAudioDMAEngine::filterInterrupt(int index)
{
	// check to see if this interupt is because the DMA went bad
    UInt32 resultOut = IOGetDBDMAChannelStatus (ioBaseDMAOutput);
    UInt32 resultIn = 1;

	if (ioBaseDMAInput) {
		resultIn = IOGetDBDMAChannelStatus (ioBaseDMAInput) & kdbdmaActive;
	}

    if (!(resultOut & kdbdmaActive) || !resultIn) {
		mNeedToRestartDMA = TRUE;
	}

	// test the takeTimeStamp :it will increment the fCurrentLoopCount and time stamp it with the time now
	takeTimeStamp ();

    return false;
}

void Apple02DBDMAAudioDMAEngine::free()
{
    if (interruptEventSource) {
        interruptEventSource->release();
        interruptEventSource = 0;
    }
    
	if (NULL != dmaCommandBufferInMemDescriptor) {
		dmaCommandBufferInMemDescriptor->release ();
		dmaCommandBufferInMemDescriptor = NULL;
	}
	if (NULL != dmaCommandBufferOutMemDescriptor) {
		dmaCommandBufferOutMemDescriptor->release ();
		dmaCommandBufferOutMemDescriptor = NULL;
	}
	if (NULL != sampleBufferInMemDescriptor) {
		sampleBufferInMemDescriptor->release ();
		sampleBufferInMemDescriptor = NULL;
	}
	if (NULL != sampleBufferOutMemDescriptor) {
		sampleBufferOutMemDescriptor->release ();
		sampleBufferOutMemDescriptor = NULL;
	}
	if (NULL != stopCommandMemDescriptor) {
		stopCommandMemDescriptor->release ();
		stopCommandMemDescriptor = NULL;
	}

    if (dmaCommandBufferOut && (commandBufferSize > 0)) {
        IOFreeAligned(dmaCommandBufferOut, commandBufferSize);
        dmaCommandBufferOut = 0;
    }
    
    if (dmaCommandBufferIn && (commandBufferSize > 0)) {
        IOFreeAligned(dmaCommandBufferIn, commandBufferSize);
        dmaCommandBufferOut = 0;
    }
    super::free();
}

UInt32 Apple02DBDMAAudioDMAEngine::getCurrentSampleFrame()
{
	UInt32 currentBlock = 0;
	vm_offset_t currentDMACommand = (vm_offset_t) dmaCommandBufferOut;
	IOByteCount bufferLength = dmaCommandBufferOutMemDescriptor->getLength ();

	if (ioBaseDMAOutput) {
		IOPhysicalAddress currentDMACommandPhys = IOGetDBDMAChannelRegister (ioBaseDMAOutput, commandPtrLo);

		IOByteCount offsetVirt = 0;
		IOByteCount segmentLength = 0;

		while (offsetVirt < bufferLength) {
			IOPhysicalAddress segmentPhys = dmaCommandBufferOutMemDescriptor->getPhysicalSegment (offsetVirt, &segmentLength); 
			if (currentDMACommandPhys >= segmentPhys) {
				IOByteCount offsetPhys = currentDMACommandPhys - segmentPhys;
				if (offsetPhys < segmentLength) {
					currentDMACommand += offsetVirt + offsetPhys;
					break;
				}
			}
			offsetVirt += segmentLength;
		}

        if ((UInt32)currentDMACommand > (UInt32)dmaCommandBufferOut) {
            currentBlock = ((UInt32)currentDMACommand - (UInt32)dmaCommandBufferOut) / sizeof(IODBDMADescriptor);
        }
    }

    return currentBlock * blockSize / 4;	// 4 bytes per frame - 2 per sample * 2 channels - BIG HACK
}

bool Apple02DBDMAAudioDMAEngine::init(OSDictionary	*properties,
                                 IOService 			*theDeviceProvider,
                                 bool				hasInput,
                                 UInt32				nBlocks,
                                 UInt32				bSize,
                                 UInt32				rate,
                                 UInt16				bitDepth,
                                 UInt16				numChannels)
{
	IOMemoryMap *map;
	Boolean					result;

	debugIOLog (3, "+ Apple02DBDMAAudioDMAEngine::init");
	result = FALSE;

	// Ususal check
	FailIf (FALSE == super::init (properties), Exit);
	FailIf (NULL == theDeviceProvider, Exit);

	// create the memory places for DMA
	map = theDeviceProvider->mapDeviceMemoryWithIndex(Apple02DBDMAAudioDMAEngine::kDBDMAOutputIndex);
	FailIf (NULL == map, Exit);
	ioBaseDMAOutput = (IODBDMAChannelRegisters *) map->getVirtualAddress();

	if(hasInput) {
		map = theDeviceProvider->mapDeviceMemoryWithIndex(Apple02DBDMAAudioDMAEngine::kDBDMAInputIndex);
		FailIf (NULL == map, Exit);
		ioBaseDMAInput = (IODBDMAChannelRegisters *) map->getVirtualAddress();
	} else {
		ioBaseDMAInput = 0;
	}

	dmaCommandBufferIn = 0;
	dmaCommandBufferOut = 0;
	commandBufferSize = 0;
	interruptEventSource = 0;

	numBlocks = nBlocks;
	blockSize = bSize;
	setSampleOffset(kMinimumLatency);
	setNumSampleFramesPerBuffer(numBlocks * blockSize / sizeof (float));

	mInputDualMonoMode = e_Mode_Disabled;		   
		   
	
	mUseSoftwareInputGain = false;	
	mInputGainLPtr = NULL;	
	mInputGainRPtr = NULL;	

#ifdef _TIME_CLIP_ROUTINE
	mCallCount = 0;
	mPreviousUptime.hi = 0;
	mPreviousUptime.lo = 0;
#endif

	result = TRUE;

Exit:
	debugIOLog (3, "- Apple02DBDMAAudioDMAEngine::init");    
	return result;
}

bool Apple02DBDMAAudioDMAEngine::initHardware(IOService *provider)
{
	vm_offset_t					offset;
	vm_offset_t					sampleBufOut;
	vm_offset_t					sampleBufIn;
	IOPhysicalAddress			commandBufferPhys;
	IOPhysicalAddress			sampleBufferPhys;
	IOPhysicalAddress			stopCommandPhys;
    UInt32						blockNum;
	UInt32						dmaCommand = 0;
    Boolean						doInterrupt = false;
    UInt32						interruptIndex;
    IOWorkLoop *				workLoop;
    IOAudioStream *				stream;
	Boolean						result;
	IOAudioSampleRate initialSampleRate;

	result = FALSE;
	sampleBufIn = NULL;
    IOAudioStreamFormat format = {
            2,
            kIOAudioStreamSampleFormatLinearPCM,
            kIOAudioStreamNumericRepresentationSignedInt,
            16,
            16,
            kIOAudioStreamAlignmentHighByte,
            kIOAudioStreamByteOrderBigEndian,
            true,
			0
		};
	
	//	rbm 7.15.2002 keep a copy for user client
	dbdmaFormat.fNumChannels = format.fNumChannels;
	dbdmaFormat.fSampleFormat = format.fSampleFormat;
	dbdmaFormat.fNumericRepresentation = format.fNumericRepresentation;
	dbdmaFormat.fBitDepth = format.fBitDepth;
	dbdmaFormat.fBitWidth = format.fBitWidth;
	dbdmaFormat.fAlignment = format.fAlignment;
	dbdmaFormat.fByteOrder = format.fByteOrder;
	dbdmaFormat.fIsMixable = format.fIsMixable;
	dbdmaFormat.fDriverTag = format.fDriverTag;

    debugIOLog (3, "+ Apple02DBDMAAudioDMAEngine::initHardware()");
    
    ourProvider = provider;
	mNeedToRestartDMA = FALSE;

    FailIf (!super::initHardware(provider), Exit);
        
	// allocate the memory for the buffer
    sampleBufOut = (vm_offset_t)IOMallocAligned(numBlocks * blockSize, PAGE_SIZE);
    if(ioBaseDMAInput)
        sampleBufIn = (vm_offset_t)IOMallocAligned(numBlocks * blockSize, PAGE_SIZE);
    
	initialSampleRate.whole = DBDMAAUDIODMAENGINE_DEFAULT_SAMPLE_RATE;
	initialSampleRate.fraction = 0;

	setSampleRate(&initialSampleRate);
 
	// create the streams
    stream = new IOAudioStream;
    if (stream) {
        const IOAudioSampleRate *rate;        
        rate = getSampleRate();
        
        stream->initWithAudioEngine(this, kIOAudioStreamDirectionOutput, 1, 0, 0);
        stream->setSampleBuffer((void *)sampleBufOut, numBlocks * blockSize);
        stream->addAvailableFormat(&format, rate, rate);
        stream->setFormat(&format);

		// [3306295]
		format.fIsMixable = false;
        stream->addAvailableFormat(&format, rate, rate);
		format.fIsMixable = true;

        addAudioStream(stream);
        stream->release();
    }
    
    if(ioBaseDMAInput) {
        stream = new IOAudioStream;
        if (stream) {
            const IOAudioSampleRate *rate;        
            rate = getSampleRate();
        
            stream->initWithAudioEngine(this, kIOAudioStreamDirectionInput, 1, 0, 0);
            stream->setSampleBuffer((void *)sampleBufIn, numBlocks * blockSize);
            stream->addAvailableFormat(&format, rate, rate);
            stream->setFormat(&format);

            addAudioStream(stream);
            stream->release();
        }
    }

    FailIf (!status || !sampleBufOut, Exit);
    if(ioBaseDMAInput) 
        FailIf (!sampleBufIn, Exit);

	// create the DMA output part
    commandBufferSize = (numBlocks + 1) * sizeof(IODBDMADescriptor);
    dmaCommandBufferOut = (IODBDMADescriptor *)IOMallocAligned(commandBufferSize, 32); 
                                                            // needs to be more than 4 byte aligned
    FailIf (!dmaCommandBufferOut, Exit);

	dmaCommandBufferOutMemDescriptor = IOMemoryDescriptor::withAddress (dmaCommandBufferOut, commandBufferSize, kIODirectionOut);
	FailIf (NULL == dmaCommandBufferOutMemDescriptor, Exit);
	sampleBufferOutMemDescriptor = IOMemoryDescriptor::withAddress ((void *)sampleBufOut, numBlocks * blockSize, kIODirectionOut);
	FailIf (NULL == sampleBufferOutMemDescriptor, Exit);
	stopCommandMemDescriptor = IOMemoryDescriptor::withAddress (&dmaCommandBufferOut[numBlocks], sizeof (IODBDMADescriptor *), kIODirectionOut);
	FailIf (NULL == stopCommandMemDescriptor, Exit);

	commandBufferPhys = dmaCommandBufferOutMemDescriptor->getPhysicalAddress ();
	FailIf (NULL == commandBufferPhys, Exit);
	sampleBufferPhys = sampleBufferOutMemDescriptor->getPhysicalAddress ();
	FailIf (NULL == sampleBufferPhys, Exit);
	stopCommandPhys = stopCommandMemDescriptor->getPhysicalAddress ();
	FailIf (NULL == stopCommandPhys, Exit);

    offset = 0;
    dmaCommand = kdbdmaOutputMore;
    interruptIndex = kDBDMAOutputIndex;

	// install an interrupt handler only on the Ouput size of it
    workLoop = getWorkLoop();
    FailIf (!workLoop, Exit);
    
    interruptEventSource = IOFilterInterruptEventSource::filterInterruptEventSource(this,
                                                                               Apple02DBDMAAudioDMAEngine::interruptHandler,
                                                                               Apple02DBDMAAudioDMAEngine::interruptFilter,
                                                                               audioDevice->getProvider(),
                                                                               interruptIndex);
    FailIf (!interruptEventSource, Exit);
    workLoop->addEventSource(interruptEventSource);

	// create the DMA program
    for (blockNum = 0; blockNum < numBlocks; blockNum++) {
		IOPhysicalAddress	cmdDest;

        if (offset >= PAGE_SIZE) {
			sampleBufferPhys = sampleBufferOutMemDescriptor->getPhysicalSegment (blockNum * blockSize, 0);
			FailIf (NULL == sampleBufferPhys, Exit);
            offset = 0;
        }

        // This code assumes that the size of the IODBDMADescriptor divides evenly into the page size
        // If this is the last block, branch to the first block
        if (blockNum == (numBlocks - 1)) {
            cmdDest = commandBufferPhys;
            doInterrupt = true;
        // Else if the next block starts on a page boundry, branch to it
        } else if ((((blockNum + 1) * sizeof(IODBDMADescriptor)) % PAGE_SIZE) == 0) {
			cmdDest = dmaCommandBufferOutMemDescriptor->getPhysicalSegment ((blockNum + 1) * sizeof(IODBDMADescriptor), 0);
			FailIf (NULL == cmdDest, Exit);
		// No branch in the common case
        } else {
            cmdDest = 0;
        }

        if (cmdDest) {
            IOMakeDBDMADescriptorDep(&dmaCommandBufferOut[blockNum],
                                     dmaCommand,
                                     kdbdmaKeyStream0,
                                     doInterrupt ? kdbdmaIntAlways : kdbdmaIntNever,
                                     kdbdmaBranchAlways,
                                     kdbdmaWaitNever,
                                     blockSize,
                                     sampleBufferPhys + offset,
                                     cmdDest);
        } else {
            IOMakeDBDMADescriptorDep(&dmaCommandBufferOut[blockNum],
                                     dmaCommand,
                                     kdbdmaKeyStream0,
                                     kdbdmaIntNever,
                                     kdbdmaBranchIfTrue,
                                     kdbdmaWaitNever,
                                     blockSize,
                                     sampleBufferPhys + offset,
                                     stopCommandPhys);
        }
        offset += blockSize;
    }

    IOMakeDBDMADescriptor(&dmaCommandBufferOut[blockNum],
                          kdbdmaStop,
                          kdbdmaKeyStream0,
                          kdbdmaIntNever,
                          kdbdmaBranchNever,
                          kdbdmaWaitNever,
                          0,
                          0);
    
	// create the DMA input code
    if(ioBaseDMAInput) {
        dmaCommandBufferIn = (IODBDMADescriptor *)IOMallocAligned(commandBufferSize, 32); 
                                                            // needs to be more than 4 byte aligned
        FailIf (!dmaCommandBufferIn, Exit);

		dmaCommandBufferInMemDescriptor = IOMemoryDescriptor::withAddress (dmaCommandBufferIn, commandBufferSize, kIODirectionOut);
		FailIf (NULL == dmaCommandBufferInMemDescriptor, Exit);
		sampleBufferInMemDescriptor = IOMemoryDescriptor::withAddress ((void *)sampleBufIn, numBlocks * blockSize, kIODirectionIn);
		FailIf (NULL == sampleBufferInMemDescriptor, Exit);
		stopCommandMemDescriptor = IOMemoryDescriptor::withAddress (&dmaCommandBufferIn[numBlocks], sizeof (IODBDMADescriptor *), kIODirectionOut);
		FailIf (NULL == stopCommandMemDescriptor, Exit);

		commandBufferPhys = dmaCommandBufferInMemDescriptor->getPhysicalAddress ();
		FailIf (NULL == commandBufferPhys, Exit);
		sampleBufferPhys = sampleBufferInMemDescriptor->getPhysicalAddress ();
		FailIf (NULL == sampleBufferPhys, Exit);
		stopCommandPhys = stopCommandMemDescriptor->getPhysicalAddress ();
		FailIf (NULL == stopCommandPhys, Exit);

        doInterrupt = false;
        offset = 0;
        dmaCommand = kdbdmaInputMore;    
        
        for (blockNum = 0; blockNum < numBlocks; blockNum++) {
			IOPhysicalAddress	cmdDest;

            if (offset >= PAGE_SIZE) {
				sampleBufferPhys = sampleBufferInMemDescriptor->getPhysicalSegment (blockNum * blockSize, 0);
				FailIf (NULL == sampleBufferPhys, Exit);
                offset = 0;
            }

                // This code assumes that the size of the IODBDMADescriptor 
                // divides evenly into the page size
                // If this is the last block, branch to the first block
            if (blockNum == (numBlocks - 1)) {
                cmdDest = commandBufferPhys;
                // doInterrupt = true;
                // Else if the next block starts on a page boundry, branch to it
            } else if ((((blockNum + 1) * sizeof(IODBDMADescriptor)) % PAGE_SIZE) == 0) {
				cmdDest = dmaCommandBufferInMemDescriptor->getPhysicalSegment ((blockNum + 1) * sizeof(IODBDMADescriptor), 0);
				FailIf (NULL == cmdDest, Exit);
                // No branch in the common case
            } else {
                cmdDest = 0;
            }

            if (cmdDest) {
                IOMakeDBDMADescriptorDep(&dmaCommandBufferIn[blockNum],
                                     dmaCommand,
                                     kdbdmaKeyStream0,
                                     doInterrupt ? kdbdmaIntAlways : kdbdmaIntNever,
                                     kdbdmaBranchAlways,
                                     kdbdmaWaitNever,
                                     blockSize,
                                     sampleBufferPhys + offset,
                                     cmdDest);
            } else {
                IOMakeDBDMADescriptorDep(&dmaCommandBufferIn[blockNum],
                                     dmaCommand,
                                     kdbdmaKeyStream0,
                                     kdbdmaIntNever,
                                     kdbdmaBranchIfTrue,
                                     kdbdmaWaitNever,
                                     blockSize,
                                     sampleBufferPhys + offset,
                                     stopCommandPhys);
            }
            offset += blockSize;
        }

        IOMakeDBDMADescriptor(&dmaCommandBufferIn[blockNum],
                          kdbdmaStop,
                          kdbdmaKeyStream0,
                          kdbdmaIntNever,
                          kdbdmaBranchNever,
                          kdbdmaWaitNever,
                          0,
                          0);

    }

	chooseOutputClippingRoutinePtr();
	chooseInputConversionRoutinePtr();
	
	result = TRUE;

Exit:
    debugIOLog (3, "- Apple02DBDMAAudioDMAEngine::initHardware()");
    return result;
}


bool Apple02DBDMAAudioDMAEngine::interruptFilter(OSObject *owner, IOFilterInterruptEventSource *source)
{
    register Apple02DBDMAAudioDMAEngine *dmaEngine = (Apple02DBDMAAudioDMAEngine *)owner;
    bool result = true;

    if (dmaEngine) {
        result = dmaEngine->filterInterrupt(source->getIntIndex());
    }

    return result;
}

void Apple02DBDMAAudioDMAEngine::interruptHandler(OSObject *owner, IOInterruptEventSource *source, int count)
{
    return;
}

IOReturn Apple02DBDMAAudioDMAEngine::performAudioEngineStart()
{
	IOPhysicalAddress			commandBufferPhys;
	IOReturn					result;

    debugIOLog (3, "+ Apple02DBDMAAudioDMAEngine::performAudioEngineStart()");

	result = kIOReturnError;
    FailIf (!ioBaseDMAOutput || !dmaCommandBufferOut || !status || !interruptEventSource, Exit);

    flush_dcache((vm_offset_t)dmaCommandBufferOut, commandBufferSize, false);
    if(ioBaseDMAInput)
        flush_dcache((vm_offset_t)dmaCommandBufferIn, commandBufferSize, false);

	*((UInt32 *)&mLastOutputSample) = 0;
	*((UInt32 *)&mLastInputSample) = 0;

    interruptEventSource->enable();

	// add the time stamp take to test
    takeTimeStamp(false);

	// start the input DMA first
    if(ioBaseDMAInput) {
        IOSetDBDMAChannelControl(ioBaseDMAInput, IOClearDBDMAChannelControlBits(kdbdmaS0));
        IOSetDBDMABranchSelect(ioBaseDMAInput, IOSetDBDMAChannelControlBits(kdbdmaS0));
		commandBufferPhys = dmaCommandBufferInMemDescriptor->getPhysicalAddress ();
		FailIf (NULL == commandBufferPhys, Exit);
		IODBDMAStart(ioBaseDMAInput, (IODBDMADescriptor *)commandBufferPhys);
    }
    
    IOSetDBDMAChannelControl(ioBaseDMAOutput, IOClearDBDMAChannelControlBits(kdbdmaS0));
    IOSetDBDMABranchSelect(ioBaseDMAOutput, IOSetDBDMAChannelControlBits(kdbdmaS0));
	commandBufferPhys = dmaCommandBufferOutMemDescriptor->getPhysicalAddress ();
	FailIf (NULL == commandBufferPhys, Exit);
	((Apple02Audio *)audioDevice)->sndHWSetCurrentSampleFrame (0);
	IODBDMAStart(ioBaseDMAOutput, (IODBDMADescriptor *)commandBufferPhys);

	dmaRunState = TRUE;				//	rbm 7.12.02	added for user client support
	result = kIOReturnSuccess;

    debugIOLog (3, "- Apple02DBDMAAudioDMAEngine::performAudioEngineStart()");

Exit:
    return result;
}

IOReturn Apple02DBDMAAudioDMAEngine::performAudioEngineStop()
{
    UInt16 attemptsToStop = 2;

    debugIOLog (3, "+ Apple02DBDMAAudioDMAEngine::performAudioEngineStop()");

    if (!interruptEventSource) {
        return kIOReturnError;
    }

    interruptEventSource->disable();
        
	// stop the output
    IOSetDBDMAChannelControl(ioBaseDMAOutput, IOSetDBDMAChannelControlBits(kdbdmaS0));
    while ((IOGetDBDMAChannelStatus(ioBaseDMAOutput) & kdbdmaActive) && (attemptsToStop--)) {
        eieio();
        IOSleep(1);
    }

    attemptsToStop = 2;

    IODBDMAStop(ioBaseDMAOutput);
    IODBDMAReset(ioBaseDMAOutput);

	// stop the input
    if(ioBaseDMAInput){
        IOSetDBDMAChannelControl(ioBaseDMAInput, IOSetDBDMAChannelControlBits(kdbdmaS0));
        while ((IOGetDBDMAChannelStatus(ioBaseDMAInput) & kdbdmaActive) && (attemptsToStop--)) {
            eieio();
            IOSleep(1);
        }

        IODBDMAStop(ioBaseDMAInput);
        IODBDMAReset(ioBaseDMAInput);
    }
    
	dmaRunState = FALSE;				//	rbm 7.12.02	added for user client support
    interruptEventSource->enable();

    debugIOLog (3, "- Apple02DBDMAAudioDMAEngine::performAudioEngineStop()");
    return kIOReturnSuccess;
}

IOReturn Apple02DBDMAAudioDMAEngine::restartDMA () {
	IOReturn					result;

	result = kIOReturnError;
    FailIf (!ioBaseDMAOutput || !dmaCommandBufferOut || !interruptEventSource, Exit);

	performAudioEngineStop ();
	performAudioEngineStart ();
	result = kIOReturnSuccess;

Exit:
    return result;
}

OSString *Apple02DBDMAAudioDMAEngine::getGlobalUniqueID()
{
    const char *className = NULL;
    const char *location = NULL;
    char *uniqueIDStr;
    OSString *localID = NULL;
    OSString *uniqueID = NULL;
    UInt32 uniqueIDSize;
    
	className = "AppleDBDMAAudioDMAEngine";
    
    location = getLocation();
    
    localID = getLocalUniqueID();
    
    uniqueIDSize = 3;
    
    if (className) {
        uniqueIDSize += strlen(className);
    }
    
    if (location) {
        uniqueIDSize += strlen(location);
    }
    
    if (localID) {
        uniqueIDSize += localID->getLength();
    }
        
    uniqueIDStr = (char *)IOMallocAligned(uniqueIDSize, sizeof (char));
    
    if (uniqueIDStr) {
		bzero(uniqueIDStr, uniqueIDSize);

        if (className) {
            sprintf(uniqueIDStr, "%s:", className);
        }
        
        if (location) {
            strcat(uniqueIDStr, location);
            strcat(uniqueIDStr, ":");
        }
        
        if (localID) {
            strcat(uniqueIDStr, localID->getCStringNoCopy());
            localID->release();
        }
        
        uniqueID = OSString::withCString(uniqueIDStr);
        
        IOFreeAligned(uniqueIDStr, uniqueIDSize);
    }

    return uniqueID;
}

void Apple02DBDMAAudioDMAEngine::setSampleLatencies (UInt32 outputLatency, UInt32 inputLatency) {
	setOutputSampleLatency (outputLatency);
	setInputSampleLatency (inputLatency);
}

void Apple02DBDMAAudioDMAEngine::stop(IOService *provider)
{
    IOWorkLoop *workLoop;
    
    debugIOLog (3, " + Apple02DBDMAAudioDMAEngine[%p]::stop(%p)", this, provider);
    
    if (interruptEventSource) {
        workLoop = getWorkLoop();
        if (workLoop) {
            workLoop->removeEventSource(interruptEventSource);
        }
    }
    
    super::stop(provider);
    stopAudioEngine();
Exit:
    debugIOLog (3, " - Apple02DBDMAAudioDMAEngine[%p]::stop(%p)", this, provider);
}

#pragma mark ------------------------ 
#pragma mark еее Conversion Routines
#pragma mark ------------------------ 

// [3094574] aml, pick the correct output conversion routine based on our current state
void Apple02DBDMAAudioDMAEngine::chooseOutputClippingRoutinePtr()
{
	if (FALSE == dbdmaFormat.fIsMixable ) { // [3383910] need to do memcpy any time format is non-mixable
		mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipLegacyMemCopyToOutputStream;
	} else {
		if (0) {
		} else {
			if (32 == dbdmaFormat.fBitWidth) {
				if (TRUE == fNeedsRightChanMixed) {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32MixRightChannel;
				} else if (TRUE == fNeedsRightChanDelay) {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32DelayRightChannel;
				} else {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32;
				}
			} else if (16 == dbdmaFormat.fBitWidth) {
				if (TRUE == fNeedsPhaseInversion) {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16InvertRightChannel;
				} else if (TRUE == fNeedsRightChanMixed) {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16MixRightChannel;
				} else if (TRUE == fNeedsRightChanDelay) {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16DelayRightChannel;
				} else {
					mClipAppleLegacyDBDMAToOutputStreamRoutine = &Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16;
				}
			} else {
				debugIOLog (3, "Apple02DBDMAAudioDMAEngine::chooseOutputClippingRoutinePtr - Non-supported output bit depth!");
			}
		}
	}
}

// [3094574] aml, pick the correct input conversion routine based on our current state
void Apple02DBDMAAudioDMAEngine::chooseInputConversionRoutinePtr() 
{
	if (32 == dbdmaFormat.fBitWidth) {
		if (mUseSoftwareInputGain) {
			if (fNeedsRightChanDelayInput) {// [3173869]
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32DelayRightWithGain;
			} else {
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32WithGain;
			}
		} else {
			mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32;
		}
	} else if (16 == dbdmaFormat.fBitWidth) {
		if (mUseSoftwareInputGain) {
			if (fNeedsRightChanDelayInput) {// [3173869]
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16DelayRightWithGain;
			} else {
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16WithGain;
			}
		} else {
			if (e_Mode_CopyRightToLeft == mInputDualMonoMode) {
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16CopyR2L;
			} else if (e_Mode_CopyLeftToRight == mInputDualMonoMode) {
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16CopyL2R;
			} else {
				mConvertInputStreamToAppleLegacyDBDMARoutine = &Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16;
			}
		}
	} else {
		debugIOLog (3, "Apple02DBDMAAudioDMAEngine::chooseInputConversionRoutinePtr - Non-supported input bit depth!");
	}
}

IOReturn Apple02DBDMAAudioDMAEngine::clipOutputSamples(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, IOAudioStream *audioStream)
{
	IOReturn result = kIOReturnSuccess;
 
 	// if the DMA went bad restart it
	if (mNeedToRestartDMA) {
		mNeedToRestartDMA = FALSE;
		restartDMA ();
	}

	startTiming();

	if (0 != numSampleFrames) {
		// [3094574] aml, use function pointer instead of if/else block - handles both iSub and non-iSub clipping cases.
		result = (*this.*mClipAppleLegacyDBDMAToOutputStreamRoutine)(mixBuf, sampleBuf, firstSampleFrame, numSampleFrames, streamFormat);
	}

	endTiming();

	return result;
}

// [3094574] aml, use function pointer instead of if/else block
IOReturn Apple02DBDMAAudioDMAEngine::convertInputSamples(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, IOAudioStream *audioStream)
{
	IOReturn result;

 	// if the DMA went bad restart it
	if (mNeedToRestartDMA) {
		mNeedToRestartDMA = FALSE;
		restartDMA ();
	}

	result = (*this.*mConvertInputStreamToAppleLegacyDBDMARoutine)(sampleBuf, destBuf, firstSampleFrame, numSampleFrames, streamFormat);

	return result;
}

#pragma mark ------------------------ 
#pragma mark еее Output Routines
#pragma mark ------------------------ 

// ------------------------------------------------------------------------
// Copy buffer directly to output
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipLegacyMemCopyToOutputStream (const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
	UInt32			offset;
	UInt32			streamSize;

	streamSize = streamFormat->fNumChannels * (streamFormat->fBitWidth / 8);
	offset = firstSampleFrame * streamSize;
	
	memcpy ((UInt8 *)sampleBuf + offset, (UInt8 *)mixBuf, numSampleFrames * streamSize);
	
	return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt16
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;
	
	Float32ToNativeInt16( inFloatBufferPtr, outSInt16BufferPtr, numSamples );

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt16, delay right channel to correct for TAS 3004
// I2S clocking issue which puts left and right samples out of phase.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16DelayRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{

    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;
	
	delayRightChannel( inFloatBufferPtr, numSamples , &mLastOutputSample);
	
	Float32ToNativeInt16( inFloatBufferPtr, outSInt16BufferPtr, numSamples );

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt16, delay right channel to correct for TAS 3004
// I2S clocking issue which puts left and right samples out of phase.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16DelayRightChannelBalance(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{

    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;
	
	delayRightChannel( inFloatBufferPtr, numSamples , &mLastOutputSample);

	balanceAdjust( inFloatBufferPtr, numSamples, (float *)&mLeftBalanceAdjust, (float *)&mRightBalanceAdjust);
	
	Float32ToNativeInt16( inFloatBufferPtr, outSInt16BufferPtr, numSamples );

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt16, invert phase to correct older iMac hardware
// assumes 2 channels.  Note that there is no 32 bit version of this 
// conversion routine, since the older hardware does not support it.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16InvertRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
	UInt32	numSamples;
	
	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;

	invertRightChannel( inFloatBufferPtr, numSamples );
 
	Float32ToNativeInt16( inFloatBufferPtr, outSInt16BufferPtr, numSamples );
   
	return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt16, mix right and left channels and mute right
// assumes 2 channels
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream16MixRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;
	
	mixAndMuteRightChannel( inFloatBufferPtr, numSamples );
	
	Float32ToNativeInt16( inFloatBufferPtr, outSInt16BufferPtr, numSamples );

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt32
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    float	*inFloatBufferPtr;
	SInt32	*outSInt32BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt32BufferPtr = (SInt32 *)sampleBuf + firstSampleFrame * streamFormat->fNumChannels;
	
	Float32ToNativeInt32( inFloatBufferPtr, outSInt32BufferPtr, numSamples );

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt32, delay right channel to correct for TAS 3004
// I2S clocking issue which puts left and right samples out of phase.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32DelayRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{

    float	*inFloatBufferPtr;
	SInt32	*outSInt32BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt32BufferPtr = (SInt32 *)sampleBuf + firstSampleFrame * streamFormat->fNumChannels;
	
	delayRightChannel( inFloatBufferPtr, numSamples, &mLastOutputSample );

	Float32ToNativeInt32( inFloatBufferPtr, outSInt32BufferPtr, numSamples );
	
    return kIOReturnSuccess;
}


// ------------------------------------------------------------------------
// Float32 to Native SInt32, delay right channel to correct for TAS 3004
// I2S clocking issue which puts left and right samples out of phase.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32DelayRightChannelBalance(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{

    float	*inFloatBufferPtr;
	SInt32	*outSInt32BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt32BufferPtr = (SInt32 *)sampleBuf + firstSampleFrame * streamFormat->fNumChannels;
	
	delayRightChannel( inFloatBufferPtr, numSamples, &mLastOutputSample );

	balanceAdjust( inFloatBufferPtr, numSamples, (float *)&mLeftBalanceAdjust, (float *)&mRightBalanceAdjust);

	Float32ToNativeInt32( inFloatBufferPtr, outSInt32BufferPtr, numSamples );
	
    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Float32 to Native SInt32, mix right and left channels and mute right
// assumes 2 channels
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::clipAppleLegacyDBDMAToOutputStream32MixRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    float	*inFloatBufferPtr;
	SInt32	*outSInt32BufferPtr;
	UInt32	numSamples;

	numSamples = numSampleFrames*streamFormat->fNumChannels;
    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
	outSInt32BufferPtr = (SInt32 *)sampleBuf + firstSampleFrame * streamFormat->fNumChannels;
	
	mixAndMuteRightChannel( inFloatBufferPtr, numSamples );

	Float32ToNativeInt32( inFloatBufferPtr, outSInt32BufferPtr, numSamples );

    return kIOReturnSuccess;
}

#pragma mark ------------------------ 
#pragma mark еее Input Routines
#pragma mark ------------------------ 

// ------------------------------------------------------------------------
// Native SInt16 to Float32
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf16;
	
    floatDestBuf = (float *)destBuf;
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf16 = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);

   	NativeInt16ToFloat32(inputBuf16, floatDestBuf, numSamplesLeft, 16);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt16 to Float32, copy the rigth sample to the left channel for
// older machines only.  Note that there is no 32 bit version of this  
// function because older hardware does not support it.
// ------------------------------------------------------------------------
// [3306493]
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16CopyL2R(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf16;
    
    floatDestBuf = (float *)destBuf;    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
 
	inputBuf16 = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
   
	NativeInt16ToFloat32CopyLeftToRight(inputBuf16, floatDestBuf, numSamplesLeft, 16);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt16 to Float32, copy the rigth sample to the left channel for
// older machines only.  Note that there is no 32 bit version of this  
// function because older hardware does not support it.
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16CopyR2L(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf16;
    
    floatDestBuf = (float *)destBuf;    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
 
	inputBuf16 = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
   
	NativeInt16ToFloat32CopyRightToLeft(inputBuf16, floatDestBuf, numSamplesLeft, 16);

    return kIOReturnSuccess;
}


// ------------------------------------------------------------------------
// Native SInt16 to Float32 with right channel delay and gain for TAS3004 compensation [3173869]
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16DelayRightWithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf16;
	
    floatDestBuf = (float *)destBuf;
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf16 = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);

	NativeInt16ToFloat32Gain(inputBuf16, floatDestBuf, numSamplesLeft, 16, mInputGainLPtr, mInputGainRPtr);

	delayRightChannel( floatDestBuf, numSamplesLeft , &mLastInputSample);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt16 to Float32, with software input gain
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream16WithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf16;
    
    floatDestBuf = (float *)destBuf;    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf16 = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);

	NativeInt16ToFloat32Gain(inputBuf16, floatDestBuf, numSamplesLeft, 16, mInputGainLPtr, mInputGainRPtr);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt32 to Float32
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt32 *inputBuf32;

    floatDestBuf = (float *)destBuf;
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf32 = &(((SInt32 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
   
	NativeInt32ToFloat32(inputBuf32, floatDestBuf, numSamplesLeft, 32);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt32 to Float32, with software input gain
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32WithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt32 *inputBuf32;
  
    floatDestBuf = (float *)destBuf;    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf32 = &(((SInt32 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);

	NativeInt32ToFloat32Gain(inputBuf32, floatDestBuf, numSamplesLeft, 32, mInputGainLPtr, mInputGainRPtr);

    return kIOReturnSuccess;
}

// ------------------------------------------------------------------------
// Native SInt32 to Float32 with right channel delay and gain for TAS3004 compensation [3173869]
// ------------------------------------------------------------------------
IOReturn Apple02DBDMAAudioDMAEngine::convertAppleLegacyDBDMAFromInputStream32DelayRightWithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt32 *inputBuf32;
  
    floatDestBuf = (float *)destBuf;    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
	inputBuf32 = &(((SInt32 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);

	NativeInt32ToFloat32Gain(inputBuf32, floatDestBuf, numSamplesLeft, 32, mInputGainLPtr, mInputGainRPtr);

	delayRightChannel( floatDestBuf, numSamplesLeft , &mLastInputSample);

    return kIOReturnSuccess;
}

#pragma mark ------------------------ 
#pragma mark еее State Routines
#pragma mark ------------------------ 

void Apple02DBDMAAudioDMAEngine::setDualMonoMode(const DualMonoModeType inDualMonoMode) 
{ 
	mInputDualMonoMode = inDualMonoMode; 
	chooseInputConversionRoutinePtr();

	return;   	
}

void Apple02DBDMAAudioDMAEngine::setInputGainL(UInt32 inGainL) 
{ 
    if (mInputGainLPtr == NULL) {        
        mInputGainLPtr = (float *)IOMalloc(sizeof(float));
    }
    inputGainConverter(inGainL, mInputGainLPtr);
	
    return;   
} 

void Apple02DBDMAAudioDMAEngine::setInputGainR(UInt32 inGainR) 
{ 
    if (mInputGainRPtr == NULL) {        
        mInputGainRPtr = (float *)IOMalloc(sizeof(float));
    }
    inputGainConverter(inGainR, mInputGainRPtr);

    return;   
} 

// [3094574] aml, updated routines below to set the proper clipping routine

void Apple02DBDMAAudioDMAEngine::setPhaseInversion(const bool needsPhaseInversion) 
{
	fNeedsPhaseInversion = needsPhaseInversion; 
	chooseOutputClippingRoutinePtr();
	
	return;   
}

void Apple02DBDMAAudioDMAEngine::setRightChanDelay(const bool needsRightChanDelay)  
{
	fNeedsRightChanDelay = needsRightChanDelay;  
	chooseOutputClippingRoutinePtr();
	
	return;   
}

void Apple02DBDMAAudioDMAEngine::setRightChanMixed(const bool needsRightChanMixed)  
{
	fNeedsRightChanMixed = needsRightChanMixed;  
	chooseOutputClippingRoutinePtr();
	
	return;   
}

void Apple02DBDMAAudioDMAEngine::setUseSoftwareInputGain(const bool inUseSoftwareInputGain) 
{     
	mUseSoftwareInputGain = inUseSoftwareInputGain;     	
	chooseInputConversionRoutinePtr();
	
	return;   
}

void Apple02DBDMAAudioDMAEngine::setRightChanDelayInput(const bool needsRightChanDelay)  // [3173869]
{
	// Don't call this because it messes up the input stream if two or more applications are recording at once.  [3398910]
/*
	debugIOLog (3, "setRightChanDelayInput (%d)", needsRightChanDelay);
	fNeedsRightChanDelayInput = needsRightChanDelay;  
	chooseInputConversionRoutinePtr();
*/
	return;   
}

void Apple02DBDMAAudioDMAEngine::setBalanceAdjust(const bool needsBalanceAdjust)  
{
	fNeedsBalanceAdjust = needsBalanceAdjust;  
	
	return;   
}

void Apple02DBDMAAudioDMAEngine::setLeftBalanceAdjust(UInt32 inVolume) 
{
	if (NULL != inVolume) {
		mLeftBalanceAdjust = inVolume;
    } else {
		mLeftBalanceAdjust = 0x3F800000;
	}
	return;   
} 

void Apple02DBDMAAudioDMAEngine::setRightBalanceAdjust(UInt32 inVolume) 
{ 
	if (NULL != inVolume) {
		mRightBalanceAdjust = inVolume;
    } else {
		mRightBalanceAdjust = 0x3F800000;
	}	
	return;   
} 

#pragma mark ------------------------ 
#pragma mark еее Format Routines
#pragma mark ------------------------ 

IOReturn Apple02DBDMAAudioDMAEngine::getAudioStreamFormat( IOAudioStreamFormat * streamFormatPtr )
{
	if ( NULL != streamFormatPtr ) {
		streamFormatPtr->fNumChannels = dbdmaFormat.fNumChannels;
		streamFormatPtr->fSampleFormat = dbdmaFormat.fSampleFormat;
		streamFormatPtr->fNumericRepresentation = dbdmaFormat.fNumericRepresentation;
		streamFormatPtr->fBitDepth = dbdmaFormat.fBitDepth;
		streamFormatPtr->fBitWidth = dbdmaFormat.fBitWidth;
		streamFormatPtr->fAlignment = dbdmaFormat.fAlignment;
		streamFormatPtr->fByteOrder = dbdmaFormat.fByteOrder;
		streamFormatPtr->fIsMixable = dbdmaFormat.fIsMixable;
		streamFormatPtr->fDriverTag = dbdmaFormat.fDriverTag;
	}
	return kIOReturnSuccess;
}

bool Apple02DBDMAAudioDMAEngine::getDmaState (void )
{
	return dmaRunState;
}

IOReturn Apple02DBDMAAudioDMAEngine::performFormatChange(IOAudioStream *audioStream, const IOAudioStreamFormat *newFormat, const IOAudioSampleRate *newSampleRate)
{
	if ( NULL != newFormat ) {									//	rbm 7.15.2002 keep a copy for user client
		dbdmaFormat.fNumChannels = newFormat->fNumChannels;
		dbdmaFormat.fSampleFormat = newFormat->fSampleFormat;
		dbdmaFormat.fNumericRepresentation = newFormat->fNumericRepresentation;
		dbdmaFormat.fBitDepth = newFormat->fBitDepth;
		dbdmaFormat.fBitWidth = newFormat->fBitWidth;
		dbdmaFormat.fAlignment = newFormat->fAlignment;
		dbdmaFormat.fByteOrder = newFormat->fByteOrder;
		dbdmaFormat.fIsMixable = newFormat->fIsMixable;
		dbdmaFormat.fDriverTag = newFormat->fDriverTag;

		// [3094574] aml, set the proper clipping routine
		chooseOutputClippingRoutinePtr();
		chooseInputConversionRoutinePtr();
	}

    return kIOReturnSuccess;
}

#pragma mark ------------------------ 
#pragma mark еее Utilities
#pragma mark ------------------------ 

inline void Apple02DBDMAAudioDMAEngine::startTiming() {
#ifdef _TIME_CLIP_ROUTINE
	AbsoluteTime				uptime;
	AbsoluteTime				lastuptime;
	AbsoluteTime				tempTime;
	UInt64						nanos;

	mCallCount++;
	clock_get_uptime (&uptime);
	tempTime = uptime;
	if ((mCallCount % kCallFrequency) == 0) {
		SUB_ABSOLUTETIME (&uptime, &mPreviousUptime);
		absolutetime_to_nanoseconds (uptime, &nanos);
		debugIOLog (3, "clipOutputSamples[%ld]:\t%ld:", mCallCount, uptime.lo);
	}
	mPreviousUptime = tempTime;

	if ((mCallCount % kCallFrequency) == 0) {
		clock_get_uptime (&lastuptime);
	}	
#endif
}

inline void Apple02DBDMAAudioDMAEngine::endTiming() {
#ifdef _TIME_CLIP_ROUTINE
	if ((mCallCount % kCallFrequency) == 0) {
		clock_get_uptime (&uptime);
		SUB_ABSOLUTETIME (&uptime, &lastuptime);
		absolutetime_to_nanoseconds (uptime, &nanos);
		debugIOLog (3, "%ld", uptime.lo);
	}
#endif
}
