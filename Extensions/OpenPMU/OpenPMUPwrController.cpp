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
 *  1 Dec 1998 suurballe  Created.
 */

#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IOSyncer.h>
#include "OpenPMUPwrController.h"
#include "OpenPMUPowerSource.h"
#include "OpenPMU.h"

#define super IOService

bool rootPowerDomainUp( OSObject * us, void * ref, IOService * yourDevice );
void powerCallback (IOService * client, UInt8 interruptMask, UInt32 length, UInt8 * buffer);

OSDefineMetaClassAndStructors(OpenPMUPwrController, IOService)

// **********************************************************************************
// init
//
// **********************************************************************************

bool OpenPMUPwrController::init ( OSDictionary * regEntry, OpenPMU * driver ) 
{
    PMUdriver = driver;

    rootPowerDomain = NULL;

    pe = NULL;

    lastEnvIntData = 0;
    
    return super::init(regEntry);
}

// **********************************************************************************
// start
//
// **********************************************************************************

bool OpenPMUPwrController::start (IOService * provider)
{
    if (!super::start(provider))
        return false;

    if (!PMUdriver)
        return false;

    // find the root power domain

    IONotifier * publishNotify = addNotification( gIOPublishNotification,serviceMatching("IOPMrootDomain"),
                                                  (IOServiceNotificationHandler)&rootPowerDomainUp, this, 0 );

    if (publishNotify == NULL)
        kprintf("OpenPMUPwrController::start -> addNotification failed\n");

    pe = IOPlatformExpert::getPlatform();

    // initialized power sources attached to this system

    powerSources = 0;
    updatePowerSources ();

    // register a callback to handle environment changes either
    // as a result of a pmu interrupt or timer interrupt for the
    // case (see below) where we have to poll

    // We do not really care about these interrupts, but if we would this is the
    // way register for interrupts:
    if (!PMUdriver->registerForPMUInterrupts(kPMUenvironmentInt, powerCallback, this)) {
#ifdef VERBOSE_LOGS_ON
        kprintf("OpenPMUPwrController::start registerForPMUInterrupts kPMUenvironmentInt fails\n");
#endif // VERBOSE_LOGS_ON
    }

    // Makes clear that I am the serializer for the battery info
    OSSerializer * infoSerializer = OSSerializer::forTarget((void *) this, &serializeBatteryInfo );
    if (infoSerializer) {
        IORegistryEntry * entry;
        if ( (entry = IORegistryEntry::fromPath("mac-io/battery", gIODTPlane))) {
            entry->setProperty(kIOBatteryInfoKey, infoSerializer );
            entry->release();
        } else if ((entry = IORegistryEntry::fromPath("mac-io/via-pmu/battery", gIODTPlane))) {
            entry->setProperty( kIOBatteryInfoKey, infoSerializer );
            entry->release();
        }
        infoSerializer->release();
    }

    // get initial state of the switches
    getInitialSwitchState ();

    return true;
}

// **********************************************************************************
// updatePowerSources
//
// **********************************************************************************

void OpenPMUPwrController::updatePowerSources (void)
{
  OpenPMUPowerSource * aSource = 0;
  int numBatteriesSupported;

  // There can be many clients polling for info on the power controller
  // so the last client can easly use the info we polled for its predecessor.
  // the following code checks when was the last time we were here and if
  // it is less than the minimum polling preiod exits immedialty.

  // Gets the current time
  AbsoluteTime currentTime;
  clock_get_uptime(&currentTime);

  // gets the interval in a managable way:
  AbsoluteTime minPollingInterval;
  clock_interval_to_deadline(kMinPollingTime, 1000000, &minPollingInterval);

  // We wish to continue if currentTime - minPollingInterval > lastPolling
  SUB_ABSOLUTETIME(&currentTime, &minPollingInterval);

  // Or in other words we wish to stop if (currentTime - minPollingInterval) < lastPolling
  if (CMP_ABSOLUTETIME(&currentTime, &lastPolling) < 0)
      return;

  if (powerSources == 0) {
    powerSources = new IOPMPowerSourceList;
    if (powerSources != 0) {
      powerSources->initialize ();
      numBatteriesSupported = pe->numBatteriesSupported ();
      for (short i = 0; i < numBatteriesSupported; i++) {
	aSource = new OpenPMUPowerSource;
        if (aSource != 0) {
           aSource->init (PMUdriver, i+1);
           powerSources->addToList ((IOPMPowerSource *)aSource);
        }
      }
    }
  }
 
  if (powerSources != 0) {
    aSource = (OpenPMUPowerSource *)powerSources->firstInList ();
    while (aSource) {
      aSource->updateStatus ();
      aSource = (OpenPMUPowerSource *)powerSources->nextInList (aSource);
    }
  }

  // Update the time of the last polling:
  clock_get_uptime(&lastPolling);
}

// **********************************************************************************
// rootPowerDomainUp
//
// **********************************************************************************

bool rootPowerDomainUp( OSObject * us, void * ref, IOService * yourDevice ) 
{
  if ( yourDevice != NULL ) {
      ((OpenPMUPwrController *)us)->rootPowerDomain = (IOPMrootDomain *)yourDevice;
  }
  return true;
}

// **********************************************************************************
// powerCallback
//
// **********************************************************************************

void powerCallback (IOService * client, UInt8 interruptMask, UInt32 length, UInt8 * buffer) 
{
    UInt8 bufferDataByte;

    // let's make sure that the interrupts re dipatched correctly:
    assert ((interruptMask & kPMUenvironmentInt) != 0);

    // handle an incoming event from the PMU that is related to the environment interrupt

    if (client == 0) {
        kprintf ("invalid client\n");
        return;
    }

    if (length < 1) {
        kprintf ("invalid power event len: %d\n",length);
        return; // hmmm...
    }

    if (buffer == NULL) {
        kprintf ("invalid power event data\n");
        return; // hmmm...
    }

    bufferDataByte = *buffer;

    ((OpenPMUPwrController *)client)->handleEnvInterrupt (bufferDataByte);
}

// **********************************************************************************
// handleEnvInterrupt
//
// **********************************************************************************

void OpenPMUPwrController::handleEnvInterrupt (UInt8 envIntData)
{
    UInt8 changedIntBits;
    bool  putToSleep        = true;

    // handle an incoming event from the PMU that is related to the environment interrupt

    changedIntBits = envIntData ^ lastEnvIntData;

    if ( envIntData & kClamshellClosedEventMask) {
	// see if this machine supports going to sleep on case being closed
        if (pe)
          putToSleep = (pe->hasPrivPMFeature(kPMClosedLidCausesSleepMask));
          //                    && (0 == (bootEnvIntData & kClamshellClosedEventMask));

        // on all machines be sure sleep is allowed again since case is closed
        sendPowerNotificationToRootDomain (kIOPMAllowSleep);

        if (putToSleep) {
          // user closed the case/clamshell
          // on machines that support it, we must put the machine to sleep now
          sendPowerNotificationToRootDomain (kIOPMSleepNow);
        }
    }
    else {
       if (pe && pe->hasPrivPMFeature (kPMOpenLidPreventsSleepMask))
          sendPowerNotificationToRootDomain (kIOPMPreventSleep);
    }

    if ( changedIntBits & kACPlugEventMask ) {
        kprintf("ac plug/unplug\n");
        // user plugged or unplugged ac
    }

    if ( envIntData & kFrontPanelButtonEventMask ) {
        // user hit the front panel button which means sleep or wake,
        // the opposite of the current state
        sendPowerNotificationToRootDomain (kIOPMSleepNow);
    }

    if ( envIntData & kBatteryStatusEventMask ) {
        // battery status has changed
        if (powerAvailable () == false)
          sendPowerNotificationToRootDomain (kIOPMSleepNow);
    }

    lastEnvIntData = envIntData;
}

// **********************************************************************************
// powerAvailable
//
// **********************************************************************************

bool OpenPMUPwrController::powerAvailable (void)
{
    IOPMPowerSource * aSource;
    bool powerSourcesAvailable = false;

    updatePowerSources ();

    // This first loop is to check if all the power sources
    // (tipically batteries) are available. If there is not
    // an available power source I assume that power is
    // available (after all we are running this code aren't we ?)
    // and that the batteries are broken or missing.
    for (aSource = powerSources->firstInList ();
         aSource != NULL;
         aSource = powerSources->nextInList (aSource)) {
        if (aSource->isInstalled ()) {
            // we have a power source:
            powerSourcesAvailable |= true;

            // If the AC adaptor is connected we also have power:
            if (aSource->acConnected ())
                return true;

            // If the capacity is still something decent we also
            // have power:
            if (aSource->capacityPercentRemaining () > 0)
                return true;
        }
    }

    // There could be only 2 reasons to be here:
    // 1] there are not power sources available
    // 2] there are power sources available but
    //    they are depleted

    if (powerSourcesAvailable) {
        // we have power sources but they are depleted
        // so we have no power:
        return false;
    }

    // we do not have power sources, but we are running. So I should
    // assume that the AC adaptor is in and working:
    return true;
}

// **********************************************************************************
// sendPowerNotificationToRootDomain
//
// **********************************************************************************

void OpenPMUPwrController::sendPowerNotificationToRootDomain (UInt8 command) 
{
    // call with command

    if (rootPowerDomain != NULL)
        rootPowerDomain->receivePowerNotification (command);
}

// **********************************************************************************
// getInitialSwitchState
//
// **********************************************************************************

void OpenPMUPwrController::getInitialSwitchState (void)
{
    IOReturn ret;
    IOByteCount iLen = sizeof(bootEnvIntData);
    ret = PMUdriver->sendMiscCommand (kPMUreadExtSwitches, 0, NULL, &iLen, &bootEnvIntData);
    if (kIOReturnSuccess == ret)
        getPlatform()->setProperty("AppleExtSwitchBootState", bootEnvIntData, 32);
    lastEnvIntData = bootEnvIntData;
}

// **********************************************************************************
// serializeBatteryInfo
//
// **********************************************************************************
/* static */ bool OpenPMUPwrController::serializeBatteryInfo(void *target, void *ref, OSSerialize *s)
{
    OpenPMUPwrController * pwrController;
    OSArray *		 array;
    bool    success;
    
    pwrController = OSDynamicCast(OpenPMUPwrController, (OSObject*)target);
    if (pwrController == NULL)
        return false;

    array = OSArray::withCapacity(2);
    if(array == NULL)
        return false;

    pwrController->updatePowerSources();

    if (pwrController->powerSources != 0) {
    OpenPMUPowerSource *aSource = (OpenPMUPowerSource *)pwrController->powerSources->firstInList ();
        while (aSource) {
            OSDictionary *status = aSource->currentStatus();

            if (status != NULL)
                array->setObject(status);
            
            aSource = (OpenPMUPowerSource *)pwrController->powerSources->nextInList (aSource);
        }
    }

    success = array->serialize(s);
    array->release();

    return success;
}



