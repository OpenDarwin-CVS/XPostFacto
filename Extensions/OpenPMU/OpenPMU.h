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

#ifndef APPLEPMU_H
#define APPLEPMU_H

#include <IOKit/IOService.h>
#include <IOKit/IOTypes.h>

#ifdef __cplusplus
extern "C" {
#include <pexpert/pexpert.h>
}
#endif

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOPlatformExpert.h>

#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOLocks.h>
#include <IOKit/adb/adb.h>

#include "OpenViaInterface.h"

// Uncomment the following line to get verbose logs of the PMU activity:
//#define VERBOSE_LOGS_ON_PMU_INT
//#define VERBOSE_LOGS_ON_PMU

// Uncomment the following line to change the pmu behavior when handling adb
// commands. To be more precise. If the following define is commented the adb
// messages (0x20) will be handled like all the other messages. if it is
// uncommented the pmu will hold the process of new messages until the adb
// transaction is completed (which happens at the first adb interrupt 0x10).
// #define ADB_COMMANDS_HOLD_ALL

// **********************************************************************************
// bits in response to kPMUReadInt command
// **********************************************************************************
typedef enum {
    kPMUMD0Int 		= 0x01,   // interrupt type 0 (machine-specific)
    kPMUMD1Int 		= 0x02,   // interrupt type 1 (machine-specific)
    kPMUpcmicia 	= 0x04,   // pcmcia (buttons and timeout-eject)
    kPMUbrightnessInt 	= 0x08,   // brightness button has been pressed, value changed
    kPMUADBint 		= 0x10,   // ADB
    kPMUbattInt         = 0x20,   // battery
    kPMUenvironmentInt 	= 0x40,   // environment
    kPMUoneSecInt       = 0x80    // one second interrupt
} interruptType;

enum {					  // when kPMUADBint is set
    kPMUwaitinglsc	= 0x01,	  // waiting to listen to charger
    kPMUautoSRQpolling	= 0x02,	  // auto/SRQ polling is enabled  
    kPMUautopoll	= 0x04	  // input is autopoll data
};

// **********************************************************************************
// kPMUpowerUpEvents command sub-types
// **********************************************************************************
enum {
    kPMUgetPowerUpEvents	= 0x00,
    kPMUsetPowerUpEvents	= 0x01,
    kPMUclearPowerUpEvents	= 0x02,
    kPMUgetWakeUpEvents		= 0x03,
    kPMUsetWakeUpEvents		= 0x04,
    kPMUclearWakeUpEvents	= 0x05
};


// **********************************************************************************
// bits which tell PMU why to wake up the system
// **********************************************************************************
enum {
    kPMUwakeUpOnKey		= 0x01,
    kPMUwakeUpOnACInsert	= 0x02,
    kPMUwakeUpOnACChanged	= 0x04,
    kPMUwakeUpOnLidOpen		= 0x08,
    kPMUwakeUpOnRing		= 0x10
};

enum {
    kPMUADBAddressField = 4
};

enum {
    kPMUResetADBBus	= 0x00,
    kPMUFlushADB	= 0x01,
    kPMUWriteADB	= 0x08,
    kPMUReadADB         = 0x0C,
    kPMURWMaskADB	= 0x0C
};

// **********************************************************************************
// pmu error messages
// **********************************************************************************
enum {
    kPMUNoError         = 0,
    kPMUInitError       = 1,    // PMU failed to initialize
    kPMUParameterError  = 2,    // Bad parameters
    kPMUNotSupported    = 3,    // PMU don't do that (Cuda does, though)
    kPMUIOError         = 4     // Nonspecific I/O failure
    };

// **********************************************************************************
// pmu commands
// **********************************************************************************

enum {
    kPMUpowerCntl	= 0x10,		// power plane/clock control
    kPMUpower1Cntl	= 0x11,		// more power control (DBLite)
    kPMUpowerRead	= 0x18,		// power plane/clock status
    kPMUpower1Read	= 0x19,		// more power status (DBLite)

    kPMUpMgrADB	= 0x20, 		// send ADB command
    kPMUpMgrADBoff	= 0x21, 		// turn ADB auto-poll off
    kPMUreadADB		= 0x28, 		// Apple Desktop Bus
    kPMUpMgrADBInt	= 0x2F, 		// get ADB interrupt data (Portable only)

    kPMUtimeWrite	= 0x30, 		// write the time to the clock chip
    kPMUpramWrite	= 0x31, 		// write the original 20 bytes of PRAM (Portable only)
    kPMUxPramWrite	 = 0x32, 		// write extended PRAM byte(s)
    kPMUNVRAMWrite	= 0x33,		// write NVRAM byte
    kPMUtimeRead		= 0x38, 		// read the time from the clock chip
    kPMUpramRead		= 0x39, 		// read the original 20 bytes of PRAM (Portable only)
    kPMUxPramRead	= 0x3A, 		// read extended PRAM byte(s)
    kPMUNVRAMRead	= 0x3B, 		// read NVRAM byte
    kPMUSetContrast	= 0x40,		// set screen contrast
    kPMUSetBrightness	= 0x41,		// set screen brightness
    kPMUReadContrast	= 0x48,		// read the contrast value
    kPMUReadBrightness	= 0x49,		// read the brightness value
    kPMUDoPCMCIAEject	= 0x4C,		// eject PCMCIA card(s)
    kPMUDoMediaBayDisp	= 0x4D,		// (MS 5/17/96) Get Media bay device status
    kPMUDisplayDisp	= 0x4F,		// Get raw Contrast numbers
    kPMUmodemSet	= 0x50,		// internal modem control
    kPMUmodemClrFIFO	= 0x51,		// clear modem fifo's
    kPMUmodemSetFIFOIntMask	= 0x52,	// set the mask for fifo interrupts
    kPMUmodemWriteData		= 0x54,	// write data to modem
    kPMUmodemSetDataMode	= 0x55,	//
    kPMUmodemSetFloCtlMode	= 0x56,	//
    kPMUmodemDAACnt		= 0x57,	//
    kPMUmodemRead		= 0x58,	// internal modem status
    kPMUmodemDAAID		= 0x59,	//
    kPMUmodemGetFIFOCnt	= 0x5A,	//
    kPMUmodemSetMaxFIFOSize	= 0x5B,	//
    kPMUmodemReadFIFOData	 = 0x5C,	//
    kPMUmodemExtend		= 0x5D,	//

    kPMUsetBattWarning	= 0x60,		// set low power warning and cutoff battery levels (PB 140/170, DBLite)
    kPMUsetCutoff		= 0x61,		// set hardware cutoff voltage<H44>
    kPMUnewSetBattWarn	= 0x62,		// set low power warning and 10 second battery levels (Epic/Mustang)
    kPMUnewGetBattWarn	= 0x63,		// get low power warning and 10 second battery levels (Epic/Mustang)
    kPMUbatteryRead	= 0x68,		// read battery/charger level and status
    kPMUbatteryNow	= 0x69,		// read battery/charger instantaneous level and status
    kPMUreadBattWarning	= 0x6A,		// read low power warning and cutoff battery levels (PB 140/170, DBLite)
    kPMUreadExtBatt	= 0x6B,		// read extended battery/charger level and status (DBLite)
    kPMUreadBatteryID	= 0x6C,		// read the battery ID
    kPMUreadBatteryInfo	= 0x6D,		// return battery parameters
    kPMUGetSOB		= 0x6F,		// Get Smarts of Battery

    kPMUSetModem1SecInt	= 0x70,		//
    kPMUSetModemInts	= 0x71,		// turn modem interrupts on/off
    kPMUreadINT		= 0x78,		// get PMGR interrupt data
    kPMUReadModemInts	= 0x79,		// read modem interrupt status
    kPMUPmgrPWRoff	= 0x7E,		// turn system power off
    kPMUsleepReq	= 0x7F,		// put the system to sleep (sleepSig='MATT')
    kPMUsleepAck	= 0x70,		// sleep acknowledge

    kPMUtimerSet	= 0x80,		// set the wakeup timer
    kPMUtimerDisable	= 0x82,		// disable wakeup timer
   
    kPMUtimerRead	= 0x88,		// read the wakeup timer setting
    kPMUpowerUpEvents	= 0x8F,		// get/set events that wake/power-up machine

    kPMUsoundSet		= 0x90,		// sound power control
    kPMUSetDFAC		= 0x91,		// set DFAC register (DBLite)
    kPMUsoundRead	= 0x98,		// read sound power state
    kPMUReadDFAC	= 0x99,		// read DFAC register (DBLite)
    kPMUI2CCmd		= 0x9A,		// read / write IIC

    kPMUmodemWriteReg		= 0xA0,	// Write Modem Register
    kPMUmodemClrRegBits		= 0xA1,	// Clear Modem Register Bits
    kPMUmodemSetRegBits		= 0xA2,	// Set Modem Register Bits
    kPMUmodemWriteDSPRam	= 0xA3,	// Write DSP RAM
    kPMUmodemSetFilterCoeff	= 0xA4,	// Set Filter Coefficients
    kPMUmodemReset		= 0xA5,	// Reset Modem
    kPMUmodemUNKNOWN	= 0xA6,	// <filler for now>
    kPMUmodemReadReg		= 0xA8,	// Read Modem Register
    kPMUmodemReadDSPRam	= 0xAB,	// Read DSP RAM

    kPMUresetCPU		= 0xD0,		// reset the CPU
    kPMUreadAtoD		= 0xD8,		// read A/D channel
    kPMUreadButton	= 0xD9,		// read button values on Channel 0 = Brightness, Channel 1 = Contrast 0-31
    kPMUreadExtSwitches	= 0xDC,		// read external switch status (DBLite)
    kPMUsystemReady		= 0xDF,		// system is fully powered up/awake

    kPMUwritePmgrRAM	= 0xE0,		// write to internal PMGR RAM
    kPMUdownloadFlash	= 0xE1,		// download Flash memory
    kPMUdownloadStatus	= 0xE2,		// PRAM status
    kPMUsetMachineAttr	= 0xE3,		// set machine id
    kPMUreadPmgrRAM	= 0xE8,		// read from internal PMGR RAM
    kPMUreadPmgrVers	= 0xEA,		// read the PMGR version number
    kPMUreadMachineAttr	= 0xEB,		// read the machine id
    kPMUPmgrSelfTest	= 0xEC,		// run the PMGR selftest
    kPMUDBPMgrTest	= 0xED,		// DON'T USE THIS!!
    kPMUFactoryTest	= 0xEE,		// hook for factory requests
    kPMUPmgrSoftReset	= 0xEF 		// soft reset of the PMGR
};

// Forward class delcarations for the common clients of the OpenPMU:
// (since their headers include OpenPMU.h we have to resort to this)
// class OpenPMUADBController;
class OpenPMUNVRAMController;
class OpenPMURTCController;
class OpenPMUPwrController;

// Method names for the callPlatformFunction:
#define kSendMiscCommand "sendMiscCommand"
#define kRegisterForPMUInterrupts "registerForPMUInterrupts"
#define kDeRegisterClient "deRegisterClient"
#define kSetLCDPower "setLCDPower"
#define kSetHDPower "setHDPower"
#define kSetMediaBayPower "setMediaBayPower"
#define kSetIRPower "setIRPower"
#define kSleepNow "sleepNow"

// The number of arguments for kSendMiscCommand is short of one, so
// I need to define a paramterblock. The fields are the same of the
// OpenPMUInterface::sendMiscCommand method.
typedef struct SendMiscCommandParameterBlock {
    int command;
    IOByteCount sLength;
    UInt8 *sBuffer;
    IOByteCount *rLength;
    UInt8 *rBuffer;
} SendMiscCommandParameterBlock;
typedef SendMiscCommandParameterBlock *SendMiscCommandParameterBlockPtr;

// =====================================================================================
// PMU Interface:
// =====================================================================================

// OpenPMU class definition. This is the class that actually handles the
// communication with the PMU. It provides a entry point for the common
// PMU clients and handles the setup/cleanup for sleep and wake. This
// class does not touch the hardware, it must always refer to a VIAInterface
// object.

// Clients get call backs with this type of function:
typedef void (*OpenPMUClient)(IOService * client,UInt8 matchingMask, UInt32 length, UInt8 * buffer); 


// RULE: OpenPMU does not touch the hardware directly.

class OpenPMUInterface : public IOService
{
    OSDeclareDefaultStructors(OpenPMUInterface)

protected:
    // De-Syncers: they make up for the lack of thread_func_call()
    // -----------------------------------------------------------

    // This is the kind of function we expect ot call:
    typedef void (*FunctionType)(void *, void *);

    // When we request a call on a separate thread we create
    // this set of parameters:
    typedef struct ThreadParameters {
        thread_call_t workThread;
        FunctionType targetFunction;
        void *functionParameters;
    } ThreadParameters;
    typedef ThreadParameters *ThreadParametersPtr;

    // this will be the PMU:
    void CallFuncDesync(FunctionType func, void* parameters, AbsoluteTime *delay = NULL);

    // This is instad a convinient call that we use as "wrapper"
    static void CallAndFreeThread(ThreadParametersPtr myParameters);
    
    // Client handling methods and functions:
    // --------------------------------------
    
    // The pmu clients register with the driver to be notifyed
    // when some events (interrupts) occur. The driver has so
    // to keep a list of all the clients:
    typedef struct PMUClient {
        UInt8	         interruptMask;
        IOService        *client;
        OpenPMUClient   callBackFunction;
        struct PMUClient *nextClient;
    } PMUClient;
    typedef PMUClient *PMUClientPtr;

    // This is the top of the list:
    PMUClientPtr listHead;

    // This lock protects the access to the clients
    // list:
    IOLock *clientMutexLock;

    // Adds a client to the list:
    bool addPMUClient(UInt8 interruptMask, OpenPMUClient function, IOService * caller);

    // Removes a client to the list:
    bool removePMUClient(IOService * caller, UInt8 interruptMask);

    // Removes all clients from the list:
    bool clearPMUClientList();

    // Calls a client in the list with the data from the current interrupt:
    bool callPMUClient(UInt8 interruptMask, UInt32 length, UInt8 * buffer);

    // Desyncers structures and methods:
    // ---------------------------------
    
    // Sice the calls to the clients are detached from the
    // workloop (remember I am trying to keep the workloop
    // running all the time and blocked as little as possible,
    // so I am de-syncing all the possible calls to and from
    // the WRKL. This structure will allow me to transfer the
    // parameters I need to the level above.
    typedef struct OutputCallParameters {
        OpenPMUInterface *myThis;
        UInt8  interruptType;
        UInt32 dataLength;
        UInt8  buffer[256];
    } OutputCallParameters;
    typedef OutputCallParameters *OutputCallParametersPtr;

    // Desyncer from PMU to clients: calls the right client:
    static void clientNotifyData(void *castMeToOutputCallParametersPtr, void *n);

    // The input desyncer also requires a small structure. Mostly because
    // it needs to remember if the data thet is received needs to be freed
    // (in case of asyncronous calls) or kept (for syncronous calls).
    typedef struct InputCallParameters {
        OpenPMUInterface *myThis;
        bool remebemberToFreeMe;
        PMUrequest theRequest;
    } InputCallParameters;
    typedef InputCallParameters *InputCallParametersPtr;
    
    // Desyncer from clients to PMU: enques to the via interface:
    static void enqueueData(void *castMeToInputCallParametersPtr, void *n);

    // Interrupt handling:
    // -------------------
    
    // Interrupt vectors:
    enum {
        VIA_DEV_VIA0 = 2,
        VIA_DEV_VIA2 = 4
    };

    // Polling interval for environments in old hardware:
    // the interval is expressed in seconds.
    enum {
        pollingInterval = 3
    };

    // Polling divider for old hardware:
    int pollingDivider;

    // This keeps track of the state of the interrupts:
    bool interruptsAreOn;
    
    IOInterruptEventSource  *pmuInterrupt;     // The interrupt
    static void pmuInterruptCaller(OSObject * PMUdriver, IOInterruptEventSource *src, int intr);

    // Handles all the interrupts that the pmu is waiting to dispatch.
    void interruptHandler();
    
    // dispatches the interrupt to the correct handler:
    void pmuInterruptDispatcher(UInt8 *buffer, UInt32 bufLen);

    // checks for the ststus of those devices that can affect the power state
    void checkPowerEnvironmentEvents(void);

    // Command gate to enqueue data to the VIA:
    // ----------------------------------------
    
    // This locks assures that the clients can access to the driver services
    // only when the client is free to run. I can not count on the workloop
    // coomand gate (or queue for that matter) because I need to keep the
    // workloop running free (and those methods lock the workloop).
    IOLock *syncLock;

    // This takes in account which is the command that is holding the
    // pmu transactions.
    int lockingCommand;
    
    IOCommandGate *commandGate;      // The command gate
    static IOReturn pmuCommandGateCaller(OSObject *object, void *arg0, void *arg1, void *arg2, void *arg3r);
    static IOReturn pmuCommandGateReFlash(OSObject *object, void *castMeToFirmware, void *arg1, void *arg2, void *arg3r);

    // transfers a request to the VIA.
    bool pmuTansferRequestToVIA(PMUrequest *pmuReq);

    // Creates a request and enque to the via interface:
    bool createAndEnqueueRequest(bool syncronousCall, UInt32 commandCode, IOByteCount  sLength, UInt8* sBuffer, IOByteCount* rLength, UInt8* rBuffer);

    // This defines the state of the PMU driver:
    // If the driver is busy it meas that it is running but It can not enqueue
    // commands. The second argument is to keep track of which command is keeping
    // the driver busy.
    void setPMUDriverBusy(bool isBusy, int command);
    
    // Misc stuff, workloop hw interfaces...
    // -------------------------------------

    // The driver needs a workloop with 3 type of event sources
    // These are the ONLY means to enque a command to the VIA
    // interface.
    IOWorkLoop *workLoop;         // The workloop:
         
    // Here the interface with the VIA registers:
    OpenViaInterface *theHWInterface;

    // This is to know on which class of machine we
    // are actually running.
    bool isOldHardware;

    // This is to remember if the clients asked for a wake on ring:
    bool wakeOnRing;

public:
    // Generic IOService stuff:
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    virtual void free(void);

    // Common place to release all the allocated resources:
    virtual bool freeAllResources();

    // This allows "uncommon" clients to send messages to the pmu
    // (the most common case would be the backlight driver)
    IOReturn sendMiscCommand (int Command, IOByteCount SLength, UInt8 *SBuffer, IOByteCount *RLength, UInt8 *RBuffer);

    // If an other driver wishes to be aware of pmu transactions (or)
    // interrupts it has to register with the pmu driver:
    // Note that clients have rules to follow: the most important is
    // that they should NEVER change the conter of the buffer status
    // they receive from the pmu driver.
    bool registerForPMUInterrupts(UInt8 interruptMask, OpenPMUClient function, IOService * caller);

    // This is tp de-register from the clients that  wish to be aware of pmu transactions
    bool deRegisterClient(IOService * caller, UInt8 interruptMask);

    // System Power commands:
    // ----------------------

    // Sets the file-server mode on and off (default is on):
    void setFileServerMode(bool fileServerModeON);

    // Defines if the system is supposed to wake up on ring (default is off):
    void setWakeOnRing(bool wakeOnRingON);
    
    // sets the conditions to wake and tells to the pmu to put the machine in sleep mode
    void putMachineToSleep();

    // powers off everything that may need to be powerd off from here
    // and set the wake up conditions.
    void preSleepSequence();
        
    // sets the machine in awake mode.
    void wakeUp();

    // reset the machine.
    void rebootSystem();

    //  reset the machine.
    void shutdownSystem();
    
    // Macro commands:
    // ---------------
    
    // Defines the state of the system:
    void saySystemReady();

    // Enable/disables Interrupts:
    void setInterrupts(bool on);

    // Clears all the pending interrupts:
    void clearInterrupts();
        
    // Turns the display on and off:
    void setLCDPower(bool on);
    
    // Turns the HD on and off:
    void setHDPower(bool on);
    
    // Turns power on and off to the
    // media bay:
    void setMediaBayPower(bool on);

    // Turns power on and off to the
    // media bay:
    void setIRPower(bool on);

    // Download a new firmware in the PMU:
    // the argument is an OSData object that
    // contains the new PMU object. Note that
    // the data MUST already start with the
    // standard signature 'BORG' and have
    // the correct length, or the download
    // will fail.
    IOReturn downloadFirmware(OSData *newFirmare);

    // This call probaly should be moved in the platform
    // expert since it is likely that more tha one driver
    // will have a different default behavior if runs
    // on mobile (powerbooks) or desktop machines.
    // retutns true if on powerbooks.
    bool hostIsMobile();

    // Overides the standard callPlatformFunction to catch all the calls
    // that may be diected to the PMU
    virtual IOReturn callPlatformFunction( const OSSymbol *functionName,
                                           bool waitForFunction,
                                           void *param1, void *param2,
                                           void *param3, void *param4 );
};


// The OpenPMU class inherits from OpenPMUInterface and expands the interface to
// add the legacy interfaces and to support those extra functionality like the sleep
// commands. Setting the wake condtions and so on.
class OpenPMU : public OpenPMUInterface
{
     OSDeclareDefaultStructors(OpenPMU)

protected:
    // Since the PMU "knows" which services provides (for example not
    // all machines have the nvram serviced by the PMU) it is the
    // PMU job to create the necessary interfaces and attach them
    // to its node. We keep here an handly copy of the interfaces:
//    OpenPMUADBController 		*ourADBinterface;
    OpenPMUNVRAMController	*ourNVRAMinterface;
    OpenPMUPwrController		*ourPwrinterface;
    OpenPMURTCController          *ourRTCinterface;

    // Platform expert interfaces:
    // this sets upt the common pointers for the PE
    // functions.
    bool setupPlatformExpertInterfaces();
    bool disablePlatformExpertInterfaces();

    // these are the actual methods that interface with PE:
    static OpenPMU *applePMUReference;
    static int OpenPMU_pmu_PE_poll_input ( unsigned int, char *  );
    static int OpenPMU_pmu_PE_halt_restart ( unsigned int type );
    static int OpenPMU_pmu_PE_write_IIC ( unsigned char, unsigned char, unsigned char );
    static int OpenPMU_pmu_PE_read_write_time_of_day ( unsigned int, long * );

    // Async caller for the allocateInterfaces.
    static void allocateInterfacesCaller( thread_call_param_t arg, thread_call_param_t);
        
public:
    // Generic IOService stuff:
    // ------------------------
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);

	// Override to match ApplePMU
	// --------------------------
	virtual bool passiveMatch (OSDictionary * matching, bool changesOK = false);

    // Handles to manage the power changes of the root domain
    // ------------------------------------------------------
    IOReturn powerStateWillChangeTo (IOPMPowerFlags theFlags, unsigned long, IOService*);
    IOReturn powerStateDidChangeTo ( IOPMPowerFlags theFlags, unsigned long, IOService*); 

    // Allocator/creator for the clients and the simmetric de-allocator:
    // -----------------------------------------------------------------
    bool allocateInterfaces(void);
    bool freeInterfaces(void);

    // User client creator:
    // -----------------------------------------------------------------
    IOReturn newUserClient(task_t owningTask, void*,     // Security id (?!)
                                 UInt32        type,     // Magic number
                                 IOUserClient  **handler);

	// probe
	IOService * probe(	IOService * 	provider,
				SInt32 	  *	score );

};

#endif /* ! APPLEPMU_H */



