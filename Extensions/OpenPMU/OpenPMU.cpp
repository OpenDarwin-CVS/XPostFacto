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

#include "OpenPMU.h"
#include "OpenPMUNVRAMController.h"
#include "OpenPMURTCController.h"
#include "OpenPMUPwrController.h"
#include <IOKit/pwr_mgt/RootDomain.h>

#include "OpenPMUUserClient.h"

// =====================================================================================
// PMU Interface:
// =====================================================================================

#undef super
#define super IOService
OSDefineMetaClassAndStructors(OpenPMUInterface,IOService)

// Method: start
//
// Purpose:
//   Finds out the hardware it is running on and creates the best interface to
//   drive this PMU. Initializes the interface and sets up its own workloop to
//   drive the pmu requests.
bool
OpenPMUInterface::start(IOService *nub)
{
    // As usual we call the parent start:
    if (!super::start(nub))
        return false;

    IOMemoryMap *viaMap;
    
    // by default we behave as we are running on new machines.
    isOldHardware = false;

    // Remember the state of the interrupts:
    interruptsAreOn = false;
    
    // look for the class of machine we are running on:
    // ------------------------------------------------
    IOService *provider = nub;

    // See if we find the top of the tree (with the machine type)
    // iterating all the way up:
    while (provider != NULL) {
        // We stop only to check if we are the io IOService
        // mac-io in the registry, in which case we get the
        // base of the io memory map since we are going to
        // need it to power the modem:
        OSData *s = OSDynamicCast(OSData, provider->getProperty("name"));
        if ((s != NULL) && s->isEqualTo("mac-io", 6)) {
            // look for the comaptibility to see if this is a machine
            // with heatrow. Old Old hardware does not have the "compatible"
            // property so the lack of it
            OSData *kl = OSDynamicCast(OSData, provider->getProperty("compatible"));
            isOldHardware = ((kl == NULL) || kl->isEqualTo("heathrow", 7) || kl->isEqualTo("paddington", 10));
#ifdef VERBOSE_LOGS_ON_PMU_INT
            IOLog("OpenPMUInterface::start found %s hardware\n", (isOldHardware ? "old" : "new"));
#endif // VERBOSE_LOGS_ON_PMU_INT

            break;
        }

        // remember who is the top provider and iterates again:
        provider = provider->getProvider();
    }

	setProperty ("isOldHardware", isOldHardware);

    // Gets the memory map with the base address for the VIA
    viaMap = nub->mapDeviceMemoryWithIndex(0);
    if(viaMap == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start no via memory found for %s\n",  nub->getName());
#endif // VERBOSE_LOGS_ON_PMU_INT
        return false;
    }

    // Creates the mutex lock to protect the clients list:
    clientMutexLock = NULL;
    clientMutexLock = IOLockAlloc();
    
    // creates the syncer lock:
    syncLock = NULL;
    syncLock = IOLockAlloc();
    
    // Creates the best interface to drive the current PMU:
    // ---------------------------------------------------
    theHWInterface = NULL;

    // FIXME: since right now the interrupt version
    // is an empty class that inherits all the funtionality
    // from the polled driver there is not too much
    // difference between the two drivers.
    theHWInterface = new OpenIntrrViaInterface; // interrupt driven interface

    // A faliture in creating the interface is a global faliture for the driver
    // after all if we can not talk with the hardware there is little point in
    // going on:
    if (theHWInterface != NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start created a new via interface\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        if (!theHWInterface->init(NULL))
            return false;

        // From now on the corret way to call the functions is to have attach/detach around them:
        attach(theHWInterface);
        if (theHWInterface->probe(this,0) == NULL) {
            detach(theHWInterface);
            return false;
        }

        // starts the via inteface (and attach it as a child):
        if (!(theHWInterface->start(this) && theHWInterface->hwInit((UInt8*)viaMap->getPhysicalAddress()))) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
            IOLog("OpenPMUInterface::start theHWInterface failed to start or to init the hardware\n");
#endif // VERBOSE_LOGS_ON_PMU_INT
            detach(theHWInterface);
            
            return false;
        }
    }
    else {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start can not create a new theHWInterface\n");
#endif // VERBOSE_LOGS_ON_PMU_INT
        return false;
    }

    // Creates the Workloop and attaches all the event handlers to it:
    // ---------------------------------------------------------------
    workLoop = IOWorkLoop::workLoop();
    if (workLoop == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start can not create a new IOWorkLoop\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        return false;
    }

    // Creates the command gate for the events that need to be in the queue
    commandGate = IOCommandGate::commandGate(this, pmuCommandGateCaller);

    // and adds it to the workloop:
    if ((commandGate == NULL) ||
        (workLoop->addEventSource(commandGate) != kIOReturnSuccess))
    {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start can not create a new IOCommandGate\n");
#endif // VERBOSE_LOGS_ON_PMU_INT
        return false;
    }

    // Creates the interruupt event souurce
    pmuInterrupt = IOInterruptEventSource::interruptEventSource(this, pmuInterruptCaller, nub, VIA_DEV_VIA2);
 
   // and adds it to the workloop:
    if ((pmuInterrupt == NULL) ||
         (workLoop->addEventSource(pmuInterrupt) != kIOReturnSuccess) ) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::start can not create a new IOInterruptEventSource\n");
#endif // VERBOSE_LOGS_ON_PMU_INT
        return false;
    }

    // And let the workloop process the interrupts:
    workLoop->enableAllInterrupts(); 

    // And since we are still not busy:
    lockingCommand = 0xFF;

    // by default this is off:
    wakeOnRing = false;

    // Reads and exports the PG&E version number
    UInt8 iBuffer = 0;
    IOByteCount iLen = sizeof(iBuffer);
    if (sendMiscCommand (kPMUreadPmgrVers, (IOByteCount)0, NULL, &iLen, &iBuffer) == kIOReturnSuccess) {
        // And publishes the version:
        char versionNuumber[3];
        UInt8 lo = (iBuffer & 0x0F);
        UInt8 hi = ((iBuffer & 0xF0) >> 4);

        versionNuumber[0] = ((hi > 9) ? ('A' + (hi - 10)) : ('0' + hi));
        versionNuumber[1] = ((lo > 9) ? ('A' + (lo - 10)) : ('0' + lo));
        versionNuumber[2] = 0;
        setProperty("PMUFirmware", versionNuumber);
    }

    return true;
}

// --------------------------------------------------------------------------
//
// Method: stop
//
// Purpose:
//   Release all driver resources.
void
OpenPMUInterface::stop(IOService *provider)
{
    // Not much to do after all:
    freeAllResources();

    // Call the parent's one too:
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
OpenPMUInterface::free(void)
{
    // Not much to do after all:
    freeAllResources();
}

// --------------------------------------------------------------------------
//
// Method: freeAllResources
//
// Purpose:
//   This really releases all the allocated resources. It returs true or false
//   depneding upon the expectations ( basically I expect to free all the resouces
//   if one was already uunavailable the function returns false).
bool
OpenPMUInterface::freeAllResources(void)
{
    bool success = true;

    // Release the events in the workloop:
    if (pmuInterrupt != NULL) {

        // Also tells to the worklopp to do not dispatch interrutps anymore:
        workLoop->disableAllInterrupts();
        
        pmuInterrupt->release();
        pmuInterrupt = NULL;
    }
    else
        success = false;

    if (commandGate != NULL) {
        commandGate->release();
        commandGate = NULL;
    }
    else
        success = false;

    // And the workloop itself:
    if (workLoop != NULL) {
        workLoop->release();
        workLoop = NULL;
    }
    else
        success = false;

    // Release the interface with the hardware:
    if (theHWInterface != NULL) {
        theHWInterface->stop(this);
        detach(theHWInterface);
        theHWInterface->release();
        theHWInterface = NULL;
    }

    // Releases the mutex lock used to protect the clients lists:
    if (clientMutexLock != NULL) {
        IOLockFree (clientMutexLock);
        clientMutexLock = NULL;
    }
    
    // Releases the syncer lock:
    if (syncLock != NULL) {
        IOLockFree (syncLock);
        syncLock = NULL;
    }

    // The last thing to release are the clients:
    return (clearPMUClientList());
}

// De-Syncers: they make up for the lack of thread_func_call()
// -----------------------------------------------------------

// --------------------------------------------------------------------------
//
// Method: CallFuncDesync
//
// Purpose:
//         sets up an asyncronous call to the given function with the given
//         parameters.
void
OpenPMUInterface::CallFuncDesync(FunctionType func, void* parameters, AbsoluteTime *delay)
{
    ThreadParametersPtr callSet= (ThreadParametersPtr)IOMalloc(sizeof(ThreadParameters));

    if (callSet != NULL) {
        // Sets up function and paramets for the destination:
        callSet->functionParameters = parameters;
        callSet->targetFunction = func;
        
        // Calls our interface function, with all the info to call the original function, but
        // also to remove the therad_call.
        callSet->workThread = thread_call_allocate((thread_call_func_t)CallAndFreeThread, (thread_call_param_t)callSet);

        if (delay != NULL)
            thread_call_enter_delayed(callSet->workThread, *delay);
        else
            thread_call_enter(callSet->workThread);
    }
}

// --------------------------------------------------------------------------
//
// Method: CallAndFreeThread
//
// Purpose:
//         this provides the interface with the original function, so that the
//         caller does not have to worry about cleaning up:
/* static */ void
OpenPMUInterface::CallAndFreeThread(ThreadParametersPtr params)
{
    ThreadParametersPtr myParameters = (ThreadParametersPtr)params;
    
    if (myParameters != NULL) {
        // Makes a local copy of the thread_call_t since one of the
        // things we are supposed to do is to free the paramter block:
        thread_call_t workThread = myParameters->workThread;
        
        // If there is a function to call, just call it:
        if (myParameters->targetFunction != NULL)
            myParameters->targetFunction(myParameters->functionParameters, 0);

        // Releases all the paramet block:
        IOFree(myParameters, sizeof(ThreadParameters));
        
        // and frees the thread_call:
        assert(workThread != NULL);
        thread_call_free(workThread);
    }
}

// Client handling methods:
// ------------------------

// --------------------------------------------------------------------------
//
// Method: addPMUClient
//
// Purpose:
//         adds a client to the list. The argumers are the interrupt mack that
//         activates the notification for this client and obviously the callback
//         and the object rapresenting the client.
bool
OpenPMUInterface::addPMUClient(UInt8 interruptMask, OpenPMUClient function, IOService * caller)
{
    bool result = false;
    
    if (clientMutexLock != NULL)
        IOLockLock(clientMutexLock);
    
    PMUClientPtr newClient = (PMUClientPtr)IOMalloc(sizeof(PMUClient));

    if (newClient != NULL) {
        // Just creates a new link and atteches to the top of the list (there
        // is no order in the list so we can attach it anywhere).
        newClient->nextClient = listHead;
        newClient->interruptMask = interruptMask;
        newClient->client = caller;
        newClient->callBackFunction = function;

        listHead = newClient;

        // The only possible problem was the lack of memory.
        result = true;
    }

    if (clientMutexLock != NULL)
         IOLockUnlock(clientMutexLock);

    return result;
}

// --------------------------------------------------------------------------
//
// Method: removePMUClient
//
// Purpose:
//         removes a client to the list:
bool
OpenPMUInterface::removePMUClient(IOService * caller, UInt8 interruptMask)
{
    if (clientMutexLock != NULL)
        IOLockLock(clientMutexLock);

    PMUClientPtr tmp, tmpPrev = NULL;

    for (tmp = listHead; tmp != NULL; tmp = tmp->nextClient) {
        if ((tmp->client == caller) && (tmp->interruptMask == interruptMask)) {

            // detaches the link from the list:
            if (tmp == listHead) {
                listHead = tmp->nextClient;
            }
            else if (tmpPrev !=NULL) {
                tmpPrev->nextClient = tmp->nextClient;
            }
            else {
                // If it is not the head of the list tmpPrev should be differnt
                // than 0, so perhaps an assertion would be more proper here.
                // for now let's just log an error and report a faliture:
#ifdef VERBOSE_LOGS_ON_PMU_INT
                IOLog("OpenPMUInterface:removePMUClient, unlinked client\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

                return false;
            }

            // And since we are here it means that the client is unlinked and found
            // so we can remove it and report a success.
            IOFree(tmp, sizeof(PMUClient));

            if (clientMutexLock != NULL)
                 IOLockUnlock(clientMutexLock);
            
            return true;
        }
        else {
            tmpPrev = tmp;
        }
    }

    if (clientMutexLock != NULL)
         IOLockUnlock(clientMutexLock);
    
    // If we are here it measn we did not find the client with the
    // given object.
    return false;
}

// --------------------------------------------------------------------------
//
// Method: clearPMUClientList
//
// Purpose:
//         removes all clients from the list:
bool
OpenPMUInterface::clearPMUClientList()
{
    // Do not be tempted to add a the lock here.
    // removePMUClient already has its mutex lock
    // so here is not only un-necessary but also
    // harmful.

    bool success = true;

    while (listHead != NULL) {
        // I could just remove all from here. But doing a call
        // to the removePMUClient allows me to change the structure
        // of the list (if one day I'll wish to do so) and to test
        // the removePMUClient function.
        success &= removePMUClient(listHead->client, listHead->interruptMask);
    }

    return success;
}

// --------------------------------------------------------------------------
//
// Method: callPMUClient
//
// Purpose:
//         calls a client in the list with the data from the current interrupt:
bool
OpenPMUInterface::callPMUClient(UInt8 interruptMask, UInt32 length, UInt8 * buffer)
{    bool success = false;

    if (clientMutexLock != NULL)
        IOLockLock(clientMutexLock);

    // Finds the client matching the given interrupt mask and
    // calls them with the given buffer. NOTE: the clients are
    // not allowed to modify the buffer. If they need to make
    // changes they are supposed to make a copy of it.
    PMUClientPtr tmp;

    for (tmp = listHead; tmp != NULL; tmp = tmp->nextClient) {
        if ((tmp->interruptMask & interruptMask) != 0) {
            
#ifdef VERBOSE_LOGS_ON_PMU_INT
            IOLog("OpenPMUInterface::callPMUClient(0x%02x, %d, 0x%08lx) on %s->0x%08lx(0x%08lx...)\n", interruptMask, length, buffer, tmp->client->getName(), tmp->callBackFunction, tmp->client);
#endif // VERBOSE_LOGS_ON_PMU_INT

            (*tmp->callBackFunction)(tmp->client,interruptMask ,length ,buffer);

            // since we found one client (at least one client) we can
            // consider the call successful:
            success = true;
        }
    }

    if (clientMutexLock != NULL)
         IOLockUnlock(clientMutexLock);

    return (success);
}

// Desyncers structures and methods:
// ---------------------------------


// Desyncer from clients to PMU: enques to the via interface:
// --------------------------------------------------------------------------
//
// Method: enqueueData
//
// Purpose:
//         Enques to the via interface this method is defined as static
//         so it can be called with a threaded func call. This is from
//         clients to PMU. (this is called from createAndEnqueueRequest)
/* static */ void
OpenPMUInterface::enqueueData(void *castMeToInputCallParametersPtr, void *n)
{
    if (castMeToInputCallParametersPtr != NULL) {
        InputCallParametersPtr ip = (InputCallParametersPtr)castMeToInputCallParametersPtr;
        OpenPMUInterface *pmuDriver = OSDynamicCast(OpenPMUInterface, (OSObject*)ip->myThis);
        
        if (pmuDriver != NULL) {
            // From now on the driver is officialy busy:
            pmuDriver->setPMUDriverBusy(true, ip->theRequest.pmCommand);
            
            // Send and process the command:
            pmuDriver->commandGate->runCommand(&ip->theRequest);

#ifdef ADB_COMMANDS_HOLD_ALL
            // Now we can open the gate to the next command:
            // except for the kPMUpMgrADB they are considered
            // completed only when we get their interrupt.
            // the only exception is 
            if ((ip->theRequest.pmCommand != kPMUpMgrADB) || (ip->theRequest.pmSBuffer[1] = 0x86))
#endif // ADB_COMMANDS_HOLD_ALL                
                pmuDriver->setPMUDriverBusy(false, ip->theRequest.pmCommand);
        }

        // If the caller is already gone (the case of asyncronous calls)
        // we can get rid of all this RAM:
        if (ip->remebemberToFreeMe)
            IOFree(ip, sizeof(InputCallParameters));
    }
}

// Desyncer from clients to PMU: enques to the via interface:
// --------------------------------------------------------------------------
//
// Method: clientNotifyData
//
// Purpose:
//         calls the client that registed for interrupts. This method is
//         defined as static so it can vbe called with a threaded func call.
//         This is from PMU to clients.
/* static */ void
OpenPMUInterface::clientNotifyData(void *castMeToOutputCallParametersPtr, void *n)
{    
    if (castMeToOutputCallParametersPtr != NULL) {
        OutputCallParametersPtr  op = (OutputCallParametersPtr)castMeToOutputCallParametersPtr;
        OpenPMUInterface *pmuDriver = OSDynamicCast(OpenPMUInterface, (OSObject*)op->myThis);

        if (pmuDriver != NULL) {
            if (!pmuDriver->callPMUClient(op->interruptType, op->dataLength, op->buffer)) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
                IOLog("OpenPMUInterface::clientNotifyData nobody registed for 0x%02x\n", op->interruptType);
#endif // VERBOSE_LOGS_ON_PMU_INT
            }
        }

        // At this point the parametes are no more necessary, so I can free the memory
        // they allocated:
        IOFree(op, sizeof(OutputCallParameters));
    }
}

// Interrupt handling:
// -------------------

// --------------------------------------------------------------------------
//
// Method: pmuInterruptCaller
//
// Purpose:
//         calls the interrupt handler.
/* static */ void
OpenPMUInterface::pmuInterruptCaller(OSObject *object, IOInterruptEventSource *src, int intr)
{
    OpenPMUInterface *pmu = OSDynamicCast(OpenPMUInterface, object);

#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::pmuInterruptCaller got an interrupt\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

    if (pmu != NULL)
        pmu->interruptHandler();
}

// --------------------------------------------------------------------------
//
// Method: interruptHandler
//
// Purpose:
//         handles  the pmu interrupt, and for old machines simultates the
//         enviroment interrupt.
void
OpenPMUInterface::interruptHandler()
{
#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::interruptHandler got an interrupt\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

    PMUrequest interruptBlock;

    // reads the type of interrupt:
    interruptBlock.pmCommand = kPMUreadINT;
    interruptBlock.pmSLength = 0;

    if (!pmuTansferRequestToVIA(&interruptBlock)) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuInterruptCaller  fails reading interrupt\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        // and returns since we failed:
        return;
    }

#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::pmuInterruptCaller interrupt type 0x%02x\n", interruptBlock.pmRBuffer[0]);
#endif // VERBOSE_LOGS_ON_PMU_INT

    pmuInterruptDispatcher(interruptBlock.pmRBuffer, interruptBlock.pmRLength);
}

// --------------------------------------------------------------------------
//
// Method: pmuInterruptDispatcher
//
// Purpose:
//	dispatches the interrupt to the correct handler:
void
OpenPMUInterface::pmuInterruptDispatcher(UInt8 *buffer, UInt32 bufLen)
{
#ifdef ADB_COMMANDS_HOLD_ALL
    // if it is the conclusion of a kPMUpMgrADB we can re-open the gate:
    if ((lockingCommand == kPMUpMgrADB) &&
        (interruptBlock.pmRBuffer[0] & kPMUADBint)
        && (!(interruptBlock.pmRBuffer[0] & kPMUautopoll)))
        setPMUDriverBusy(false, kPMUpMgrADB);

    // The only reason we may wish to hold this is if we have an oustanfing ADB
    // command. Holding for one fraction of the polling period is not so bad
    // and it is an acceptabale delay.
    if (lockingCommand != kPMUpMgrADB)
#endif

        // This is a little tricky. The old powerbooks (PowerbookG3 1998 and 1999)
        // do not support interrupts for the environmet variables (battery and lid)
        // so we have to poll for these. Since on these machines we however need to
        // set the oneSec interrupt. It seems a waste to create a timer just for
        // this pourpose. So I'm going to use the oneSecInterrupt.
        // I also wish to be able to change the polling time so pollingDivider
        // allows me to increase the polling time with a resolution of seconds.
        // The current polling time is pollingInterval.
        // Note the kPMUoneSecInt is set only for old hardware so the second check
        // is a little eccessive, but it does no harm so.

        if ((buffer[0] & kPMUoneSecInt) && (isOldHardware) && ((--pollingDivider) == 0)) {
            pollingDivider = pollingInterval;
            checkPowerEnvironmentEvents();
        }

    // Now we can let the right client handle this interrupt:
    // Allocates a transfer block:
    OutputCallParametersPtr outputData= (OutputCallParametersPtr)IOMalloc(sizeof(OutputCallParameters));

    // Makes a copy of the data we care to transfer above:
    outputData->myThis = this;
    outputData->interruptType = buffer[0];
    outputData->dataLength = bufLen - 1;
    bcopy(buffer + 1, outputData->buffer, bufLen - 1);

    // and (asyncronously, so we free the workloop) we call the clients interested.
    // we do not need to dispose the memory allocated above,  clientNotifyData will
    // do it for us.
    CallFuncDesync(clientNotifyData, outputData);
}

// --------------------------------------------------------------------------
//
// Method: checkPowerEnvironmentEvents
//
// Purpose:
//	checks for the ststus of those devices that can affect the power state
//      the most common are the shell and battery. From here we know if the
//      user closed the clam shell on a powerbook or if we are very low on
//      battery.
void
OpenPMUInterface::checkPowerEnvironmentEvents(void)
{
    PMUrequest environmentBlock;

    // reads the status of the nevironemt varaible:
    environmentBlock.pmCommand = kPMUreadExtSwitches;
    environmentBlock.pmSLength = 0;

    if (pmuTansferRequestToVIA(&environmentBlock)) {
        // Enforce battery status check:
        environmentBlock.pmRBuffer[0] |= kBatteryStatusEventMask;

#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::checkPowerEnvironmentEvents envStatus 0x%02x\n", environmentBlock.pmRBuffer[0]);
#endif // VERBOSE_LOGS_ON_PMU_INT

        // Now we can let the right client handle this interrupt:
        // Allocates a transfer block:
        OutputCallParametersPtr outputData= (OutputCallParametersPtr)IOMalloc(sizeof(OutputCallParameters));

        // Makes a copy of the data we care to transfer above
        // NOTES: 1] since the whole buffer is meaningful here
        //           we need to make a copy of all of it:
        //        2] we always say this was an evironment interrupt
        //           even if we know this is not really the case
        //           for old machines.
        //        3] I am using bcopy only for consistency. I know
        //           that the result is always 1 byte, but hopefully
        //           this will keep the driver compatible with future
        //           versions of the pmu.
        outputData->myThis = this;
        outputData->interruptType = kPMUenvironmentInt;
        outputData->dataLength = environmentBlock.pmRLength;
        bcopy(environmentBlock.pmRBuffer, outputData->buffer, environmentBlock.pmRLength);

        // and (asyncronously, so we free the workloop) we call the clients interested.
        // we do not need to dispose the memory allocated above,  clientNotifyData will
        // do it for us.
        CallFuncDesync(clientNotifyData, outputData);
    }
}

// Command gate to enqueue data to the VIA:
// ----------------------------------------

// --------------------------------------------------------------------------
//
// Method: pmuCommandGateCaller
//
// Purpose:
//          this is where the run command ends:
/* static */ IOReturn
OpenPMUInterface::pmuCommandGateCaller(OSObject *object, void *arg0, void *arg1, void *arg2, void *arg3r)
{
    IOReturn retVal = kIOReturnError;
    OpenPMUInterface *pmu = OSDynamicCast(OpenPMUInterface, object);

    if (pmu != NULL)
        if (pmu->pmuTansferRequestToVIA((PMUrequest*)arg0))
            retVal = kIOReturnSuccess;

    return  retVal;
}

// --------------------------------------------------------------------------
//
// Method: pmuCommandGateReFlash
//
// Purpose:
//          this is a rare and exceptional call trough the command gate. Its
//          pourpose to is download a new firmaware in the PMU:
/* static */ IOReturn
OpenPMUInterface::pmuCommandGateReFlash(OSObject *object, void *castMeToFirmware, void *arg1, void *arg2, void *arg3r)
{
    OpenPMUInterface *pmu = OSDynamicCast(OpenPMUInterface, object);
    OSData *newFirmware = OSDynamicCast(OSData, (OSObject*)castMeToFirmware);

    if (pmu == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuCommandGateReFlash missing pmu object\n");
#endif // VERBOSE_LOGS_ON_PMU_INT
        
        return kIOReturnError;
    }

    if (newFirmware == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuCommandGateReFlash missing newfirmware to download\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        return kIOReturnError;
    }

    if (pmu->theHWInterface == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuCommandGateReFlash pmu lacks interface\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        return kIOReturnError;
    }

    // O.K. if we are here we have all the data to pergorm a transfer:
    if (!pmu->theHWInterface->downloadMicroCode((UInt8*)newFirmware->getBytesNoCopy(), newFirmware->getLength())) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuCommandGateReFlash downloadMicroCode download failed\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

        return kIOReturnError;
    }

    return  kIOReturnSuccess;
}

// --------------------------------------------------------------------------
//
// Method: pmuTansferRequestToVIA
//
// Purpose:
//         transfers a request to the VIA. The command assumes that the
//         request is alredy correcly formed.
bool
OpenPMUInterface::pmuTansferRequestToVIA(PMUrequest *pmuReq)
{
    bool returnValue = false;

    // Sends the request:
    if ((theHWInterface != NULL) && (pmuReq != NULL)) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuTansferRequestToVIA cpmmand 0x%02x\n", pmuReq->pmCommand);
#endif // VERBOSE_LOGS_ON_PMU_INT
        
        returnValue = theHWInterface->processPMURequest(pmuReq);

#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::pmuTansferRequestToVIA cpmmand 0x%02x %s\n", pmuReq->pmCommand, (returnValue ? "TRUE" : "FALSE"));
        IOSleep(1000);
#endif // VERBOSE_LOGS_ON_PMU_INT
    }

    return returnValue;
}

// --------------------------------------------------------------------------
//
// Method: createAndEnqueueRequest
//
// Purpose:
//        Creates a request starting from the given arguments and sends it to
//        the via interface (passing trough the commad gate).
bool
OpenPMUInterface::createAndEnqueueRequest(bool synchronousCall, UInt32 commandCode, IOByteCount sLength, UInt8* sBuffer, IOByteCount* rLength, UInt8* rBuffer)
{
    UInt8 success = true;

    // Is the command code valid ?
    if (commandCode > 255)
        return false;

    // copy the fields in the request:
    InputCallParametersPtr theNewRequest = (InputCallParametersPtr)IOMalloc(sizeof(InputCallParameters));
    if (theNewRequest != NULL) {
        theNewRequest->theRequest.pmCommand = commandCode;
        theNewRequest->theRequest.pmSLength = sLength;
        if (sBuffer != NULL)
            bcopy(sBuffer, theNewRequest->theRequest.pmSBuffer, sLength);

        // transfers this to the caller:
        theNewRequest->myThis = this;

        // If we care about running syncronously:

       if (synchronousCall) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
           IOLog("OpenPMUInterface::createAndEnqueueRequest calling syncronusly 0x%02x\n", commandCode);
#endif // VERBOSE_LOGS_ON_PMU_INT
                theNewRequest->remebemberToFreeMe = false;
                enqueueData(theNewRequest, NULL);
            }
        else {
#ifdef VERBOSE_LOGS_ON_PMU_INT
           IOLog("OpenPMUInterface::createAndEnqueueRequest calling asyncronusly 0x%02x\n", commandCode);
#endif // VERBOSE_LOGS_ON_PMU_INT

            theNewRequest->remebemberToFreeMe = true;
            CallFuncDesync(enqueueData, theNewRequest);

            // And since we do not care about the results and enqueueData
            // will free our structure (this is the reason for the
            // remebemberToFreeMe = true above) we can exit:
            return true;
        }

        // if successful bother setting up the answer:
        if (success) {
            // it is acceptable that the caller does not need an answer (even if the command
            // provides one) so we do not complain if the return buffer or the pointer for
            // the return data are null.
            if ((rLength != NULL) && (*rLength > 0)) {
                // we want to be sure that we are coping the right amount of data. In the pointer for the return
                // data there MUST be the dimension of the buffer set up by the caller.
                *rLength = (*rLength < theNewRequest->theRequest.pmRLength ? *rLength : theNewRequest->theRequest.pmRLength);

                // Also we may have a lenth as answer but not a buffer where to
                // transfer the data. This is also o.k. the caller my wish to know
                // if there was a reasult for its command, but without going in
                // the detail to know what the result was. (this is useful for
                // error handling. A command may answer with an error and the
                // caller want to know if there was an error (return > 0) or
                // not (return == 0) without caring of what the error was.
                if ((rBuffer != NULL) && (*rLength > 0)) {
                    bcopy(theNewRequest->theRequest.pmRBuffer, rBuffer, *rLength);
                }
            }
        }
    }

    // And since we do not need this guy anymore:
    IOFree(theNewRequest, sizeof(InputCallParameters));

    return success;
}

// --------------------------------------------------------------------------
//
// Method: setPMUDriverBusy
//
// Purpose:
//         This is to set the state of the pmu driver and block the pmu
//         clients when the driver (or the hardware) is busy and can not
//         process commands. This can not be implemented with the command
//         gate since I wish to keep the workloop running.
//         We also wish to remember the command that put us
//         in sleep mode.
void
OpenPMUInterface::setPMUDriverBusy(bool isBusy, int command)
{
#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::setPMUDriverBusy(%s, 0x%02x)\n", (isBusy? "true" : "false"), command);
#endif // VERBOSE_LOGS_ON_PMU_INT
    
    if (syncLock != NULL) {
        if (isBusy) {
            IOLockLock(syncLock);
            lockingCommand = command;
        }
        else {
#ifdef VERBOSE_LOGS_ON_PMU_INT
            if (lockingCommand != command)
                panic("Extra unlock on OpenPMUInterface::setPMUDriverBusy syncLock\n");
#endif // VERBOSE_LOGS_ON_PMU_INT

            lockingCommand = 0xFF;
            IOLockUnlock(syncLock);   
        }
    }
}

// --------------------------------------------------------------------------
//
// Method: sendMiscCommand
//
// Purpose:
//       This allows "uncommon" clients to send messages to the pmu.
//       To make the life simpler for the calling clients the generic rule is
//       that if the caller deos not require a result (rLength is NULL or
//       *rLength is 0) the call will be asyncronous. If you wish to enforce
//       a syncronous call, just set a return buffer and a return length
//       different than 0.
//       IMPORTANT: to avoid overflows of the return buffer *rLength MUST
//       contain the size of the rBuffer. at the return *rLength will contain
//       the number of transfered bytes.
IOReturn
OpenPMUInterface::sendMiscCommand (int command, IOByteCount sLength, UInt8 *sBuffer, IOByteCount *rLength, UInt8 *rBuffer)
{
    IOReturn retVal = kIOReturnError;

    // by default we assume that all the calls from a client are asyncronous.
    // but if the client obviously is expecting an answer .....
    bool isSync = false;

    // ... thay become syncronous
    if (rLength != NULL)
        isSync = ((*rLength) != 0);
  
    if (createAndEnqueueRequest(isSync, command, sLength, sBuffer, rLength, rBuffer))
        retVal = kIOReturnSuccess;
    
#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::sendMiscCommand for 0x%08lx returns %s\n", command, (retVal == kIOReturnSuccess ? "kIOReturnSuccess" : "kIOReturnError"));
#endif // VERBOSE_LOGS_ON_PMU_INT

    return  retVal;
}


// --------------------------------------------------------------------------
//
// Method: registerForPMUInterrupts
//
// Purpose:
//       If an other driver wishes to be aware of pmu transactions (or)
//       interrupts it has to register with the pmu driver:
//       Note that clients have rules to follow: the most important is
//       that they should NEVER change the conter of the buffer status
//       they receive from the pmu driver.
bool
OpenPMUInterface::registerForPMUInterrupts(UInt8 interruptMask, OpenPMUClient function, IOService * caller)
{
    // Just calls into the private method:
    return addPMUClient(interruptMask, function, caller); 
}

// --------------------------------------------------------------------------
//
// Method: deRegisterClient
//
// Purpose:
//        This is to de-register from the clients that  wish to be aware of pmu transactions
bool
OpenPMUInterface::deRegisterClient(IOService * caller, UInt8 interruptMask)
{
    // Just calls into the private method:
    return removePMUClient(caller, interruptMask);
}

// System Power commands:
// ----------------------

// --------------------------------------------------------------------------
//
// Method: setFileServerMode
//        
// Purpose:
//        turns on and off the file server mode.
//        the file server is normally on.
void
OpenPMUInterface::setFileServerMode(bool fileServerModeON)
{
    if (!isOldHardware) {
        UInt8 outBuffer[3];
        UInt8 inBuffer[2];
        IOByteCount iLen = sizeof(inBuffer);

	// First read the current mode:
        outBuffer[0] = kPMUgetPowerUpEvents;

        sendMiscCommand (kPMUpowerUpEvents, (IOByteCount)1,outBuffer, &iLen, inBuffer);

        //... and after defining the new set of events:
        outBuffer[0] = kPMUsetPowerUpEvents;
        outBuffer[1] = inBuffer[0];

	if (fileServerModeON)
        	outBuffer[2] = inBuffer[1] | kPMUwakeUpOnACInsert;
	else
		outBuffer[2] = inBuffer[1] & (~kPMUwakeUpOnACInsert);

        // we do not really want an answer but we wish to make it syncronous:
        iLen = sizeof(inBuffer);
        sendMiscCommand (kPMUpowerUpEvents, (IOByteCount)3, outBuffer, &iLen, inBuffer);

        // And sets a property to know how is the default for this setting:
        setProperty("FileServer", fileServerModeON);
    }
}

// --------------------------------------------------------------------------
//
// Method: setWakeOnRing
//
// Purpose:
//        turns on and off the wake on ring feature. This is just a wrapper
//        for a private variable.
void
OpenPMUInterface::setWakeOnRing(bool wakeOnRingON)
{
    wakeOnRing = wakeOnRingON;
    
    // And sets a property to know how is the default for this setting:
    setProperty("WakeOnRing", wakeOnRingON);

}

// --------------------------------------------------------------------------
//
// Method: putMachineToSleep
//
// Purpose:
//        sets the conditions to wake and tells to the pmu to put the machine
//        in sleep mode.
void
OpenPMUInterface::putMachineToSleep()
{
    // From now on the driver is officialy busy and
    // will not accept anymore commands until the
    // system wakes up. I can not place this command
    // after the sleep because it would create a race
    // condition between setting the busy and sending
    // a command after the sleep.
    setPMUDriverBusy(true, kPMUsleepReq);

    // since we can not use the misc command since the system is officially
    // (and pratically) busy now and we can not use the normal paths to
    // enquee messages.
    PMUrequest theRequest;

    theRequest.pmCommand = kPMUsleepReq;
    theRequest.pmSLength = 4;
    theRequest.pmSBuffer[0] = 'M';
    theRequest.pmSBuffer[1] = 'A';
    theRequest.pmSBuffer[2] = 'T';
    theRequest.pmSBuffer[3] = 'T';

    do {
        // Send and process the command:
        theRequest.pmRBuffer[0] = 0;
        commandGate->runCommand(&theRequest);
    } while (theRequest.pmRBuffer[0] != 0x70);
}

// --------------------------------------------------------------------------
//
// Method: preSleepSequence
//
// Purpose:
//        sets the conditions to wake up and powers off everything that it could
//        be powerd off at this level
void
OpenPMUInterface::preSleepSequence()
{
    // Powers off the LCD
    // We need this only for PwerbookG31999 that in this moment does not have
    // a brightness driver:
    if (IODTMatchNubWithKeys(getPlatform()->getProvider(), "'PowerBook1,1'"))
        setLCDPower(false);

    // Powers off the HD
    setHDPower(false);

    // Powers off the media bay
    setMediaBayPower(false);

    // Powers off the IR
    setIRPower(false);

    if (!isOldHardware) {
        UInt8 outBuffer[3];
        UInt8 inBuffer[2];
        IOByteCount iLen = sizeof(inBuffer);
        
        // Defines the power up events for this machine, by as first
        // clearing the current settings ...
        outBuffer[0] = kPMUclearWakeUpEvents;
        outBuffer[1] = 0xFF;
        outBuffer[2] = 0xFF;

        sendMiscCommand (kPMUpowerUpEvents, (IOByteCount)3, outBuffer, &iLen, inBuffer);

        //... and after defining the new set of events:
        outBuffer[0] = kPMUsetWakeUpEvents;
        outBuffer[1] = 0;

        if (wakeOnRing)
            outBuffer[2] = kPMUwakeUpOnRing | kPMUwakeUpOnKey | kPMUwakeUpOnLidOpen;
        else
            outBuffer[2] = kPMUwakeUpOnKey | kPMUwakeUpOnLidOpen;

        iLen = sizeof(inBuffer);
        sendMiscCommand (kPMUpowerUpEvents, (IOByteCount)3, outBuffer, &iLen, inBuffer);
    }
}

// --------------------------------------------------------------------------
//
// Method: wakeUP
//
// Purpose:
//        sets the machine in awake mode.
void
OpenPMUInterface::wakeUp()
{
    // From now on the driver is officialy non busy
    // and will accept all the next commands (plus
    // the ones that were enqueued while the driver
    // was busy).
    setPMUDriverBusy(false, kPMUsleepReq);

    // Informs the pmu that the system is up:
    saySystemReady();

    // Powers on the LCD
    // We need this only for PwerbookG31999 that in this moment does not have
    // a brightness driver:
    if (IODTMatchNubWithKeys(getPlatform()->getProvider(), "'PowerBook1,1'"))
        setLCDPower(true);

    // Powers on the HD
    setHDPower(true);

    // Powers on the media bay
    setMediaBayPower(true);

    // Powers on the IR
    setIRPower(true);
}

// --------------------------------------------------------------------------
//
// Method: rebootSystem
//
// Purpose:
//          reset the machine.
void
OpenPMUInterface::rebootSystem()
{
    // We are going to reboot, so we can not use the standard
    // send misc command because that pat requires the kernel
    // to work, and this is not assured (e.g. we are rebooting
    // after a panic). So this is waht we have to do:

    // Take a lock, so we know that there are not otstanding
    // commands, and we assure that no commands will be issued
    // after the reboot one (I know it because I am not releasing
    // the lock).
    setPMUDriverBusy(true, kPMUsleepReq);

    // We should not trust the kernel (at least regarding allocations and locks)
    // at this point:
    theHWInterface->trustTheKernel(false);

    // since we can not use the misc command since the system is officially
    // (and pratically) busy now and we can not use the normal paths to
    // enquee messages.
    PMUrequest theRequest;

    // We allocate the command on the stack. And send it .. this is all
    theRequest.pmCommand = kPMUresetCPU;
    theRequest.pmSLength = 0;
    commandGate->runCommand(&theRequest);
}


// --------------------------------------------------------------------------
//
// Method: shutdownSystem
//
// Purpose:
//          powers the machine off.
void
OpenPMUInterface::shutdownSystem()
{
    // first we make sure that file server mode is off (otherwise it will
    // power back on immediatly:
    setFileServerMode(false);

    // and after we shut down everything
    UInt8 *oBuffer = (UInt8*)"MATT";
    UInt8 iBuffer = 0;
    IOByteCount iLen = sizeof(iBuffer);

    do {
        sendMiscCommand (kPMUPmgrPWRoff, (IOByteCount)4, oBuffer, &iLen, &iBuffer);
    } while (iBuffer != 0x70);
}


// Macro commands:
// ---------------

// --------------------------------------------------------------------------
//
// Method: saysystemReady
//
// Purpose:
//        Defines the state of the system as "ready"
void
OpenPMUInterface::saySystemReady()
{
    if (isOldHardware) {
        // Do not do anything, we do not have anything to do on old pmus
    }
    else {
        UInt8 oBuffer = 2;

        sendMiscCommand (kPMUsystemReady, (IOByteCount)1, &oBuffer, NULL, NULL);
    }
}

// --------------------------------------------------------------------------
//
// Method: setInterrupts
//
// Purpose:
//         Enable/disables Interrupts. Not only however. On old hardwate where
//         the state of the battery and the clam shell must be polled this
//         also sets the oneSecInterrupt. Note that this interrupt would
//         be needed anyway because of a bug in MacOs9.
void
OpenPMUInterface::setInterrupts(bool on)
{
    UInt8 iBuffer[256];
    UInt8 interruptBits;
    IOByteCount iLen = sizeof(iBuffer);
    
    // if we do not have an activer hardware interface there is no point
    // in going on.

    if (theHWInterface == NULL) {
#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::setInterrupts %s without an theHWInterface\n", (on ? "on" : "off"));
#endif // VERBOSE_LOGS_ON_PMU_INT
        return;
    }
    
    // remember how we set the interrupts:
    interruptsAreOn = on;

    // Actually sets the interrupt in the way we like it:
    if (on) {
        // This is meaningful only on old hardware:
        pollingDivider = pollingInterval;
            
        // Also tell to the PMU which interrupts we need:
        // note that in one case we set kPMUoneSecInt, this works around a bug in 9 and
        // provides a method for polling the enviromentstatus since old hardware does not
        // have an interrupt for it.
        if (isOldHardware)
            interruptBits = kPMUpcmicia | kPMUbrightnessInt | kPMUADBint | kPMUoneSecInt;
        else
            interruptBits = kPMUpcmicia | kPMUbrightnessInt | kPMUADBint | kPMUenvironmentInt;

#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::setInterrupts on with flags: 0x%02x\n", interruptBits);
#endif // VERBOSE_LOGS_ON_PMU_INT

        sendMiscCommand (kPMUSetModem1SecInt, (IOByteCount)1, &interruptBits, &iLen, iBuffer);

        // Clear all the pending interrupts:
        clearInterrupts();

        // enable the PMU interrupts from the HW interface
        theHWInterface->enablePMUInterrupt(); 
    }
    else {        
        // first tell to the PMU thet we do not need any interrupt:
        interruptBits = 0;

#ifdef VERBOSE_LOGS_ON_PMU_INT
        IOLog("OpenPMUInterface::setInterrupts off with flags: 0x%02x\n", interruptBits);
#endif // VERBOSE_LOGS_ON_PMU_INT

        sendMiscCommand (kPMUSetModem1SecInt, (IOByteCount)1, &interruptBits, &iLen, iBuffer);

        // enable the PMU interrupts from the HW interface
        theHWInterface->disablePMUInterrupt();

        // Clear all the pending interrupts:
        clearInterrupts();
    }
}

// --------------------------------------------------------------------------
//
// Method: clearInterrupts
//
// Purpose:
//        clears all the pending interrupts:
void
OpenPMUInterface::clearInterrupts()
{
    UInt8 iBuffer[256];
    IOByteCount iLen = sizeof(iBuffer);
    
    // Read any pending interrupt from PGE, but do not call the
    // clients, we just want to clear it:
    iLen = sizeof(iBuffer);
    sendMiscCommand (kPMUreadINT, (IOByteCount)0, NULL, &iLen, iBuffer);
}

// --------------------------------------------------------------------------
//
// Method: setLCDPower
//
// Purpose:
//        Turns the display on and off:
void
OpenPMUInterface::setLCDPower(bool on)
{
    if (isOldHardware) {
        UInt8 iBuffer[256];
        IOByteCount iLen = sizeof(iBuffer);

        // the brighness (values find by the try and error method)
        // 14 = max. 127 = min.
        UInt8 bBuffer = (on ? 14 : 127);
        iLen = sizeof(iBuffer);
        sendMiscCommand (kPMUSetBrightness, (IOByteCount)1, &bBuffer, &iLen, iBuffer);     
       
        // the contrast (values find by the try and error method)
        // 127 = max. 0 = min.
        UInt8 cBuffer = (on ? 127 : 0);
        iLen = sizeof(iBuffer);
        sendMiscCommand (kPMUSetContrast, (IOByteCount)1, &cBuffer, &iLen, iBuffer);

        // the most left bit sets the mode on or off
        // 0x01 is the bit that controls the screen.
        UInt8 sBuffer = (on ? 0x80 : 0x00) | 0x01;
        iLen = sizeof(iBuffer);
        sendMiscCommand (kPMUpower1Cntl, (IOByteCount)1, &sBuffer, &iLen, iBuffer);

    }
    else {
        // Do not do anything, we do not have anything to do on new pmus
        // since the ndrv does everything there.
    }
}

// --------------------------------------------------------------------------
//
// Method: setHDPower
//
// Purpose:
//        Turns the HD on and off
void
OpenPMUInterface::setHDPower(bool on)
{
    if (isOldHardware) {
        UInt8 iBuffer[256];
        IOByteCount iLen = sizeof(iBuffer);
        
        // the most left bit sets the mode on or off
        // 0x04 is teh bit that controls the HD.
        UInt8 bBuffer = (on ? 0x80 : 0x00) | 0x04;
        sendMiscCommand (kPMUpowerCntl, (IOByteCount)1, &bBuffer, &iLen, iBuffer);
    }
    else {
        // Do not do anything, we do not have anything to do on new pmus.
    }
}

// --------------------------------------------------------------------------
//
// Method: setMediaBayPower
//
// Purpose:
//        turns power on and off to the media bay
void
OpenPMUInterface::setMediaBayPower(bool on)
{
    if (isOldHardware) {
        UInt8 iBuffer[256];
        IOByteCount iLen = sizeof(iBuffer);

        // the most left bit sets the mode on or off
        // 0x08 is the bit that controls the media bay.
        UInt8 bBuffer = (on ? 0x80 : 0x00) | 0x08;
        sendMiscCommand (kPMUpower1Cntl, (IOByteCount)1, &bBuffer, &iLen, iBuffer);
    }
    else {
        // Do not do anything, we do not have anything to do on new pmus.
    }
}


// --------------------------------------------------------------------------
//
// Method: setIRPower
//
// Purpose:
//          turns power on and off to the IR emitter
void
OpenPMUInterface::setIRPower(bool on)
{
    if (isOldHardware) {
        UInt8 iBuffer[256];
        IOByteCount iLen = sizeof(iBuffer);

        // the most left bit sets the mode on or off
        // 0x08 is the bit that controls the IR.
        UInt8 bBuffer = (on ? 0x80 : 0x00) | 0x04;
        sendMiscCommand (kPMUpower1Cntl, (IOByteCount)1, &bBuffer, &iLen, iBuffer);
    }
    else {
        // Do not do anything, we do not have anything to do on new pmus.
    }
}

// --------------------------------------------------------------------------
//
// Method: callPlatformFunction
//
// Purpose:
//         Overides the standard callPlatformFunction to catch all the calls
//         that may be diected to the PMU.
IOReturn
OpenPMUInterface::callPlatformFunction( const OSSymbol *functionName,
                                       bool waitForFunction,
                                       void *param1, void *param2,
                                       void *param3, void *param4 )
{
#ifdef VERBOSE_LOGS_ON_PMU_INT
    IOLog("OpenPMUInterface::callPlatformFunction calling %s %s 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
          functionName->getCStringNoCopy(), (waitForFunction ? "TRUE" : "FALSE"),
          param1, param2, param3, param4);
#endif VERBOSE_LOGS_ON_PMU_INT
    
    // Dispatches the right call:
    if (functionName->isEqualTo(kSendMiscCommand)) {
        SendMiscCommandParameterBlockPtr param = (SendMiscCommandParameterBlockPtr)param1;

//        IOLog("OpenPMUInterface::callPlatformFunction sendMiscCommand 0x%02x %d 0x%08lx 0x%08lx 0x%08lx\n",
//              param->command, param->sLength,  param->sBuffer, param->rLength, param->rBuffer);
        
        if (param != NULL) {
            return sendMiscCommand (param->command, param->sLength,  param->sBuffer, param->rLength, param->rBuffer);
        }
        else
            return kIOReturnBadArgument;
    }
    else if (functionName->isEqualTo(kRegisterForPMUInterrupts)) {
        assert((param1 != 0) && (param2 != 0) && (param3 != 0));
 
        if (registerForPMUInterrupts((UInt8)param1,(OpenPMUClient)param2, (IOService *) param3))
            return kIOReturnSuccess;
        else
            return kIOReturnBadArgument;     
    }
    else if (functionName->isEqualTo(kDeRegisterClient)) {
        assert((param1 != 0) && (param2 != 0));

        if (deRegisterClient((IOService *)param1, (UInt8)param2))
            return kIOReturnSuccess;
        else
            return kIOReturnBadArgument;
    }
    else if (functionName->isEqualTo(kSetLCDPower)) {
        setLCDPower(param1 != NULL);
        return kIOReturnSuccess;
    }
    else if (functionName->isEqualTo(kSetHDPower)) {
        setHDPower(param1 != NULL);
        return kIOReturnSuccess;
    }
    else if (functionName->isEqualTo(kSetMediaBayPower)) {
        setMediaBayPower(param1 != NULL);
        return kIOReturnSuccess;
    }
    else if (functionName->isEqualTo(kSetIRPower)) {
        setIRPower(param1 != NULL);
        return kIOReturnSuccess;
    }
	else if (functionName->isEqualTo(kSleepNow)) {
		putMachineToSleep ();
		return kIOReturnSuccess;
	}

    // If we are here it means we failed to find the correct call
    // so we pass the parametes to the function above:
    return super::callPlatformFunction(functionName, waitForFunction,
                                       param1, param2, param3, param4);
}

// --------------------------------------------------------------------------
//
// Method: downloadFirmware
//
// Purpose:
//         stars the download process for a new pmu firmware. The
//         function does not chesk for the firmware version, it
//         assumes that the caller knows what to do.
IOReturn
OpenPMUInterface::downloadFirmware(OSData *newFirmare)
{
    // First of all gets the pmu-gate, so nobody can enqueue new commands:
    setPMUDriverBusy(true, kPMUdownloadFlash);

    // requests to the command gate to call the download action:
    IOReturn success = commandGate->runAction(pmuCommandGateReFlash, newFirmare);

    // releases the pmu-gate:
    setPMUDriverBusy(false, kPMUdownloadFlash);

    // and hopefully everything went fine:
    return success;
}


// --------------------------------------------------------------------------
//
// Method: hostIsMobile
//
// Purpose:
//          retutns true if on powerbooks.
// This call probaly should be moved in the platform
// expert since it is likely that more tha one driver
// will have a different default behavior if runs
// on mobile (powerbooks) or desktop machines..
bool
OpenPMUInterface::hostIsMobile()
{
    IOService *topProvider = NULL;
    IOService *tmp;

    // goes up all the way to the top to find the root node:
    for (tmp = getProvider(); tmp != NULL; tmp = tmp->getProvider()) {
        topProvider = tmp;
    }

    // and checks if the name starts with powerbook it is a mobile device:
    if (topProvider != NULL) {
		const char *topProviderName = topProvider->getName();
		if (strncmp (topProviderName, "PowerBook", 9) == 0) return true;
		if (strncmp (topProviderName, "AAPL,3500", 9) == 0) return true;
		if (strncmp (topProviderName, "AAPL,3400/2400", 14) == 0) return true;
	}

    // it must be a desktop:
    return false;
}

// =====================================================================================
// The OpenPMU driver: extra functionality and legacy interfaces
// =====================================================================================

#undef super
#define super OpenPMUInterface
OSDefineMetaClassAndStructors(OpenPMU, OpenPMUInterface)

IOService *
OpenPMU::probe(	IOService * 	provider,
				SInt32 	  *	score )
{
	// Want to make sure that we are on one of the machines that is unsupported
	if (getPlatform()->metaCast ("ApplePowerStarPE")) return this;
	
	return NULL;
}

bool
OpenPMU::passiveMatch (OSDictionary *matching, bool changesOK)
{
	// The problem is that other drivers call waitForService (serviceMatching ("ApplePMU")).
	// That would be OK if we could inherit from ApplePMU, but we can't, since ApplePMU.h is
	// not available. So, instead, we just
	// override passiveMatch so that this will match ApplePMU. This works OK so long as the
	// other drivers use callPlatformFunction for talking to ApplePMU
	
	OSString *str = OSDynamicCast (OSString, matching->getObject (gIOProviderClassKey));
	if (str && str->isEqualTo ("ApplePMU")) return true;
	return super::passiveMatch (matching, changesOK);
}


// Method: start
//
// Purpose:
//   Finds out the hardware it is running on and creates the best interface to
//   drive this PMU. Initializes the interface and sets up its own workloop to
//   drive the pmu requests.
bool
OpenPMU::start(IOService *nub)
{
    // We call the parent that in this case sets up all the
    // workloop and the basic functinos. It will also add the
    // interface to the via.
    if (!super::start(nub))
        return false;

    // We are upa nad running, so the kernel is trustable
    // and we can use the interrupt engine.
    theHWInterface->trustTheKernel(true);
    
    // Say "system ready" and turns the pmu interrupts on:
    saySystemReady();
    setInterrupts(true);

    // by default behaves as a file server:
    if (hostIsMobile())
        setFileServerMode(false);
    else
        setFileServerMode(true);

    // And by default the wake on ring is false:
    setWakeOnRing(false);
    
    // This make sure that we can easly be found:
    registerService();

    // Before to go to sleep we wish to disable the napping mode so that the PMU
    // will not shutdown the system while going to sleep:
    IOService *service = waitForService(serviceMatching("IOPMrootDomain"));
    IOPMrootDomain *pmRootDomain = OSDynamicCast(IOPMrootDomain, service);
    if (pmRootDomain != 0) {
        IOLog("Register OpenPMU to acknowledge power changes\n");
        pmRootDomain->registerInterestedDriver(this);
    }
    
    // Well, if we are here everything was fine. The last thing to do is.
    // to create our clients and attach ourself to them. We have to call
    // it in a separate thread so that if the clients needs the full services
    // of the PMU when they start the pmu will be ready up and running.
    AbsoluteTime deadline;
    clock_interval_to_deadline( 1000, kMillisecondScale, &deadline);
    CallFuncDesync(&allocateInterfacesCaller, this, &deadline);
    
    return true;
}

// --------------------------------------------------------------------------
//
// Method: stop
//
// Purpose:
//   Release all driver resources.
void
OpenPMU::stop(IOService *provider)
{
    // turns the interrupt off:
    setInterrupts(false);
    
    // Not much to do after all:
    freeInterfaces();

    // Call the parent's one too:
    super::stop(provider);
}

// --------------------------------------------------------------------------
//
// Method: powerStateWillChangeTo
//
// Purpose:
//        we are notified here of power changes in the root domain.  The root domain
//        cannot actually turn itself on and off, but it notifies us anyway.
//
//        If power is going down in the root domain, tell the PMU what events
//        can wake the system.  (There will be more power-down activity in
//        powerStateDidChangeTo after other objects in the system have
//        finished using the PMU.)
//
//        If power is coming on in the root domain, open up our request queue
//        and process anything on it.  (It was closed on power down)
IOReturn
OpenPMU::powerStateWillChangeTo (IOPMPowerFlags theFlags, unsigned long, IOService*)
{
    // Powering up:
    if (theFlags & IOPMPowerOn) {
#ifdef VERBOSE_LOGS_ON_PMU
        IOLog("OpenPMU::powerStateWillChangeTo setting awake status\n");
#endif // VERBOSE_LOGS_ON_PMU

        wakeUp();

#ifdef VERBOSE_LOGS_ON_PMU
        IOLog("OpenPMU::powerStateWillChangeTo re-enable the interrupts\n");
#endif // VERBOSE_LOGS_ON_PMU
        
        setInterrupts(true);
    }

#ifdef VERBOSE_LOGS_ON_PMU
    IOLog("OpenPMU::powerStateWillChangeTo acknoledging power change\n");
#endif // VERBOSE_LOGS_ON_PMU

    return IOPMAckImplied;
}


// --------------------------------------------------------------------------
//
// Method: powerStateDidChangeTo
//
// Purpose:
//       we are notified here of power changes in the root domain.  The root domain
//       cannot actually turn itself on and off, but it notifies us anyway.
//
//      If power is being brought down, then the system is about to sleep.  We
//      instruct the PMU to prepare to sleep the system.
IOReturn
OpenPMU::powerStateDidChangeTo ( IOPMPowerFlags theFlags, unsigned long, IOService*)
{
    // powering down:
   if (!(theFlags & IOPMPowerOn) ) {
#ifdef VERBOSE_LOGS_ON_PMU
       IOLog("OpenPMU::powerStateDidChangeTo setting sleep conditions\n");
#endif // VERBOSE_LOGS_ON_PMU

	   kprintf("OpenPMU::powerStateDidChangeTo will call preSleepSequence\n");
	   
       preSleepSequence();

#ifdef VERBOSE_LOGS_ON_PMU
       IOLog("OpenPMU::powerStateDidChangeTo disable the interrupts\n");
#endif // VERBOSE_LOGS_ON_PMU

	   kprintf("OpenPMU::powerStateDidChangeTo setting interrupts false\n");

	   setInterrupts(false);

#ifdef VERBOSE_LOGS_ON_PMU
       IOLog("OpenPMU::powerStateDidChangeTo going to sleep\n");
#endif // VERBOSE_LOGS_ON_PMU

//	   kprintf ("OpenPMU::powerStateDidChangeTo putMachineToSleep\n");
	   
//     putMachineToSleep();

	   kprintf("OpenPMU::powerStateDidChangeTo returning\n");
    }

#ifdef VERBOSE_LOGS_ON_PMU
    IOLog("OpenPMU::powerStateDidChangeTo acknoledging power change\n");
#endif // VERBOSE_LOGS_ON_PMU

    return IOPMAckImplied;
}

// --------------------------------------------------------------------------
//
// Method: newUserClient
//
// Purpose:
//        newUserClient is called by the IOKit manager to create the
//        kernel receiver of a user request. The "type" is a qualifier
//        shared between the kernel and user client class instances..

#define kOpenPMUUserClientMagicCookie 0x0101BEEF

IOReturn
OpenPMU::newUserClient(task_t        owningTask,
                        void          *securityToken,
                        UInt32        magicCookie,
                        IOUserClient  **handler)
{
    IOReturn ioReturn = kIOReturnSuccess;
    OpenPMUUserClient *client = NULL;

    IOLog("OpenPMU::newUserClient\n");

    if (IOUserClient::clientHasPrivilege(securityToken, "root") != kIOReturnSuccess) {
        IOLog("OpenPMU::newUserClient: Can't create user client, not privileged\n");
        return kIOReturnNotPrivileged;
    }

    // Check that this is a user client type that we support.
    // type is known only to this driver's user and kernel
    // classes. It could be used, for example, to define
    // read or write privileges. In this case, we look for
    // a private value.
    if (magicCookie == kOpenPMUUserClientMagicCookie) {
        // Construct a new client instance for the requesting task.
        // This is, essentially  client = new OpenPMUUserClient;
        //				... create metaclasses ...
        //				client->setTask(owningTask)
        client = OpenPMUUserClient::withTask(owningTask);
        if (client == NULL) {
            ioReturn = kIOReturnNoResources;
            IOLog("OpenPMU::newUserClient: Can't create user client\n");
        }
    }
    else {
        ioReturn = kIOReturnInvalid;
        IOLog("OpenPMU::newUserClient: bad magic cookie.\n");
    }

    if (ioReturn == kIOReturnSuccess) {
        // Attach ourself to the client so that this client instance
        // can call us.
        if (client->attach(this) == false) {
            ioReturn = kIOReturnError;
            IOLog("OpenPMU::newUserClient: Can't attach user client\n");
        }
    }

    if (ioReturn == kIOReturnSuccess) {
        // Start the client so it can accept requests.
        if (client->start(this) == false) {
            ioReturn = kIOReturnError;
            IOLog("OpenPMU::newUserClientt: Can't start user client\n");
        }
    }

    if (ioReturn != kIOReturnSuccess && client != NULL) {
        client->detach(this);
        client->release();
    }

    *handler = client;
    return (ioReturn);
}

// --------------------------------------------------------------------------
//
// Method: allocateInterfacesCaller
//
// Purpose:
//         a delayed caller for the creation of the interfaces that attach to
//         the pmu driver.
/* static */ void
OpenPMU::allocateInterfacesCaller( thread_call_param_t arg, thread_call_param_t)
{
    OpenPMU *pmu = OSDynamicCast(OpenPMU, (OSObject*)arg);
    
    // if the argument is really a pmu driver:
    if (pmu != NULL)
        pmu->allocateInterfaces();
}

// --------------------------------------------------------------------------
//
// Method: allocateInterfaces
//
// Purpose:
//   The pmu "knows" which services can provide (it is in its property table)
//   so it is the only device with the "authority" to create its clients. If
//   the function fails to create one of the clients it will however continue
//   to create the others. It is up to the caller to decide what to do (if to
//   completly fail, or try to conitinue to run).
bool
OpenPMU::allocateInterfaces()
{
    bool success = true;

    // gets the provider (which property we should look at):
    IOService *myProvider = getProvider();

    // Do not create the OpenPMUNVRAMController if the no-nvram property is set.
    if (!myProvider->getProperty("no-nvram")) {
        ourNVRAMinterface = new OpenPMUNVRAMController;

        if (ourNVRAMinterface != NULL) {
            if (ourNVRAMinterface->init(0,this) &&
                ourNVRAMinterface->attach(this) &&
                ourNVRAMinterface->start(this)) {
#ifdef VERBOSE_LOGS_ON_PMU
                IOLog("OpenPMU::allocateInterfaces ourNVRAMinterface is up and running\n");
#endif // VERBOSE_LOGS_ON_PMU
            }
            else {
#ifdef VERBOSE_LOGS_ON_PMU
                IOLog("OpenPMU::allocateInterfaces ourNVRAMinterface is not working\n");
#endif // VERBOSE_LOGS_ON_PMU
                success = false;
            }
        }
    }

    // Creates the real time clock controller and (if successful) we publish its resource:
    ourRTCinterface = new OpenPMURTCController;

    if (ourRTCinterface != NULL) {
        if (ourRTCinterface->init(0,this) &&
            ourRTCinterface->attach(this) &&
            ourRTCinterface->start(this)) {
#ifdef VERBOSE_LOGS_ON_PMU
            IOLog("OpenPMU::allocateInterfaces ourRTCinterface is up and running\n");
#endif // VERBOSE_LOGS_ON_PMU
        }
        else {
#ifdef VERBOSE_LOGS_ON_PMU
            IOLog("OpenPMU::allocateInterfaces ourRTCinterface is not working\n");
#endif // VERBOSE_LOGS_ON_PMU
            success = false;
        }
    }

    // Creates the power controller:
    ourPwrinterface = new OpenPMUPwrController;

    if (ourPwrinterface != NULL) {
        if (ourPwrinterface->init(0,this) &&
            ourPwrinterface->attach(this) &&
            ourPwrinterface->start(this)) {
#ifdef VERBOSE_LOGS_ON_PMU
            IOLog("OpenPMU::allocateInterfaces ourPwrinterface is up and running\n");
#endif // VERBOSE_LOGS_ON_PMU
         }
        else {
#ifdef VERBOSE_LOGS_ON_PMU
            IOLog("OpenPMU::allocateInterfaces ourPwrinterface is not working\n");
#endif // VERBOSE_LOGS_ON_PMU
            success = false;
        }
    }

    // Why do we do this ?

    return setupPlatformExpertInterfaces();
}

// --------------------------------------------------------------------------
//
// Method: freeInterfaces
//
// Purpose:
//        releases the clients:
bool
OpenPMU::freeInterfaces()
{
    bool success = true;

//    if (ourADBinterface != NULL) {
//        ourADBinterface->stop(this);
//        ourADBinterface->release();
//        ourADBinterface = NULL;
//    }
//    else
//        success = false;

    if (ourNVRAMinterface != NULL) {
        ourNVRAMinterface->stop(this);
        ourNVRAMinterface->release();
        ourNVRAMinterface = NULL;
    }
    else
        success = false;

    if (ourRTCinterface != NULL) {
        ourRTCinterface->stop(this);
        ourRTCinterface->release();
        ourRTCinterface = NULL;
    }
    else
        success = false;

    if (ourPwrinterface != NULL) {
        ourPwrinterface->stop(this);
        ourPwrinterface->release();
        ourPwrinterface = NULL;
    }
    else
        success = false;

    return (success & disablePlatformExpertInterfaces());
}

// --------------------------------------------------------------------------
//
// Method: OpenPMU_pmu_PE_halt_restart
//
// Purpose:
//	   resets the CPU. or powers off the machine.

/* static */ int
OpenPMU:: OpenPMU_pmu_PE_halt_restart ( unsigned int type )
{
    if (OpenPMU::applePMUReference == NULL)
        return 0;

	switch( type ) {
	    case kPERestartCPU:
        	OpenPMU::applePMUReference->rebootSystem();
        break;

    	case kPEHaltCPU:
    		OpenPMU::applePMUReference->shutdownSystem();
        break;

    	default:
        return 1;
	}

	// workaround 2377033; avoid memory accesses after this point
	ml_set_interrupts_enabled(false);

	while(true) {
            // just to make the compiler happy
        }

	return 1;
}

// --------------------------------------------------------------------------
//
// Method: OpenPMU_pmu_PE_read_write_time_of_day
//
// Purpose:
/* static */ int
OpenPMU:: OpenPMU_pmu_PE_read_write_time_of_day ( unsigned int options, long * secs )
{
    if (OpenPMU::applePMUReference == NULL)
        return 0;

    if( (options == kPEReadTOD) && (OpenPMU::applePMUReference->ourRTCinterface != NULL) ) {
        UInt8	currentTime[8];
        IOByteCount length, i;
        long	longTime = 0;

        length = sizeof(currentTime);
        OpenPMU::applePMUReference->ourRTCinterface->getRealTimeClock(currentTime,&length);

        for ( i = 0; i < length; i++ ) {
            longTime |= currentTime[i] << ((length-i-1)*8);
        }
        *secs = longTime;
    }

    if( (options == kPEWriteTOD) && (OpenPMU::applePMUReference->ourRTCinterface != NULL) ) {
        OpenPMU::applePMUReference->ourRTCinterface->setRealTimeClock((UInt8 *)secs);
    }

    return 0;
}

// --------------------------------------------------------------------------
//
// Method: OpenPMU_pmu_PE_write_IIC
//
// Purpose:
/* static */ int
OpenPMU:: OpenPMU_pmu_PE_write_IIC ( unsigned char, unsigned char, unsigned char )
{
    if (OpenPMU::applePMUReference == NULL)
        return 0;

    return 1;
}

// --------------------------------------------------------------------------
//
// Method: OpenPMU_pmu_PE_poll_input
//
// Purpose
/* static */ int
OpenPMU:: OpenPMU_pmu_PE_poll_input ( unsigned int, char *  )
{
    if (OpenPMU::applePMUReference == NULL)
        return 0;

    return 1;
}

// --------------------------------------------------------------------------
//
// Method: setupPlatformExpertInterfaces
//
// Purpose:
//         attach the local functions to the pointers held by the
//         platform expert:

// sets the global pointer to NULL by default.
OpenPMU *OpenPMU::applePMUReference = NULL;

bool
OpenPMU::setupPlatformExpertInterfaces()
{
    PE_poll_input = OpenPMU_pmu_PE_poll_input;
    PE_halt_restart = OpenPMU_pmu_PE_halt_restart;
    PE_write_IIC =  OpenPMU_pmu_PE_write_IIC;
    PE_read_write_time_of_day = OpenPMU_pmu_PE_read_write_time_of_day;
    OpenPMU::applePMUReference = this;

    publishResource( "IOiic0", this );
    publishResource( "IORTC", this );

    return true;
}

// -------------------------------------------------------------------------
//
// Method: disablePlatformExpertInterfaces
//
// Purpose:
//         there may be a moment when the pmu services are mo more available
//         at this point we do not want the PE to access to the PMU since the
//         behavior of the PMU is unkownn. This function disables the access
//         to the PMU:
bool
OpenPMU::disablePlatformExpertInterfaces()
{
    OpenPMU::applePMUReference = NULL;
    return true;
}

// -------------------------------------------------------------------------

IOReturn
OpenPMUSendMiscCommand( UInt32 command, IOByteCount  sendLength, UInt8 * sendBuffer,
                         IOByteCount * readLength, UInt8 * readBuffer )
{
    static OpenPMU * pmu;

    if (!pmu) {
        OSIterator * iter = IOService::getMatchingServices(
                                IOService::serviceMatching("OpenPMU"));
        if (iter) {
            pmu = OSDynamicCast( OpenPMU, iter->getNextObject());
            iter->release();
        }
    }

    if (!pmu)
        return(kIOReturnUnsupported);

    return( pmu->sendMiscCommand( command, sendLength, sendBuffer,
                                           readLength, readBuffer ));
}

