 
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

#include "OpenViaInterface.h"
#include "OpenPMU.h"

// all the table definitions and the variables
// for the PMU:
#include "OpenPMUTables.h"

// Uncomment the following define to trace the PMU/VIA code:
// #define TRACE 1

extern "C" {
	void disable_preemption();
	void enable_preemption();
}

// =====================================================================================
// VIA Interfaces: Polled
// =====================================================================================

#undef super
#define super IOService
OSDefineMetaClassAndStructors(OpenViaInterface,IOService)

// Method: start
//
// Purpose:
//   Hardware was detected and initialized, start the driver. Actually
//   we do not do much here. We just check  that the provider is of the
//   right type and we setup the registers.
bool
OpenViaInterface::start(IOService *provider)
{
    // As usual we call the parent start:
    bool success = super::start(provider);

    if (success) {
        // And we get ready to setup the hardware
        OpenPMUInterface *pmuParent = OSDynamicCast(OpenPMUInterface, provider);

        // if my parent is not what I am expecting exit complaining:
        if (pmuParent == NULL)
            return false;

        // Sets all the pointers to NULL, since the real
        // resource allocation is in hwInit()
        mutex = NULL;

        // Since we are in a kernel tread (the one that starts the
        // IOKit resources) we can say that from now on we can use
        // kernel resources:
        trustTheKernel(true);

        // Init the hardware to NULL, just so we know it is in
        // a clean state.
        hwRelease();
    }

    return success;
}

// --------------------------------------------------------------------------
//
// Method: stop
//
// Purpose:
//   Release all driver resources.
void
OpenViaInterface::stop(IOService *provider)
{
    // Not much to do after all:
    hwRelease();

    // And the super:
    super::stop(provider);
}

// --------------------------------------------------------------------------
//
// Method: free
//
// Purpose:
//   This also release all driver resources. We need to do this in both stop
//   and free so that we are sure that if start fails we still release all the
//   resources we allocated.
void
OpenViaInterface::free(void)
{
    // Not much to do after all:
    hwRelease();

    // And the super:
    super::free();
}

// --------------------------------------------------------------------------
//
// Method: hwInit
//
// Purpose:
//    This is the common function used to setup the hardware
bool
OpenViaInterface::hwInit(UInt8 *baseAddress)
{
    if (baseAddress != NULL) {
        VIA1_shift            = baseAddress + 0x1400;	// shift register address
        VIA1_auxillaryControl = baseAddress + 0x1600;   // mostly to define the direction of the data.
        VIA1_interruptFlag    = baseAddress + 0x1A00;   // interrupt status and acknowledgment
        VIA1_interruptEnable  = baseAddress + 0x1C00;   // interrupt enabling
        VIA2_dataB            = baseAddress + 0x0000;   // misc data ack bits

        PMreq	= (0x01 << HooperReq);
        PMack	= (0x01 << HooperAck);

#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::hwInit setting VIA interface at 0x%08lx PMreq=0x%02x PMack=0x%02x\n", (UInt32)baseAddress, PMreq, PMack);
#endif // VERBOSE_LOGS_ON_VIA
        
        // Creates and initializes the lock:;
        mutex = IOLockAlloc();
        if (mutex != NULL) {
            IOLockInit( mutex );
            return true;
        }
    }

    // If we are here something went wrong:
    return false;
}

// --------------------------------------------------------------------------
//
// Method: hwRelease
//
// Purpose:
//    Releases all the resources allocated in hwInit
bool
OpenViaInterface::hwRelease(void)
{
#ifdef VERBOSE_LOGS_ON_VIA
    kprintf("cleaning VIA interface \n");
#endif // VERBOSE_LOGS_ON_VIA

    VIA1_shift            = NULL;	               // shift register address
    VIA1_auxillaryControl = NULL;                  // mostly to define the direction of the data.
    VIA1_interruptFlag    = NULL;                  // interrupt status and acknowledgment
    VIA1_interruptEnable  = NULL;                  // interrupt enabling
    VIA2_dataB            = NULL;                  // misc data ack bits

    if (mutex != NULL) {
        IOLockFree(mutex);
        mutex = NULL;
    }

    return true;
}

// --------------------------------------------------------------------------
//
// Method: hwIsReady
//
// Purpose:
//    returns true if the hardware is properly allocated, false otherwise
bool
OpenViaInterface::hwIsReady(void)
{
    // we check for one (any of them would be good) of the
    // register pointers to see if it is set properly:
    return ((VIA1_shift != NULL) && (mutex != NULL));
}

// --------------------------------------------------------------------------
//
// Method: trustTheKernel
//
// Purpose:
//     This lets us set if the kernel is trustable for timing. Once the kernel
//     started the power down of the cpu most of the thread functions are no more
//     available. So IOSleep is likely to do not work.
void
OpenViaInterface::trustTheKernel(bool trustIt)
{
    theKernelIsUp = trustIt;
}

// --------------------------------------------------------------------------
//
// Method: isTheKernelUp
//
// Purpose:
//     returns the state of the kernel
bool
OpenViaInterface::isTheKernelUp()
{
    return theKernelIsUp;
}

// --------------------------------------------------------------------------
//
// Method: takeVIALock
//
// Purpose:
//     caller to lock the simple lock. The main reanson of existance for this
//     method is that when we will move to the interrupt based model this lock
//     will have to become a IOSimpleLockLockDisableInterrupt(). (NOTE: it
//     would greatly affect the overall performace to make this such a lock
//     in this POLLED driver. So wait for the interruupt based driver).
void OpenViaInterface::takeVIALock()
{
    if ((theKernelIsUp) && (mutex != NULL))
        IOLockLock(mutex);
}

// --------------------------------------------------------------------------
//
// Method: releaseVIALock
//
// Purpose:
//     caller to unlock the simple lock. The main reanson of existance for this
//     method is that when we will move to the interrupt based model this lock
//     will have to become a IOSimpleLockUnlockEnableInterrupt(). (NOTE: it
//     would greatly affect the overall performace to make this such a lock
//     in this POLLED driver. So wait for the interruupt based driver).
void OpenViaInterface::releaseVIALock()
{
    if ((theKernelIsUp) && (mutex != NULL))
        IOLockUnlock(mutex);
}

// re-flashes the pmu firmware:
// --------------------------------------------------------------------------
//
// Method: processPMURequest
//
// Purpose:
//     Reflashes the pmu microcode. It does not require any command (the E1 is
//     already provided by this method). The microCodeBlock is the image to
//     be downloaded. It is imperative that the BORG code is already at the
//     begin of the block.

#define kPMUImageLength 32772   // (32K + 4 bytes of signature)
#define kDownloadCommand 0xE1

bool
OpenViaInterface::downloadMicroCode(UInt8 *microCodeBlock, UInt32 length)
{
    bool success = true;

    if (microCodeBlock == NULL) {
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::downloadMicroCode microcode empty\n");
#endif // VERBOSE_LOGS_ON_VIA
        return  false;
    }

    if (length != kPMUImageLength) {
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::downloadMicroCode microcode of unsupported length\n");
#endif // VERBOSE_LOGS_ON_VIA

        return  false;
    }

    if ((microCodeBlock[0] != 'B') ||
        (microCodeBlock[1] != 'O') ||
        (microCodeBlock[2] != 'R') ||
        (microCodeBlock[3] != 'G')) {
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::downloadMicroCode microcode lacks signature\n");
#endif // VERBOSE_LOGS_ON_VIA

        return  false;
    }

    // O.K. If we are here we can suppose that the block of data is acceptable:

    // Diable the interrupts. So we do not get confused
    // (must be called before to take a lock since they are
    // lock-protected);
    disableSRInterrupt();

    // Starts the critical section area:
    takeVIALock();

    // and makes clear that we do not want to be interrupted
    // (but let the primary interrupt handlers run):
    disable_preemption();

    // Wait (up to 15 seconds) for the PMU to be ready to accept a command:
    if (waitForAck(true, 15000)) {
        UInt32 iterator;

        // first sends the download command:
        success &= sendByte(kDownloadCommand);

        // And after trasfers the image:
        for (iterator = 0 ; iterator < length; iterator++)
            success &= sendByte(microCodeBlock[iterator]);
    }
    else {
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::downloadMicroCode PMU not ready for download\n");
#endif // VERBOSE_LOGS_ON_VIA
        success = false;
    }

    // however things went we can others taks be rescheduled:
    enable_preemption();

    // End of the critical section area:
    releaseVIALock();

    // Re-enable the interrupts.
    enableSRInterrupt();

    return success;
}

// --------------------------------------------------------------------------
//
// Method: processPMURequest
//
// Purpose:
//     Sends a record to the PMU using the polling code instead than the interrupt code
//     this is (more or less) a direct port of the MacOS 9 sources.
//     The method returns true if the transmission was succesful, false if an error
//     occured.
bool
OpenViaInterface::processPMURequest(PMUrequestPtr plugInMessage)
{
    int howManyBytes, currentByte;
    char myByte;
    bool success = true;

    // If there is not a message to transmit we do not send anything.
    // However I believe that this is NOT a transmission error and
    // so we sould return true.
    if (plugInMessage == NULL)
        return success;

    // Diable the interrupts. So we do not get confused
    // (must be called before to take a lock since they are
    // lock-protected);
    disableSRInterrupt();

    // Starts the critical section area:
    takeVIALock();

#ifdef TRACE
    /* The debug code consists of the following
        *
        * ----------------------------------------------------------------------
        *|              |               |                               |Func   |
        *| Class (8)    | SubClass (8)  |          Code (14)            |Qual(2)|
        * ----------------------------------------------------------------------
        * Class     = drivers = 0x06
        * Sub Class = pmu     = 0x0A
        * Code      = func ID = 1
        * FuncQulif: it is
        * DBG_FUNC_START          1
        * DBG_FUNC_END            2
        * DBG_FUNC_NONE           0
        * 0x060A0005
        * how to trace:
        * trace -i enables the tracing and sets up the kernel buffer for tracing
        * trace -g shows the trace setup.
        * trace -e start tracing (but I use trace -e -c 6 -s 10 to trace only the calls in the
                                  *          serial driver.
                                  * trace -d stop the tracer.
                                  * trace -t codefile >result dumps the content of the trace buffer in the file "result"
                                  */

    KERNEL_DEBUG_CONSTANT((DRVDBG_CODE(0x0A, 1)) | DBG_FUNC_START, plugInMessage->pmCommand, plugInMessage->pmSLength, plugInMessage->pmSBuffer[0], plugInMessage->pmSBuffer[1], plugInMessage->pmSBuffer[2]); //0x060A0005
#endif
    
    // and makes clear that we do not want to be interrupted
    // (but let the primary interrupt handlers run):
    disable_preemption();

    // This is in case we jump at the end becuase of an error:
    plugInMessage->pmRLength = 0;

#ifdef VERBOSE_LOGS_ON_VIA
    kprintf("OpenViaInterface::processPMURequest starts for 0x%02x\n", plugInMessage->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA

    // Attempts to send the first byte:
    int attemptsForFirstByte = 512;
    do {
        // Are we in the right state to send a byte ?
       if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA
            kprintf("OpenViaInterface::processPMURequest [1] waitForAck(true, 32) failed\n");
#endif // VERBOSE_LOGS_ON_VIA

            success = false;
            goto exitFromPolledTransmitter;
        }

        // o.k. So let's try to send it:
        if (sendByte(plugInMessage->pmCommand))
            break;

        // Let the hardware relax for awhile (32 milliseconds)
        IODelay(32000);

    } while (attemptsForFirstByte--);

    if (attemptsForFirstByte == 0)  {
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::processPMURequest all the attempts of sending the first byte failed\n");
#endif // VERBOSE_LOGS_ON_VIA

        success = false;
        goto exitFromPolledTransmitter;
    }
#ifdef VERBOSE_LOGS_ON_VIA
    else
        kprintf("OpenViaInterface::processPMURequest sent command code 0x%02x\n", plugInMessage->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA


    // The first byte was sent, the following byte is the lenght of the message if the
    // message has a varaible length (code -1 in the pmu tables) otherwise we skip
    // to send the buffer.

    // From now on we are in a critical path a fail menas that the machine will power off
    // Should we send the lenght ?
    if (cmdLengthTable[plugInMessage->pmCommand] < 0 ) {
        // So send the message lenght:
        if (!sendByte(plugInMessage->pmSLength)) {
#ifdef VERBOSE_LOGS_ON_VIA
            kprintf("OpenViaInterface::processPMURequest all the attempts of sending the message lenght failed\n");
#endif // VERBOSE_LOGS_ON_VIA

            success = false;
            goto exitFromPolledTransmitter;
        }
#ifdef VERBOSE_LOGS_ON_VIA
    else
        kprintf("OpenViaInterface::processPMURequest message lenght %d\n", plugInMessage->pmSLength);
#endif // VERBOSE_LOGS_ON_VIA
        
        howManyBytes = plugInMessage->pmSLength;
    }
    else // the data legnth is predefined
        howManyBytes = cmdLengthTable[plugInMessage->pmCommand];

    // Send all the bytes in the buffer:
#ifdef VERBOSE_LOGS_ON_VIA
    kprintf("OpenViaInterface::processPMURequest sending %d bytes\n", howManyBytes);
#endif // VERBOSE_LOGS_ON_VIA
    currentByte = 0;
    while (currentByte < howManyBytes) {
        if (!sendByte(plugInMessage->pmSBuffer[currentByte])) {
#ifdef VERBOSE_LOGS_ON_VIA
            kprintf("OpenViaInterface::processPMURequest all the attempts of sending the byte message %d failed\n", currentByte);
#endif // VERBOSE_LOGS_ON_VIA

            success = false;
            goto exitFromPolledTransmitter;
        }
        else {
#ifdef VERBOSE_LOGS_ON_VIA
            kprintf("OpenViaInterface::processPMURequest  sent byte %d 0x%02x\n", currentByte, plugInMessage->pmSBuffer[currentByte]);
#endif // VERBOSE_LOGS_ON_VIA
        }

        currentByte++;
    }

    // All the bytes to be sent are gone. Now 2 things may happen:
    // 1] we do not have anything to read.
    // 2] we have something to read and ...
    //    a] we know the lenght
    //    b] we do not know the length (code -1)
    // 3] (didn't I write 2?) there is nothing to read and we
    //    can exit.
    howManyBytes = rspLengthTable[plugInMessage->pmCommand];

    // if there is something to read:
    if (howManyBytes != 0) {
        // How many bytes should we read ?
        if (howManyBytes < 0 ) {
            // So read the answer lenght:
            if (!readByte(&myByte)) {
#ifdef VERBOSE_LOGS_ON_VIA
                kprintf("OpenViaInterface::processPMURequest all the attempts of reading the answer lenght failed\n");
#endif // VERBOSE_LOGS_ON_VIA

                success = false;
                goto exitFromPolledTransmitter;
            }
            else
                howManyBytes = (unsigned char)myByte;
        }
        else if (rspLengthTable[plugInMessage->pmCommand] > 1) // the data legnth is predefined and is 1
            howManyBytes--;

        // define the return lenght
        plugInMessage->pmRLength = howManyBytes;

        // Reads all the bytes:
#ifdef VERBOSE_LOGS_ON_VIA
        kprintf("OpenViaInterface::processPMURequest reading %d bytes\n", howManyBytes);
#endif // VERBOSE_LOGS_ON_VIA

        currentByte = 0;
        while (currentByte < howManyBytes) {
            if (!readByte(&myByte)) {
#ifdef VERBOSE_LOGS_ON_VIA
                kprintf("OpenViaInterface::processPMURequest all the attempts of read the byte answer %d failed\n", currentByte);
#endif // VERBOSE_LOGS_ON_VIA

                success = false;
                goto exitFromPolledTransmitter;
            }

            // If the caller provided a buffer where to store the data, there we place it:
            if (plugInMessage->pmRBuffer != NULL) {
#ifdef VERBOSE_LOGS_ON_VIA
                kprintf("OpenViaInterface::processPMURequest  read byte answer %d 0x%02x\n", currentByte, myByte);
#endif // VERBOSE_LOGS_ON_VIA
                plugInMessage->pmRBuffer[currentByte] = myByte;
            }

            currentByte++;
        }
    }

    // Common exit point for the method
exitFromPolledTransmitter:

    // however things went we can others taks be rescheduled:
    enable_preemption();

    // End of the critical section area:
    releaseVIALock();

    // Re-enable the interrupts.
    enableSRInterrupt();

#ifdef TRACE
    KERNEL_DEBUG_CONSTANT((DRVDBG_CODE(0x0A, 1)) | DBG_FUNC_END, plugInMessage->pmCommand, plugInMessage->pmRLength, plugInMessage->pmRBuffer[0], plugInMessage->pmRBuffer[1], plugInMessage->pmRBuffer[2]);
#endif

#ifdef VERBOSE_LOGS_ON_VIA
    kprintf("OpenViaInterface::processPMURequest ends for 0x%02x\n", plugInMessage->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA
    return true;
}

// --------------------------------------------------------------------------
//
// Method: sendByte
//
// Purpose:
//    Sends a byte to the PMU. Returns true if the byte was succesully sent, false if
//    an error occured.
bool
OpenViaInterface::sendByte(char byte)
{
    bool success = false;

    *VIA1_auxillaryControl |= 0x1C;		        // set shift register to output
    eieio();

    *VIA1_shift = byte;		                    // give it the byte (this clears any pending SR interrupt)
    eieio();

    // *VIA1_interruptEnable = 0x84;	        // enable SR interrupt
    *VIA2_dataB &= ~PMreq;			            // assert /REQ
    eieio();

    if (waitForAck(false, 32)) {                // Wait for ack low (false)
        *VIA2_dataB |= PMreq;			        // deassert /REQ
        eieio();

        if (waitForAck(true, 32))               // Wait for ack hi (true)
            success = true;
#ifdef VERBOSE_LOGS_ON_VIA
        else
            kprintf("OpenViaInterface::sendByte [2] waitForAck(true, 32) failed\n");
#endif // VERBOSE_LOGS_ON_VIA
    }
#ifdef VERBOSE_LOGS_ON_VIA
    else
        kprintf("OpenViaInterface::sendByte [3] waitForAck(false, 32) failed\n");
#endif // VERBOSE_LOGS_ON_VIA

    *VIA2_dataB |= PMreq;			   // deassert /REQ
    eieio();

    *VIA1_auxillaryControl |= 0x1C;		// set shift register to output
    eieio();

    return (success);
}

// --------------------------------------------------------------------------
//
// Method: readByte
//
// Purpose:
//    Read a byte to the PMU. Returns true if the byte was succesully read, false if
//    an error occured, in this case the byte is not trustable.
bool OpenViaInterface::readByte(char *byte)
{
    bool success = false;

    *VIA1_auxillaryControl |= 0x0C;             // set shift register to input
    eieio();

    *VIA1_auxillaryControl &= ~0x10;            // set shift register to use external clock
    eieio();

    *byte = *VIA1_shift;                        // get the byte (this clears any pending SR interrupt)
    eieio();
    //    kprintf("OpenViaInterface::readByte read  [1] 0x%02x\n", *byte);

    *VIA2_dataB &= ~PMreq;		                // assert /REQ
    eieio();

    if (waitForAck(false, 32)) {                // Wait for ack low (false)
        *VIA2_dataB |= PMreq;			        // deassert /REQ
        eieio();

        if (waitForAck(true, 32)) {              // Wait for ack hi (true)
            *byte = *VIA1_shift;                 // get the byte (this time for real)
            eieio();
            //            kprintf("OpenViaInterface::readByte read  [2] 0x%02x\n", *byte);
            success = true;
        }
#ifdef VERBOSE_LOGS_ON_VIA
        else
            kprintf("OpenViaInterface::readByte [4] waitForAck(true, 32) failed\n");
#endif // VERBOSE_LOGS_ON_VIA
    }
#ifdef VERBOSE_LOGS_ON_VIA
    else
        kprintf("OpenViaInterface::readByte [5] waitForAck(false, 32) failed\n");
#endif // VERBOSE_LOGS_ON_VIA

    *VIA2_dataB |= PMreq;			    // deassert /REQ
    eieio();

    *VIA1_auxillaryControl |= 0x1C;		// set shift register to output
    eieio();

    return (success);
}

// --------------------------------------------------------------------------
//
// Method: waitForAck
//
// Purpose:
//       Waits for tha ack bit to match the given state:
//       wait for ack hi - > waitForAck(true, time)
//       wait for ack lo - > waitForAck(false, time)
bool OpenViaInterface::waitForAck(bool mode, UInt32 milliseconds)
{
    AbsoluteTime        currentTime, endTime;
    
    clock_interval_to_deadline(milliseconds, 1000000, &endTime);

    do {
        UInt8 viaNow = *VIA2_dataB;
        eieio();

        if (((viaNow & PMack) != 0) && (mode)) {           // we are waiting for ack hi and ..
            return ( true );				   // ack is high, return
        }

        if (((viaNow & PMack) == 0) && (!mode)) {          // we are waiting for ack low and ..
            return ( true );				   // ack is low, return
        }

        clock_get_uptime(&currentTime);
    } while ( CMP_ABSOLUTETIME(&endTime, &currentTime) > 0 );

    return (false);
}

// --------------------------------------------------------------------------
//
// Method: disableSRInterrupt
//
// Purpose:
//
void
OpenViaInterface::disableSRInterrupt ( void )
{
    *VIA1_interruptEnable = 1<<ifSR;
    eieio();
}

// --------------------------------------------------------------------------
//
// Method: enableSRInterrupt
//
// Purpose:
//
void
OpenViaInterface::enableSRInterrupt ( void )
{
    *VIA1_interruptEnable = (1<<ifSR) | 0x80;
    eieio();
}

// --------------------------------------------------------------------------
//
// Method: disablePMUInterrupt
//
// Purpose:
//
void
OpenViaInterface::disablePMUInterrupt ( void )
{
    takeVIALock();
    *VIA1_interruptEnable = 1<<ifCB1;
    eieio();
    releaseVIALock();
}

// --------------------------------------------------------------------------
//
// Method: enablePMUInterrupt
//
// Purpose:
//
void
OpenViaInterface::enablePMUInterrupt ( void )
{
    takeVIALock();
    *VIA1_interruptEnable = (1<<ifCB1) | 0x80;
    eieio();
    releaseVIALock();
}

// --------------------------------------------------------------------------
//
// Method: acknowledgePMUInterrupt
//
// Purpose:
//
void
OpenViaInterface::acknowledgePMUInterrupt ( void )
{
    takeVIALock();
    *VIA1_interruptFlag = 1<<ifCB1;
    eieio();
    releaseVIALock();
}

// --------------------------------------------------------------------------
//
// Method: pmuInteruptPending
//
// Purpose:
//	returns true if there is a pmu interrupt pending.
bool
OpenViaInterface::pmuInteruptPending(void)
{
    bool interruptisPending;
    
    takeVIALock();
    interruptisPending = (*VIA1_interruptFlag & 0x10);
    eieio();
    releaseVIALock();

    return true;
}

// --------------------------------------------------------------------------
//
// Method: srInteruptPending
//
// Purpose:
//	returns true if there is a shift register interrupt pending.
bool
OpenViaInterface::srInteruptPending(void)
{
    return (*VIA1_interruptFlag & 0x04);
}



// =====================================================================================
// VIA Interfaces: Interrupt
// =====================================================================================

#undef super
#define super OpenViaInterface
OSDefineMetaClassAndStructors(OpenIntrrViaInterface,OpenViaInterface)

// --------------------------------------------------------------------------
//
// Method: disableSRInterrupt
//
// Purpose:
//
void
OpenIntrrViaInterface::disableSRInterrupt ( void )
{
    super::disableSRInterrupt();

    if (interruptSource != NULL)
        interruptSource->disableInterrupt(VIA_DEV_VIA0);

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::disableSRInterrupt\n");
#endif // VERBOSE_LOGS_ON_VIA_INTR
}

// --------------------------------------------------------------------------
//
// Method: enableSRInterrupt
//
// Purpose:
//
void
OpenIntrrViaInterface::enableSRInterrupt ( void )
{
    if (interruptSource != NULL)
        interruptSource->enableInterrupt(VIA_DEV_VIA0);

    super::enableSRInterrupt();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::enableSRInterrupt\n");
#endif // VERBOSE_LOGS_ON_VIA_INTR

}

// --------------------------------------------------------------------------
//
// Method: shiftRegisterInt
//
// Purpose:
//	This is the interrupt handler for the shift register:
//	The shift register has finished shifting in a byte from PG&E or finished
//	shifting out a byte to PG&E.  Here we continue the transaction by starting
//	the i/o of the next byte, or we finish the transaction by indicating an
//	interrupt on either the command-done interrupt source or the autopoll
//	interrupt source on the workloop.
//	Both the VIA interrupt flag register and the interrupt enable registers
//	have been cleared by the ohare ISR.

/* static */ void
OpenIntrrViaInterface::shiftRegisterInt (OSObject *castMeToOpenIntrrViaInterface, IOInterruptEventSource *, int)
{
    OpenIntrrViaInterface *myThis = OSDynamicCast(OpenIntrrViaInterface, castMeToOpenIntrrViaInterface);

    if (myThis != NULL) {
        myThis->disableSRInterrupt();
        myThis->actUponState();
        myThis->enableSRInterrupt();
    }
}

// --------------------------------------------------------------------------
//
// Method: prepareSync
//
// Purpose:
//         this prepare the syncer for the wait:
void
OpenIntrrViaInterface::prepareSync()
{
    kern_return_t kr;
    
    kr = semaphore_create(current_task(), (semaphore **) &mySync, SYNC_POLICY_FIFO, 0);

    assert(kr == KERN_SUCCESS);
}

// --------------------------------------------------------------------------
//
// Method: waitForSync
//
// Purpose:
//         waits on the syncer. This function will return only once
//         sigTheSync is called.
void
OpenIntrrViaInterface::waitForSync()
{
    // Wait here from now on
    semaphore_wait(mySync);

    // Since we are here we had been already uunblocked and we
    // do not need the semaphore anymore:
    semaphore_destroy(current_task(), mySync);
}

// --------------------------------------------------------------------------
//
// Method: sigTheSync
//
// Purpose:
//         signal the sync restarting the thread that was stopped in the
//         waitForSync.
void
OpenIntrrViaInterface::sigTheSync()
{
    semaphore_signal(mySync);
}

// --------------------------------------------------------------------------
//
// Method: sigTheSync
//
// Purpose:
//         This method starts the transfer. this means that it sets the correct
//         initial status and calls processState for the first time so that the
//         sequence of interrupts kicks in:
bool
OpenIntrrViaInterface::sendToPMU(PMUrequestPtr theRequest)
{
    assert(theRequest != NULL);

    // how many data bytes we sent (or read, it depends from the state
    // we are in). These are the default values, corretion follows few
    // lines below:
    transferState.numberOfTransferedBytes = 0;
    transferState.numberOfBytesToBeTransfered = 0;

    // the request we have to process:
    transferState.currentTransfer = theRequest;

    // this keeps track if the trasfer was succesful:
    transferState.success = true;

    // first of all we need to enable the interrupts on the hardware ...
    enableSRInterrupt();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::sendToPMU(0x%02x) START\n", transferState.currentTransfer->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA_INTR

    // defines the state following the transmission of the
    // command.
    if (cmdLengthTable[transferState.currentTransfer->pmCommand] < 0) {
        // So the lenght of this command is not already defined, so
        // we got to transfer it:
        transferState.numberOfBytesToBeTransfered = transferState.currentTransfer->pmSLength;
        transferState.currentInterruptState = kSendLenght;
    }
    else if (cmdLengthTable[transferState.currentTransfer->pmCommand] > 0) {
        // The length is pre-defined, so if it is greater than 0 sets the number of bytes
        // we got to send:

        // Number of bytes to send:
        transferState.numberOfBytesToBeTransfered = cmdLengthTable[transferState.currentTransfer->pmCommand];

        // and since it is not neede to send the bytes, the next state is sending the data:
        transferState.currentInterruptState = kSendData;
    }
    else /* if (cmdLengthTable[transferState.currentTransfer->pmCommand] == 0) */ {
        // We do not have to send anything as data. So the next state has to end
        // the write (of the command) and decide if there is need to read or not.
        transferState.currentInterruptState =  kSwitchToRead;
    }

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::sendToPMU(0x%02x) next state %d\n", transferState.currentTransfer->pmCommand, transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

    // Waits until the PMU is actually ready
    if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
        kprintf("OpenIntrrViaInterface::sendToPMU(0x%02x) waitForAck(true, 32) fails\n", transferState.currentTransfer->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA_INTR
        return false;
    }

    // Prepare the syncer so we can wait for the train of interrupt to finish:
    prepareSync();

    // Send the command byte and start to make the ball rolling.
    sendIntrByte(transferState.currentTransfer->pmCommand);

    // now all we got to do is wait until the process finishes:
    waitForSync();

    // Hardware and provider interrupts are not needed anymore:
    disableSRInterrupt();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    if ((!waitForAck(true, 32)) || (!transferState.success)) {
        kprintf("OpenIntrrViaInterface::sendToPMU(0x%02x) waitForAck(true, 32) fails: WARNING PMU IN BAD STATE\n", transferState.currentTransfer->pmCommand);
        return false;
    }

    kprintf("OpenIntrrViaInterface::sendToPMU(0x%02x) END \n", transferState.currentTransfer->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA_INTR

    // and return how it went:
    return (transferState.success);
}

// --------------------------------------------------------------------------
//
// Method: actUponState
//
// Purpose:
// This method knowing the current InterruptState (it is the
// argument), and the next interrupt state (which MUST be alresdy
// in transferState) performs the correct set of actions.
// The method is build in three sectors:
//     1] Complete the previous command (whatever it was a read or write).
//     2] Act on the current command (so do the needed read or write).
//     3] Ony in case we need to read more data request the pmu to send
//        more data.
//
void
OpenIntrrViaInterface::actUponState()
{
#ifdef VERBOSE_LOGS_ON_VIA_INTR
    AbsoluteTime currentTime;
    abstime_scalar_t divider = 1000000;
    UInt32 myTime;

    // We wish to time-stamp each state, so that we know who caused the power off 250 milliseconds ago.
    clock_get_uptime(&currentTime);
    myTime = AbsoluteTime_to_scalar(&currentTime) / divider;

    kprintf("OpenIntrrViaInterface::actUponState(%d) at time %d\n", transferState.currentInterruptState, myTime);
#endif // VERBOSE_LOGS_ON_VIA_INTR

    // First of all: is this a real interrupt or a glich:
    if (transferState.currentInterruptState == kInterfaceIdle)
        return;

    // If the provious operation was a write here we are waiting
    // for the pmu to acknowledge that it read the byte we sent.
    // If the provious operation was a request to read here we are
    // waiting for the pmu to acknowledge that it sent the byte to
    // the shift register.
    if (!waitForAck(false, 320)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
        // We wish to time-stamp each state, so that we know who caused the power off 250 milliseconds ago.
        clock_get_uptime(&currentTime);
        myTime = AbsoluteTime_to_scalar(&currentTime) / divider;

        kprintf("OpenIntrrViaInterface::actUponState(%d) intial waitForAck(false, 320) failed at time %d\n", transferState.currentInterruptState, myTime);
#endif // VERBOSE_LOGS_ON_VIA_INTR

        transferState.success = false;
        return;
    }

    // what state are we in?
    switch (transferState.currentInterruptState)
    {
        case kSendCommand:
            panic("We should not be here\n");
            break;

        case kSendLenght:
            // We are doing a command transaction.  The command byte
            // transmission  has completed.
            // Start length byte transmission:

            // Completes the previous send (the command byte)
            // deassert /REQ
            *VIA2_dataB |= PMreq;
            eieio();

            // Intialize the counter of sent bytes:
            transferState.numberOfTransferedBytes = 0;

            // defines the next state:
            transferState.currentInterruptState = kSendData;

            // Waits to be sure that the pmu is ready to receive a byte (and sends it).
            if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                kprintf("OpenIntrrViaInterface::actUponState(%d) kSendLenght waitForAck(true, 32) failed\n", transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                transferState.success = false;
                return;
            }
                
            transferState.numberOfBytesToBeTransfered = transferState.currentTransfer->pmSLength;
            
            sendIntrByte(transferState.currentTransfer->pmSLength);
            break;

        case kSendData:
            // We just seny a byte (the data length, or a previous data byte)
            // so whatever there is to do now (continue sending data or begin
            // reading data)...

            // ... complete the previous write:
            *VIA2_dataB |= PMreq;
            eieio();

            // Waits to be sure that the pmu is ready again.
            if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                kprintf("OpenIntrrViaInterface::actUponState(%d) kSendData waitForAck(true, 32) failed\n", transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                transferState.success = false;
                return;
            }

            // If we still have bytes to send:
            if (transferState.numberOfTransferedBytes < transferState.numberOfBytesToBeTransfered) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                kprintf("OpenIntrrViaInterface::actUponState() kSendData sending byte no %d\n", transferState.numberOfTransferedBytes);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                // Send the next data-byte to the pmu:
                sendIntrByte(transferState.currentTransfer->pmSBuffer[transferState.numberOfTransferedBytes]);
                transferState.numberOfTransferedBytes++;
            }

            // Now that the byte is sent we may be either in the situation of
            // sending more data, or to begin to request data.
            if (transferState.numberOfTransferedBytes >= transferState.numberOfBytesToBeTransfered) {
                // O.K. We sent all the data. Decide what to do go in read mode (or exit)
                transferState.currentInterruptState = kSwitchToRead;
            }
            break;

        case kSwitchToRead:
            // We sent everything there was to send so we complete the last write:
            // by de-asserting the req:
            *VIA2_dataB |= PMreq;
            eieio();

            // Waits to be sure that the pmu is ready again.
            if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                kprintf("OpenIntrrViaInterface::actUponState(%d) kSendData kSwitchToRead(true, 32) failed\n", transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                transferState.success = false;
                return;
            }

            // Let's define the next state:
            if ( rspLengthTable[transferState.currentTransfer->pmCommand] < 0 ) {
                // The number of bytes is variable, so the next step will be to read
                // the lenght:
                transferState.currentInterruptState = kReadLenght;
            }
            else if ( rspLengthTable[transferState.currentTransfer->pmCommand] > 0 ) {
                // Number of bytes is instead fixed let see if it is 0 (nothing anymore to do)
                // or greater than 0 (there is data to read).

                // So since we need to read data:
                transferState.currentInterruptState = kReadData;

                // so many bytes:
                transferState.currentTransfer->pmRLength = rspLengthTable[transferState.currentTransfer->pmCommand];

                // This is something is still not clear, why do we need the following correction ?:
                // FIXME: figure out why we need this and insert appropriate comment:
                if (transferState.currentTransfer->pmRLength > 1) {
                    transferState.currentTransfer->pmRLength--;
                }

                // updates the bytes to be transfered with the corrected length:
                transferState.numberOfBytesToBeTransfered = transferState.currentTransfer->pmRLength;

                // And none read so far:
                transferState.numberOfTransferedBytes = 0;
            }
            else /* if ( rspLengthTable[transferState.currentTransfer->pmCommand] == 0 ) */ {
                // there is no data to trasfer, so the next state is ack to idle.
                transferState.currentInterruptState = kInterfaceIdle;
            }

            // If at the end of the sequence we have a next state different than
            // idle it means that we have to set the conditions to begin to read:
            if (transferState.currentInterruptState != kInterfaceIdle) {
                // switch the shift register in read (input) mode:
                *VIA1_auxillaryControl &= 0xEF;
                eieio();

                // read shift reg to clear SR int
                UInt8 lostRead = *VIA1_shift;
                eieio();

                // assert /REQ to begin the read
                *VIA2_dataB &= ~PMreq;
                eieio();
            }
            break;

        case kReadLenght:
            // The length byte has arrived, read it and start data read
            transferState.currentTransfer->pmRLength = readIntrByte();

            // Let's get ready for the next operation, this means that if we
            // do not have anything to read we can go in idle mode:
            if (transferState.currentTransfer->pmRLength == 0)
                transferState.currentInterruptState = kInterfaceIdle;
            else {
                // so sets up to read the incoming data:
                transferState.numberOfTransferedBytes = 0;
                transferState.numberOfBytesToBeTransfered = transferState.currentTransfer->pmRLength;

                // The next state is the read-data state:
                transferState.currentInterruptState = kReadData;

                // Waits to be sure that the pmu is ready again.
                if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                    kprintf("OpenIntrrViaInterface::actUponState(%d) kReadLenght kSwitchToRead(true, 32) failed\n", transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                    transferState.success = false;
                    return;
                }

                // Assert /REQ to continue to the read (or begin to read the data)
                *VIA2_dataB &= ~PMreq;
                eieio();
            }
            break;

        case kReadData:
#ifdef VERBOSE_LOGS_ON_VIA_INTR
            kprintf("OpenIntrrViaInterface::actUponState() kReadData read byte no %d\n", transferState.numberOfTransferedBytes);
#endif // VERBOSE_LOGS_ON_VIA_INTR

            // Read the next data-byte to the pmu:
            transferState.currentTransfer->pmRBuffer[transferState.numberOfTransferedBytes] = readIntrByte();
            transferState.numberOfTransferedBytes++;

            if (transferState.numberOfTransferedBytes < transferState.numberOfBytesToBeTransfered) {
                // again wait for the pmu to be rady.
                if (!waitForAck(true, 32)) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
                    kprintf("OpenIntrrViaInterface::actUponState(%d) kReadData kSwitchToRead(true, 32) failed\n", transferState.currentInterruptState);
#endif // VERBOSE_LOGS_ON_VIA_INTR

                    transferState.success = false;
                    return;
                }

                // Assert /REQ to continue to the read the data.
                *VIA2_dataB &= ~PMreq;
                eieio();
            }
            else {
                // We read everything there was to read and we go back idle.
                transferState.currentInterruptState = kInterfaceIdle;
            }
            break;

        case kInterfaceIdle:
            // empty this is state is looked at, at the end of this method.
            break;
    }

    // If now we are idle we are not expected to do anything anymore, so the
    // best thing to do is to signal and let the source-task to continue
    if (transferState.currentInterruptState == kInterfaceIdle) {
#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::actUponState end of a sequence , signal the sync\n");
#endif // VERBOSE_LOGS_ON_VIA_INTR
        sigTheSync();
    }

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    // We wish to time-stamp each state, so that we know who caused the power off 250 milliseconds ago.
    clock_get_uptime(&currentTime);
    myTime = AbsoluteTime_to_scalar(&currentTime) / divider;

    kprintf("OpenIntrrViaInterface::actUponState the next will be: %d time %d\n", transferState.currentInterruptState, myTime);
#endif // VERBOSE_LOGS_ON_VIA_INTR
}

// --------------------------------------------------------------------------
//
// Method: sendIntrByte
//
// Purpose:
//         sends a byte ro the PMU
void
OpenIntrrViaInterface::sendIntrByte(char byte)
{
    // set shift register to output
    *VIA1_auxillaryControl |= 0x1C;
    eieio();

    // give it the byte (this clears any pending SR interrupt)
    *VIA1_shift = byte;
    eieio();

    // assert /REQ so that the pmu knows that ther is abyte available.
    *VIA2_dataB &= ~PMreq;
    eieio();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::sendIntrByte sends for 0x%02x\n", (UInt8)byte);
#endif // VERBOSE_LOGS_ON_VIA_INTR
}

// --------------------------------------------------------------------------
//
// Method: readIntrByte
//
// Purpose:
//         reads a byte from the PMU
char
OpenIntrrViaInterface::readIntrByte()
{
    // read the data byte
    UInt8 byte = *VIA1_shift;
    eieio();

    // deassert /REQ line to let the pmu
    // know that we read the byte
    *VIA2_dataB |= PMreq;
    eieio();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
    kprintf("OpenIntrrViaInterface::readIntrByte returns for 0x%02x\n", (UInt8)byte);
#endif // VERBOSE_LOGS_ON_VIA_INTR

    return ((char)byte);
}

// Generic IOService stuff:
// --------------------------------------------------------------------------
//
// Method: v
//
// Purpose:
//         that's the usual start
bool
OpenIntrrViaInterface::start(IOService *provider)
{
    // Call the super to setup the pmu:
    if (!super::start(provider))
        return false;

    // register our handler for the interrupts:
    interruptSource = provider->getProvider();
    if (interruptSource != NULL) {
        IOReturn ret = interruptSource->registerInterrupt(VIA_DEV_VIA0, this, (IOInterruptAction) shiftRegisterInt);
        if (ret != kIOReturnSuccess) {
            kprintf("OpenIntrrViaInterface::start registerInterrupt fails with error %s\n", stringFromReturn(ret));
            panic("interruptSource->registerInterrupt fails\n");
            return false;
        }
    }
    else
        return false;

    return true;
}

// --------------------------------------------------------------------------
//
// Method: stop
//
// Purpose:
//         that's the usual stop
void
OpenIntrrViaInterface::stop(IOService *provider)
{
    if (interruptSource != NULL) {
        interruptSource->unregisterInterrupt(VIA_DEV_VIA0);

        disableSRInterrupt();
    }
    super::stop(provider);
}

// --------------------------------------------------------------------------
//
// Method: stop
//
// Purpose:
//         inherits the processPMURequest from the polling driver to start to
//         rollover of the interrupt transfer.
bool
OpenIntrrViaInterface::processPMURequest(PMUrequestPtr plugInMessage)
{
    // proceed with an interrupt-based transfer only if the kernel services
    // are trustable and of course if we have an interrupt source.
    if ((interruptSource != NULL) && (isTheKernelUp())) {
        // No interrupts to the pmu while we transfer this message:
        disablePMUInterrupt();

        // Starts the critical section area:
        takeVIALock();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
        kprintf("OpenIntrrViaInterface::processPMURequest starts for 0x%02x\n", plugInMessage->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA_INTR

        bool success = sendToPMU(plugInMessage);

        // End of the critical section area:
        releaseVIALock();

        // Re-enable the interrupts.
        enablePMUInterrupt();

#ifdef VERBOSE_LOGS_ON_VIA_INTR
        kprintf("OpenIntrrViaInterface::processPMURequest ends for 0x%02x\n", plugInMessage->pmCommand);
#endif // VERBOSE_LOGS_ON_VIA_INTR

        // returns how it did go:
        return success;
    }

    // If we do not have an interface for the intrrupts call the parent
    // methd (which will perform the transfer polling).
    return super::processPMURequest(plugInMessage);
}

