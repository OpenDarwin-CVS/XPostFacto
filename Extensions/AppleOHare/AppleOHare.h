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

#ifndef _IOKIT_APPLEOHARE_H
#define _IOKIT_APPLEOHARE_H

#include <IOKit/platform/AppleMacIO.h>

#include <IOKit/IOInterrupts.h>
#include <IOKit/IOInterruptController.h>

#define kPrimaryOHare    (0)
#define kSecondaryOHare  (1)

#define kNumVectors      (32)

#define kTypeLevelMask   (0x1FF00000)

#define kEventsOffset    (0x00020)
#define kMaskOffset      (0x00024)
#define kClearOffset     (0x00028)
#define kLevelsOffset    (0x0002C)

class AppleOHareInterruptController;

class AppleOHare : public AppleMacIO
{
	OSDeclareDefaultStructors(AppleOHare);

private:
	IOLogicalAddress         ohareBaseAddress;
	long                     ohareNum;
	AppleOHareInterruptController *interruptController;

	virtual OSSymbol *getInterruptControllerName(void);
	
	/* Feature Control Register */

	enum {
		ohareFCTrans         = 24,                       // 0 = Transceiver On (for SCC ports)
		ohareFCMBPwr         = 25,                       // 1 = power off Media Bay
		ohareFCPCIMBEn       = 26,                       // 1 = enable PCI Media Bay
		ohareFCATAMBEn       = 27,                       // 1 = enable ATA Media Bay
		ohareFCFloppyEn      = 28,                       // 1 = enable floppy
		ohareFCATAINTEn      = 29,                       // 1 = enable internal ATA inputs
		ohareFCATA0Reset     = 30,                       // reset ATA0
		ohareFCMBReset       = 31,                       // reset Media Bay
		ohareFCIOBusEn       = 16,                       // IO Bus Enable
		ohareFCSCCCEn        = 17,                       // 0 = Stop SCC clock
		ohareFCSCSICEn       = 18,                       // 0 = Stop SCSE clock
		ohareFCSWIMCEn       = 19,                       // 0 = Stop SWIM clock
		ohareFCSndPwr        = 20,                       // 0 = power off to sound chip
		ohareFCSndClkEn      = 21,                       // 1 = enable external shift sound clock
		ohareFCSCCAEn        = 22,                       // 1 = enable SCCA
		ohareFCSCCBEn        = 23,                       // 1 = enable SCCB
		ohareFCVIAPort       = 8,                        // 1 = VIA functions in port mode
		ohareFCPWM           = 9,                        // 0 = turns off PWM counters
		ohareFCHookPB        = 10,                       // changes functions of IO pins
		ohareFCSWIM3         = 11,                       // changes functions of floppy pins
		ohareFCAud22         = 12,                       // 1 = SND_22M is running
		ohareFCSCSILink      = 13,                       //
		ohareFCArbByPass     = 14,                       // 1 = internal arbiter by passed
		ohareFCATA1Reset     = 15,                       //
		ohareFCSCCPClk       = 0,                        // 1 = SCC pClk forced low
		ohareFCResetSCC      = 1,                        // 1 = reset SCC cell

		ohareFCMediaBaybits  = (1<<ohareFCPCIMBEn)|(1<<ohareFCATAMBEn)|(1<<ohareFCFloppyEn),
		ohareFCMBlogical     = (1<<ohareFCMBPwr)|(1<<ohareFCMBReset)       // these bits are negative true logic
	};

	// register backup (to save the status before to sleep and restore
 // at wake).

    // 6522 VIA1 (and VIA2) register offsets
    enum
	{
		vBufB  =   0,        // BUFFER B
		vBufAH =   0x200,    // buffer a (with handshake) [ Dont use! ]
		vDIRB  =   0x400,    // DIRECTION B
		vDIRA  =   0x600,    // DIRECTION A
		vT1C   =   0x800,    // TIMER 1 COUNTER (L.O.)
		vT1CH  =   0xA00,    // timer 1 counter (high order)
		vT1L   =   0xC00,    // TIMER 1 LATCH (L.O.)
		vT1LH  =   0xE00,    // timer 1 latch (high order)
		vT2C   =   0x1000,   // TIMER 2 LATCH (L.O.)
		vT2CH  =   0x1200,   // timer 2 counter (high order)
		vSR    =   0x1400,   // SHIFT REGISTER
		vACR   =   0x1600,   // AUX. CONTROL REG.
		vPCR   =   0x1800,   // PERIPH. CONTROL REG.
		vIFR   =   0x1A00,   // INT. FLAG REG.
		vIER   =   0x1C00,   // INT. ENABLE REG.
		vBufA  =   0x1E00,   // BUFFER A
		vBufD  =   vBufA     // disk head select is buffer A
	};

    // This is a short version of the IODBDMAChannelRegisters which includes only
    // the registers we actually mean to save
    struct DBDMAChannelRegisters {
        UInt32 	commandPtrLo;
        UInt32 	interruptSelect;
        UInt32 	branchSelect;
        UInt32 	waitSelect;
    };
    typedef struct DBDMAChannelRegisters DBDMAChannelRegisters;
    typedef volatile DBDMAChannelRegisters *DBDMAChannelRegistersPtr;

    struct OHareState {
        bool                thisStateIsValid;
        UInt32				interruptMask;
        UInt32				featureControlReg;
        UInt32				auxControlReg;
        DBDMAChannelRegisters		savedDBDMAState[12];
        UInt8				savedVIAState[9];
    };
    typedef struct OHareState OHareState;
    OHareState savedState;	

	const OSSymbol      *ohare_powerMediaBay;
	const OSSymbol 		*ohare_sleepState;
	bool 				mediaIsOn;

public:
	virtual bool start(IOService *provider);

	virtual bool passiveMatch (OSDictionary * matching, bool changesOK = false);

   virtual IOReturn callPlatformFunction(const OSSymbol *functionName,
									   bool waitForFunction,
									   void *param1, void *param2,
									   void *param3, void *param4);
	virtual void powerMediaBay(bool powerOn, UInt8 powerDevice);
	virtual void enableMBATA(void);
	virtual void processNub(IOService *nub);


	// PM MEthods:
	void initForPM (IOService *provider);
	IOReturn setPowerState(unsigned long powerStateOrdinal, IOService* whatDevice);

	// Sleep/Wake methods:
	virtual void sleepState(bool sleepMe);
	virtual void saveVIAState(void);
	virtual void restoreVIAState(void);
	virtual void saveDMAState(void);
	virtual void restoreDMAState(void);
	virtual void saveGPState(void);
	virtual void restoreGPState(void);
	virtual void saveInterruptState(void);
	virtual void restoreInterruptState(void);

};

class AppleOHareInterruptController : public IOInterruptController
{
	OSDeclareDefaultStructors(AppleOHareInterruptController);

private:
	IOService         *parentNub;
	IOLock            *taskLock;
	unsigned long     pendingEvents;
	unsigned long     eventsReg;
	unsigned long     maskReg;
	unsigned long     clearReg;
	unsigned long     levelsReg;

public:
		virtual IOReturn initInterruptController(IOService *provider,
										   IOLogicalAddress interruptControllerBase);

	virtual IOInterruptAction getInterruptHandlerAddress(void);
	virtual IOReturn handleInterrupt(void *refCon, IOService *nub, int source);

	virtual bool vectorCanBeShared(long vectorNumber, IOInterruptVector *vector);
	virtual int  getVectorType(long vectorNumber, IOInterruptVector *vector);
	virtual void disableVectorHard(long vectorNumber, IOInterruptVector *vector);
	virtual void enableVector(long vectorNumber, IOInterruptVector *vector);
	virtual void causeVector(long vectorNumber, IOInterruptVector *vector);
};


#endif /* ! _APPLE_OHARE_H */
