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

#ifndef APPLEVIAINTERFACE_H
#define APPLEVIAINTERFACE_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOSyncer.h>

// Uncomment the following line to get verbose logs of the VIA activity:
//#define VERBOSE_LOGS_ON_VIA
//#define VERBOSE_LOGS_ON_PMU_INT

// Uncomment the following line to change the pmu behavior when handling adb
// commands. To be more precise. If the following define is commented the adb
// messages (0x20) will be handled like all the other messages. if it is
// uncommented the pmu will hold the process of new messages until the adb
// transaction is completed (which happens at the first adb interrupt 0x10).
// #define ADB_COMMANDS_HOLD_ALL

// **********************************************************************************
// VIA definitions
// **********************************************************************************
enum {
    // M2 uses VIA2
    M2Req = 2,			      	// Power manager handshake request
    M2Ack = 1,				// Power manager handshake acknowledge

    // Hooper uses VIA1
    HooperReq = 4,			      	// request
    HooperAck = 3				// acknowledge
};

enum {					        // IFR/IER
    ifCA2 = 0,				// CA2 interrupt
    ifCA1 = 1,				// CA1 interrupt
    ifSR  = 2,				// SR shift register done
    ifCB2 = 3,				// CB2 interrupt
    ifCB1 = 4,				// CB1 interrupt
    ifT2  = 5,				// T2 timer2 interrupt
    ifT1  = 6,				// T1 timer1 interrupt
    ifIRQ = 7				// any interrupt
};

// The interface with the core of the driver (the part that actually writes to
// the PMU) is build around a transfer. This is the structure that holds an atomic
// transfer:
typedef struct PMUrequest {
    UInt32		pmCommand;		// PMU Command
    UInt32		pmSLength;		// data length (out)
    UInt8		pmSBuffer[256];		// data buffer (out)
    UInt32		pmRLength;		// data length (in)
    UInt8		pmRBuffer[256];		// data buffer (in)
} PMUrequest;
typedef PMUrequest* PMUrequestPtr;

// =====================================================================================
// VIA Interfaces:
// =====================================================================================

// This class provides the interface with the VIA registers. and processes the
// requests from the PMU.
class OpenViaInterface : public IOService
{
    OSDeclareDefaultStructors(OpenViaInterface)

protected: // protected DATA:
    // This is the VIA interface:
    typedef volatile UInt8  *VIAAddress;	// This is an address on the bus

    // This is the actual VIA interface
    VIAAddress VIA1_shift;              // shift register address:
    VIAAddress VIA1_auxillaryControl;   // mostly to define the direction of the data.
    VIAAddress VIA1_interruptFlag;      // interrupt status and acknowledgment
    VIAAddress VIA1_interruptEnable;	// interrupt enabling.
    VIAAddress VIA2_dataB;		        // misc data ack bits.

    // These bits depend of which interface we are using, so we got to store
    // them somewhere.
    UInt8		PMreq;                  // req bit
    UInt8		PMack;                  // ack bit.

private: // private DATA
    // This is to enforce the exclusivity access to the hardware. A workloop
    // for the services provided by OpenViaInterface would ber overkilling
    // since the class is a basically providing a simple API to access to the
    // VIA functionality. The reason for having the lock provate it is described
    // below (in the lock methods comment).
    IOLock *mutex;
	
	// In Tiger, we can't link to disable_preemption and enable_preemption any more.
	// But we can get a similar effect with a simple lock
	IOSimpleLock *preemptionMutex;

    // This variable is set to remember if we can use kernel resources (as timers
    // and locks) or if we have to do without:
    bool theKernelIsUp;
    
protected: // protected METHODS
    // Returns if the kernel can be trusted:
    bool isTheKernelUp();
        
    // In future I may decide to implement the locking in a
    // different way, so I'm going to add here the functions
    // to access the lock:
    void takeVIALock();
    void releaseVIALock();

    // These 3 functions are used as part of the internal engine
    // of the VIA interface. They MUST not been made public since
    // they are not directly protected by the mutex lock.
    virtual bool sendByte(char byte);
    virtual bool readByte(char *byte);
    virtual bool waitForAck(bool mode, UInt32 milliseconds);

    // Enables and disables the shift register
    // interrupt. (not very useful in a polled
    // driver).
    virtual void disableSRInterrupt ( void );
    virtual void enableSRInterrupt ( void );
    virtual bool srInteruptPending(void);
    
public:
    // Generic IOService stuff:
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    virtual void free(void);

    // methods to setup the hardware:
    virtual bool hwInit(UInt8 *baseAddress);
    virtual bool hwRelease(void);
    virtual bool hwIsReady(void);

    // this code should be albe to run with and without
    // support from the kernel. So the following variable
    // tells if the kerenel is up and usable:
    virtual void trustTheKernel(bool trustIt);
    
    // methods to interface with the PMU driver:
    virtual bool processPMURequest(PMUrequestPtr plugInMessage);

    // methods to interface with the PMU hardware:
    virtual void disablePMUInterrupt ( void );
    virtual void enablePMUInterrupt ( void );
    virtual void acknowledgePMUInterrupt ( void );
    virtual bool pmuInteruptPending(void);

    // re-flashes the pmu firmware:
    virtual bool downloadMicroCode(UInt8 *microCodeBlock, UInt32 length);
};

// This is a subclass of ApplePolledViaInterface
// same interface but interrupt driven instead than using the
// polling mechanism.
class OpenIntrrViaInterface : public OpenViaInterface
{
    OSDeclareDefaultStructors(OpenIntrrViaInterface)
private:
    // Interrupt vectors:
    enum {
        VIA_DEV_VIA0 = 2,
        VIA_DEV_VIA2 = 4
    };

    // Remember here who is the source of the interrupts:
    IOService *interruptSource;

    // These are the possible states for the via interface:
    typedef enum InterruptState {
        kInterfaceIdle = 0,
        kSendCommand,
        kSendLenght,
        kSendData,
        kSwitchToRead,
        kReadLenght,
        kReadData
    } InterruptState;

    // And this is the state holder:
    typedef struct ViaInterfaceState {
        InterruptState currentInterruptState;
        UInt32         numberOfTransferedBytes;
        UInt32         numberOfBytesToBeTransfered;
        PMUrequestPtr  currentTransfer;
        bool           success;
    } ViaInterfaceState;
    typedef ViaInterfaceState *ViaInterfaceStatePtr;

    // Placeholder for the current state:
    ViaInterfaceState transferState;

    // Syncronizer:
    volatile semaphore_t mySync;

    // This is the real interrupt handler:
    static void shiftRegisterInt (OSObject *castMeToOpenIntrrViaInterface, IOInterruptEventSource *, int);

protected: // protected METHODS
    // Enables and disables the shift register
    // interrupt. Expands the same functions
    // of the polling driver to involve the
    // provider interface.
    virtual void disableSRInterrupt ( void );
    virtual void enableSRInterrupt ( void );

    // in future I may wish to implement the syncer in a different way
    // so for mow I'll wrap it around two calls:
    void prepareSync();
    void waitForSync();
    void sigTheSync();

    // This guy initiates the transfer:
    bool sendToPMU(PMUrequestPtr theRequest);

    // This method knowing the current InterruptState (it is the
    // argument), and the next interrupt state (which MUST be alresdy
    // in transferState) performs the correct set of actions.
    void actUponState();

    // byte-moving methods, specific for the interrupt mode:
    void sendIntrByte(char byte);
    char readIntrByte();

public:
    // Generic IOService stuff:
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);

    // methods to interface with the PMU driver:
    virtual bool processPMURequest(PMUrequestPtr plugInMessage);
};

#endif /* ! APPLEVIAINTERFACE_H */



