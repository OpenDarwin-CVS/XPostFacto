/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1999-2000 Apple Computer, Inc.  All rights reserved.
 *
 *  DRI: Josh de Cesare
 *
 */

#include <ppc/proc_reg.h>

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IODeviceMemory.h>
#include <IOKit/IOPlatformExpert.h>

#include <IOKit/platform/AppleNMI.h>

#include <IOKit/ppc/IODBDMA.h>

#include <IOKit/pci/IOPCIBridge.h>

#include "AppleOHare.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super AppleMacIO

OSDefineMetaClassAndStructors(AppleOHare, AppleMacIO);

#define BANDIT_MAGIC	0x50
#define BANDIT_COHERENT	0x40

bool
AppleOHare::passiveMatch (OSDictionary *matching, bool changesOK)
{
	// The problem is that the AppleMediaBay driver calls waitForService (serviceMatching ("OHare")).
	// That would be OK if we could inherit from OHare. The problem, among other things, is that OHare
	// is compiled into the kernel in 10.0 and 10.1, but probably not later. So, instead, we just
	// override passiveMatch so that this will match OHare. This works, because AppleMediaBay uses
	// callPlatformFunction to interface with OHare.

	OSString *str = OSDynamicCast (OSString, matching->getObject (gIOProviderClassKey));
	if (str && str->isEqualTo ("OHare")) return true;
	return super::passiveMatch (matching, changesOK);
}

bool AppleOHare::start(IOService *provider)
{
	IOInterruptAction  handler;
	OSSymbol           *interruptControllerName;
	AppleNMI           *appleNMI;
	long               nmiSource;
	OSData             *nmiData;
	IOReturn           error;

	// Call MacIO's start.
	if (!super::start(provider))
		return false;

	// get the base address of the this OHare.
	ohareBaseAddress = fMemory->getVirtualAddress();

	// Figure out which ohare this is.
	if (IODTMatchNubWithKeys(provider, "ohare"))
		ohareNum = kPrimaryOHare;
	else if (IODTMatchNubWithKeys(provider, "'pci106b,7'"))
		ohareNum = kSecondaryOHare;
	else return false;  // This should not happen.

	// just initializes this:
	mediaIsOn = true;

	ohare_powerMediaBay = OSSymbol::withCString("powerMediaBay");
	ohare_sleepState = OSSymbol::withCString("ohare_sleepState");

	if (ohareNum == kPrimaryOHare) {
		getPlatform()->setCPUInterruptProperties(provider);
	}

	// Make nubs for the children.
	publishBelow( provider );

//		*((unsigned long *)(ohareBaseAddress + 0x38)) = 0x7affbe00;

	// get the name of the interrupt controller
	interruptControllerName  = getInterruptControllerName();

	// Allocate the interruptController instance.
	interruptController = new AppleOHareInterruptController;
	if (interruptController == NULL) return false;

	// call the interruptController's init method.
	error = interruptController->initInterruptController(provider, ohareBaseAddress);
	if (error != kIOReturnSuccess) return false;

	handler = interruptController->getInterruptHandlerAddress();
	provider->registerInterrupt(0, interruptController, handler, 0);

	provider->enableInterrupt(0);

	// Register the interrupt controller so clients can find it.
	getPlatform()->registerInterruptController(interruptControllerName,
											interruptController);

	// mark the current state as invalid:
	savedState.thisStateIsValid = false;

	// attach to the power managment tree:
	initForPM(provider);
	
	if (ohareNum != kPrimaryOHare) return true;

	// Create the NMI Driver.
	nmiSource = 20;
	nmiData = OSData::withBytes(&nmiSource, sizeof(long));
	appleNMI = new AppleNMI;
	if ((nmiData != 0) && (appleNMI != 0)) {
		appleNMI->initNMI(interruptController, nmiData);
	}

	IOPCIBridge *bandit = OSDynamicCast (IOPCIBridge, getProvider()->getProvider());
	if (bandit) {
		unsigned magic = bandit->configRead32 (bandit->getBridgeSpace (), BANDIT_MAGIC);
		setProperty ("origBanditMagic", magic, 32);
		if ((magic & BANDIT_COHERENT) == 0) {
			bandit->configWrite32 (bandit->getBridgeSpace (), BANDIT_MAGIC, magic | BANDIT_COHERENT);
		}
	}

	registerService ();

	return true;
}

IOReturn
AppleOHare::callPlatformFunction(const OSSymbol *functionName,
								 bool waitForFunction,
								 void *param1, void *param2,
								 void *param3, void *param4)
{
    if (functionName == ohare_sleepState)
    {
        sleepState((bool)param1);
        return kIOReturnSuccess;
    }
	if (functionName == ohare_powerMediaBay) {
		bool powerOn = (param1 != NULL);
		powerMediaBay (powerOn, (UInt8)param2);
		return kIOReturnSuccess;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}

void
AppleOHare::enableMBATA()
{
    unsigned long ohareIDs, ohareFCR;

    ohareIDs = lwbrx(ohareBaseAddress + 0x34);
    if ((ohareIDs & 0x0000FF00) == 0x00003000) {
        ohareFCR = lwbrx(ohareBaseAddress + 0x38);
        ohareFCR |= 0x00800000;
        stwbrx(ohareFCR, ohareBaseAddress + 0x38);
		IODelay(100);
    }
}

void
AppleOHare::powerMediaBay(bool powerOn, UInt8 deviceOn)
{
	unsigned long ohareIDs;
	unsigned long powerDevice = deviceOn;
	
    // kprintf("AppleOHare::powerMediaBay(%s) 0x%02x\n", (powerOn ? "TRUE" : "FALSE"), powerDevice);
    // kprintf(" 0 AppleOHare::powerMediaBay = 0x%08lx\n", lwbrx(appleOHareBaseAddress + 0x38));

    if (mediaIsOn == powerOn) return;

    // Align the bits of the power device:
    powerDevice = powerDevice << 26;
    powerDevice &= ohareFCMediaBaybits;

    ohareIDs = lwbrx(ohareBaseAddress + 0x34);
    if ((ohareIDs & 0x0000FF00) == 0x00003000) {
		unsigned long *ohareFCR = (unsigned long*)(ohareBaseAddress + 0x38);

		// kprintf(" 1 AppleOHare::powerMediaBay = 0x%08lx\n", *ohareFCR);

		// make sure media bay is in reset (MB reset bit is low)
		*ohareFCR &= ~(1<<ohareFCMBReset);
		eieio();

		if (powerOn) {
			// we are powering on the bay and need a delay between turning on
            // media bay power and enabling the bus
			*ohareFCR &= ~(1<<ohareFCMBPwr);
			eieio();

			IODelay(50000);
		}

		// to turn on the buses, we ensure all buses are off and then turn on the ata bus
		*ohareFCR &= ~(ohareFCMediaBaybits);
		eieio();
		*ohareFCR |= powerDevice;
		eieio();

		if (!powerOn) {
			// turn off media bay power
			*ohareFCR |= 1 << ohareFCMBPwr;
			eieio();
		} else {
			// take us out of reset
			*ohareFCR |= 1 << ohareFCMBReset;
			eieio();

			enableMBATA();
		}

		IODelay(50000);
		// kprintf(" 2 AppleOHare::powerMediaBay = 0x%08lx\n", *ohareFCR);
        // kprintf(" 3 AppleOHare::powerMediaBay = 0x%08lx\n", lwbrx(appleOHareBaseAddress + 0x38));
    }

    mediaIsOn = powerOn;
}

OSSymbol *
AppleOHare::getInterruptControllerName(void)
{
	OSSymbol *interruptControllerName;

	switch (ohareNum) {
		case kPrimaryOHare :
			interruptControllerName = (OSSymbol *) gIODTDefaultInterruptController;
			break;

		case kSecondaryOHare :
			interruptControllerName = (OSSymbol *) OSSymbol::withCStringNoCopy("SecondaryInterruptController");
			break;

		default:
			interruptControllerName = (OSSymbol *) OSSymbol::withCStringNoCopy("UnknownInterruptController");
			break;
	}

	return interruptControllerName;
}

void AppleOHare::processNub(IOService *nub)
{
	int           cnt, numSources;
	OSArray       *controllerNames, *controllerSources;
	OSSymbol      *interruptControllerName;
	char          *nubName;

	nubName = (char *)nub->getName();

	if (!strcmp(nubName, "media-bay")) {
		enableMBATA();
	}

	// change the interrupt controller name for this nub
    // if it is on the secondary ohare
	if (ohareNum == kPrimaryOHare) return;

	interruptControllerName = getInterruptControllerName();

	if (!strcmp(nubName, "media-bay")) {
		controllerSources = OSDynamicCast(OSArray, getProperty("vectors-media-bay"));
	} else if (!strcmp(nubName, "ch-a")) {
		controllerSources = OSDynamicCast(OSArray, getProperty("vectors-escc-ch-a"));
	} else if (!strcmp(nubName, "floppy")) {
		controllerSources = OSDynamicCast(OSArray, getProperty("vectors-floppy"));
	} else if (!strcmp(nubName, "ata4")) {
		controllerSources = OSDynamicCast(OSArray, getProperty("vectors-ata4"));
	} else return;

	if (controllerSources == NULL) {
		IOLog ("controllerSources was NULL\n");
		return;
	}
	
	numSources = controllerSources->getCount();

	controllerNames = OSArray::withCapacity(numSources);
	for (cnt = 0; cnt < numSources; cnt++) {
		controllerNames->setObject(interruptControllerName);
	}

	nub->setProperty(gIOInterruptControllersKey, controllerNames);
	nub->setProperty(gIOInterruptSpecifiersKey, controllerSources);
}

// --------------------------------------------------------------------------
// Method: initForPM
//
// Purpose:
//   initialize the driver for power managment and register ourselves with
//   superclass policy-maker
void AppleOHare::initForPM (IOService *provider)
{
    PMinit();                   // initialize superclass variables
    provider->joinPMtree(this); // attach into the power management hierarchy

    // KeyLargo has only 2 power states::
    // 0 OFF
    // 1 all ON
    // Pwer state fields:
    // unsigned long	version;		// version number of this struct
    // IOPMPowerFlags	capabilityFlags;	// bits that describe (to interested drivers) the capability of the device in this state
    // IOPMPowerFlags	outputPowerCharacter;	// description (to power domain children) of the power provided in this state
    // IOPMPowerFlags	inputPowerRequirement;	// description (to power domain parent) of input power required in this state
    // unsigned long	staticPower;		// average consumption in milliwatts
    // unsigned long	unbudgetedPower;	// additional consumption from separate power supply (mw)
    // unsigned long	powerToAttain;		// additional power to attain this state from next lower state (in mw)
    // unsigned long	timeToAttain;		// time required to enter this state from next lower state (in microseconds)
    // unsigned long	settleUpTime;		// settle time required after entering this state from next lower state (microseconds)
    // unsigned long	timeToLower;		// time required to enter next lower state from this one (in microseconds)
    // unsigned long	settleDownTime;		// settle time required after entering next lower state from this state (microseconds)
    // unsigned long	powerDomainBudget;	// power in mw a domain in this state can deliver to its children

    // NOTE: all these values are made up since now I do not have areal clue of what to put.
#define number_of_power_states 2

    static IOPMPowerState ourPowerStates[number_of_power_states] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,IOPMDeviceUsable,IOPMPowerOn,IOPMPowerOn,0,0,0,0,0,0,0,0}
    };

    // register ourselves with ourself as policy-maker
    if (pm_vars != NULL) registerPowerDriver(this, ourPowerStates, number_of_power_states);
}

// Method: setPowerState
//
// VERY IMPORTANT NOTE:
// sleepState(bool) can be called from here or directly. This is NOT an oversight.
// What I am trying to resolve here is a problem with those powerbooks that have
// 2 Heathrow chips. In these machines the main Heathrow should be powered on
// in the CPU driver, and the second here. Since the HeathrowState holds a bit
// to remeber if the state is valid, and such a bit is cleared once the state is
// restored I am sure that I am not going to overwrite a valid state with an (older)
// invalid one.
IOReturn
AppleOHare::setPowerState(unsigned long powerStateOrdinal, IOService* whatDevice)
{
    if ( powerStateOrdinal == 0 ) {
        if (ohareNum == kSecondaryOHare) {
            kprintf("Secondary OHare would be powered off here\n");
            sleepState(true);
        }
        else if (ohareNum == kPrimaryOHare) {
            kprintf("Primary OHare would be powered off here\n");
            /* None the CPU driver handles this */
        }
    }
    if ( powerStateOrdinal == 1 ) {
        if (ohareNum == kSecondaryOHare) {
            kprintf("Secondary OHare would be powered on here\n");
            sleepState(false);
        }
        else if (ohareNum == kPrimaryOHare) {
            kprintf("Primary OHare would be powered on here\n");
            /* None the CPU driver handles this */
        }
    }
    return IOPMAckImplied;
}


// If sleepMe is true places heatrow to sleep,
// Otherwise wakes it up.
void
AppleOHare::sleepState(bool sleepMe)
{
    if (sleepMe) {
        // Saves the state and creates the conditions for sleep:
		kprintf ("AppleOHare::sleepMe %s\n", ohareNum == kPrimaryOHare ? "primary" : "secondary");
		
        // Disables and saves all the interrupts:
        kprintf("AppleOHare::sleepState saveInterruptState\n");
        saveInterruptState();

        // Saves all the DMA registers:
        kprintf("AppleOHare::sleepState saveDMAState\n");
        saveDMAState();

        // Saves the VIA registers:
		if (ohareNum == kPrimaryOHare) {
			kprintf("AppleOHare::sleepState saveVIAState\n");
			saveVIAState();
		}
		
        // Saves the GP registers:
        kprintf("AppleOHare::sleepState saveGPState\n");
        saveGPState();

        // Defines the state as valid:
		kprintf("AppleOHare::sleepState defineStateValid\n");
        savedState.thisStateIsValid = true;
    }
    else if (savedState.thisStateIsValid) {
        // Restores the GP registers:
        kprintf("AppleOHare::sleepState restoreGPState\n");
        restoreGPState();

        // Wakes up and restores the state:
		if (ohareNum == kPrimaryOHare) {
			kprintf("AppleOHare::sleepState restoreVIAState\n");
			restoreVIAState();
		}
			
        // Restores the DMA registers:
        kprintf("AppleOHare::sleepState restoreDMAState\n");
        restoreDMAState();

        // Restores and enables the interrupts:
        kprintf("AppleOHare::sleepState restoreInterruptState\n");
        restoreInterruptState();

        // This state is no more valid:
		kprintf("AppleOHare::sleepState defineStateInvalid\n");
        savedState.thisStateIsValid = false;

        // Turn on the media bay if necessary.
		kprintf("AppleOHare::sleepState enableMBATA\n");
        enableMBATA();
    }
}

void AppleOHare::saveInterruptState()
{
    // Save the interrupt state
    savedState.interruptMask = *(UInt32*)(ohareBaseAddress + kMaskOffset);
    eieio();
}

void AppleOHare::restoreInterruptState()
{
    // Clears all the possible pending interrupts
    *(UInt32*)(ohareBaseAddress + kClearOffset) = 0xFFFFFFFF;
    eieio();

    // Restores the interrupts
    *(UInt32*)(ohareBaseAddress + kMaskOffset) = savedState.interruptMask;
    eieio();

    // Clears all the possible pending interrupts (again)
    *(UInt32*)(ohareBaseAddress + kClearOffset) = 0xFFFFFFFF;
    eieio();
}

void AppleOHare::saveGPState()
{
    savedState.featureControlReg = *(UInt32*)(ohareBaseAddress + 0x00000038);
    savedState.auxControlReg     = *(UInt32*)(ohareBaseAddress + 0x0000003C);
}

void AppleOHare::restoreGPState()
{
    *(UInt32*)(ohareBaseAddress + 0x00000038) = savedState.featureControlReg;
    eieio();
    IODelay(1000);
    *(UInt32*)(ohareBaseAddress + 0x0000003C) = savedState.auxControlReg;
    eieio();
    IODelay(1000);
}

void AppleOHare::saveDMAState()
{
    int i;
    UInt32 channelOffset;

    for (i = 0, channelOffset = 0; i < 12; i++, channelOffset += 0x0100)
    {
        volatile DBDMAChannelRegisters*	currentChannel;

        currentChannel = (volatile DBDMAChannelRegisters *) (ohareBaseAddress + 0x8000 + channelOffset);

        savedState.savedDBDMAState[i].commandPtrLo = IOGetDBDMACommandPtr(currentChannel);
        savedState.savedDBDMAState[i].interruptSelect = IOGetDBDMAInterruptSelect(currentChannel);
        savedState.savedDBDMAState[i].branchSelect = IOGetDBDMABranchSelect(currentChannel);
        savedState.savedDBDMAState[i].waitSelect = IOGetDBDMAWaitSelect(currentChannel);
    }
}

void AppleOHare::restoreDMAState()
{
    int i;
    UInt32 channelOffset;

    for (i = 0, channelOffset = 0; i < 12; i++, channelOffset += 0x0100)
    {
        volatile DBDMAChannelRegisters* currentChannel;

        currentChannel = (volatile DBDMAChannelRegisters *) (ohareBaseAddress + 0x8000 + channelOffset);

        IODBDMAReset((IODBDMAChannelRegisters*)currentChannel);
        IOSetDBDMACommandPtr(currentChannel, savedState.savedDBDMAState[i].commandPtrLo);
        IOSetDBDMAInterruptSelect(currentChannel, savedState.savedDBDMAState[i].interruptSelect);
        IOSetDBDMABranchSelect(currentChannel, savedState.savedDBDMAState[i].branchSelect);
        IOSetDBDMAWaitSelect(currentChannel, savedState.savedDBDMAState[i].waitSelect);
    }
}

void AppleOHare::saveVIAState(void)
{
    UInt8* viaBase = (UInt8*) ohareBaseAddress + 0x16000;
    UInt8* savedViaState = savedState.savedVIAState;

    // Save VIA state.  These registers don't seem to get restored to any known state.
    savedViaState[0] = *(UInt8*)(viaBase + vBufA);
    savedViaState[1] = *(UInt8*)(viaBase + vDIRA);
    savedViaState[2] = *(UInt8*)(viaBase + vBufB);
    savedViaState[3] = *(UInt8*)(viaBase + vDIRB);
    savedViaState[4] = *(UInt8*)(viaBase + vPCR);
    savedViaState[5] = *(UInt8*)(viaBase + vACR);
    savedViaState[6] = *(UInt8*)(viaBase + vIER);
    savedViaState[7] = *(UInt8*)(viaBase + vT1C);
    savedViaState[8] = *(UInt8*)(viaBase + vT1CH);
}

void AppleOHare::restoreVIAState(void)
{
    UInt8* viaBase = (UInt8*) ohareBaseAddress + 0x16000;
    UInt8* savedViaState = savedState.savedVIAState;

    // Restore VIA state.  These registers don't seem to get restored to any known state.
    *(UInt8*)(viaBase + vBufA) = savedViaState[0];
    eieio();
    *(UInt8*)(viaBase + vDIRA) = savedViaState[1];
    eieio();
    *(UInt8*)(viaBase + vBufB) = savedViaState[2];
    eieio();
    *(UInt8*)(viaBase + vDIRB) = savedViaState[3];
    eieio();
    *(UInt8*)(viaBase + vPCR) = savedViaState[4];
    eieio();
    *(UInt8*)(viaBase + vACR) = savedViaState[5];
    eieio();
    *(UInt8*)(viaBase + vIER) = savedViaState[6];
    eieio();
    *(UInt8*)(viaBase + vT1C) = savedViaState[7];
    eieio();
    *(UInt8*)(viaBase + vT1CH) = savedViaState[8];
    eieio();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef  super
#define super IOInterruptController

OSDefineMetaClassAndStructors(AppleOHareInterruptController, IOInterruptController);

IOReturn AppleOHareInterruptController::initInterruptController(IOService *provider, IOLogicalAddress interruptControllerBase)
{
	int cnt;

	parentNub = provider;

	// Allocate the task lock.
	taskLock = IOLockAlloc();
	if (taskLock == 0) return kIOReturnNoResources;

	// Allocate the memory for the vectors
	vectors = (IOInterruptVector *)IOMalloc(kNumVectors * sizeof(IOInterruptVector));
	if (vectors == NULL) {
		IOLockFree(taskLock);
		return kIOReturnNoMemory;
	}
	bzero(vectors, kNumVectors * sizeof(IOInterruptVector));

	// Allocate locks for the
	for (cnt = 0; cnt < kNumVectors; cnt++) {
		vectors[cnt].interruptLock = IOLockAlloc();
		if (vectors[cnt].interruptLock == NULL) {
			for (cnt = 0; cnt < kNumVectors; cnt++) {
				IOLockFree(taskLock);
				if (vectors[cnt].interruptLock != NULL)
					IOLockFree(vectors[cnt].interruptLock);
			}
			return kIOReturnNoResources;
		}
	}

	// Setup the registers accessors
	eventsReg = (unsigned long)(interruptControllerBase + kEventsOffset);
	maskReg   = (unsigned long)(interruptControllerBase + kMaskOffset);
	clearReg  = (unsigned long)(interruptControllerBase + kClearOffset);
	levelsReg = (unsigned long)(interruptControllerBase + kLevelsOffset);

	// Initialize the registers.

	// Disable all interrupts.
	stwbrx(0x00000000, maskReg);
	eieio();

	// Clear all pending interrupts.
	stwbrx(0xFFFFFFFF, clearReg);
	eieio();

	// Disable all interrupts. (again?)
	stwbrx(0x00000000, maskReg);
	eieio();

	return kIOReturnSuccess;
}

IOInterruptAction AppleOHareInterruptController::getInterruptHandlerAddress(void)
{
	return (IOInterruptAction)&AppleOHareInterruptController::handleInterrupt;
}

IOReturn AppleOHareInterruptController::handleInterrupt(void * /*refCon*/,
IOService * /*nub*/,
int /*source*/)
{
	int               done;
	long              events, vectorNumber;
	IOInterruptVector *vector;
	unsigned long     maskTmp;

	do {
		done = 1;

		// Do all the sources for events, plus any pending interrupts.
  // Also add in the "level" sensitive sources
		maskTmp = lwbrx(maskReg);
		events = lwbrx(eventsReg) & ~kTypeLevelMask;
		events |= lwbrx(levelsReg) & maskTmp & kTypeLevelMask;
		events |= pendingEvents & maskTmp;
		pendingEvents = 0;
		eieio();

		// Since we have to clear the level'd one clear the current edge's too.
		stwbrx(kTypeLevelMask | events, clearReg);
		eieio();

		if (events) done = 0;

		while (events) {
			vectorNumber = 31 - cntlzw(events);
			events ^= (1 << vectorNumber);
			vector = &vectors[vectorNumber];

			vector->interruptActive = 1;
			sync();
			isync();
			if (!vector->interruptDisabledSoft) {
				isync();

				// Call the handler if it exists.
				if (vector->interruptRegistered) {
					vector->handler(vector->target, vector->refCon,
					 vector->nub, vector->source);
				}
			} else {
				// Hard disable the source.
				vector->interruptDisabledHard = 1;
				disableVectorHard(vectorNumber, vector);
			}

			vector->interruptActive = 0;
		}
	} while (!done);

	return kIOReturnSuccess;
}

bool AppleOHareInterruptController::vectorCanBeShared(long /*vectorNumber*/, IOInterruptVector */*vector*/)
{
	return true;
}

int AppleOHareInterruptController::getVectorType(long vectorNumber, IOInterruptVector */*vector*/)
{
	int interruptType;

	if (kTypeLevelMask & (1 << vectorNumber)) {
		interruptType = kIOInterruptTypeLevel;
	} else {
		interruptType = kIOInterruptTypeEdge;
	}

	return interruptType;
}

void AppleOHareInterruptController::disableVectorHard(long vectorNumber, IOInterruptVector */*vector*/)
{
	unsigned long     maskTmp;

	// Turn the source off at hardware.
	maskTmp = lwbrx(maskReg);
	maskTmp &= ~(1 << vectorNumber);
	stwbrx(maskTmp, maskReg);
	eieio();
}

void AppleOHareInterruptController::enableVector(long vectorNumber,
											IOInterruptVector *vector)
{
	unsigned long     maskTmp;

	maskTmp = lwbrx(maskReg);
	maskTmp |= (1 << vectorNumber);
	stwbrx(maskTmp, maskReg);
	eieio();
	if (lwbrx(levelsReg) & (1 << vectorNumber)) {
		// lost the interrupt
		causeVector(vectorNumber, vector);
	}
}

void AppleOHareInterruptController::causeVector(long vectorNumber,
										   IOInterruptVector */*vector*/)
{
	pendingEvents |= 1 << vectorNumber;
	parentNub->causeInterrupt(0);
}
										   

