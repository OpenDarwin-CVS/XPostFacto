/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
#include "OpenPMUPowerSource.h"
#include "OpenPMU.h"
#include <IOKit/IOSyncer.h>

#define super IOPMPowerSource

OSDefineMetaClassAndStructors(OpenPMUPowerSource, IOPMPowerSource)

// **********************************************************************************
// init
//
// **********************************************************************************
bool OpenPMUPowerSource::init (OpenPMU * pmu, unsigned short whichBatteryIndex)
{
  if (!super::init (whichBatteryIndex))
    return false;

  if (pmu == 0)
    return false;

  PMUdriver      = pmu;

  bCurCapacity   = 0;
  bMaxCapacity   = 0;
  bVoltage       = 0;
  bCurrent       = 0;
  bFlags         = 0;
  bTimeRemaining = 0;

  dict = OSDictionary::withCapacity(3);
  
  return true;
}

// **********************************************************************************
// free
//
// **********************************************************************************
void OpenPMUPowerSource::free ()
{
    if (dict) {
        dict->release();
        dict = NULL;
    }

    super::free();
}

// **********************************************************************************
// currentStatus
//
// **********************************************************************************
OSDictionary *OpenPMUPowerSource::currentStatus()
{
    // do not bother if the dictionaty does not hold all the
    // data we need:
    if (dict->getCount() < 5)
        return NULL;

    return dict;
}

// **********************************************************************************
// updateStatus
//
// **********************************************************************************
void OpenPMUPowerSource::updateStatus (void)
{
    UInt8		batteryInfo[16];
    IOReturn		err;
    unsigned char       which = bBatteryIndex;
    IOByteCount         len = sizeof(batteryInfo);

    // by default the battery is not in at all:
    bFlags = 0;

    err = PMUdriver->sendMiscCommand ( kPMUGetSOB, sizeof(which), (UInt8 *)&which, &len, (UInt8 *)&batteryInfo );
    if( kIOReturnSuccess != err) {
        // Fails reading the status of this battery, so exit:
        return;
    }

    switch( batteryInfo[0] ) {
        case 3:
        case 4:
            bCurCapacity = batteryInfo[2];
            bMaxCapacity = batteryInfo[3];
            bCurrent = batteryInfo[4];
            bVoltage = batteryInfo[5];
            break;
        case 5:
            bCurCapacity = (batteryInfo[2] << 8) | batteryInfo[3];
            bMaxCapacity = (batteryInfo[4] << 8) | batteryInfo[5];
            bCurrent = (batteryInfo[6] << 8) | batteryInfo[7];
            bVoltage = (batteryInfo[8] << 8) | batteryInfo[9];
            break;
        default:
            return;
    }

    if (dict != NULL) {
        // Writes in the dictionary the current flags:
        UInt32 tempLong32 = batteryInfo[1];
        OSNumber *num = OSNumber::withNumber(tempLong32, 32);
        if( num) {
            dict->setObject(kIOBatteryFlagsKey, num);
            num->release();
        }

        // the current capacity of the battery:
        tempLong32 = bCurCapacity;
        num = OSNumber::withNumber(tempLong32, 32);
        if( num) {
            dict->setObject(kIOBatteryCurrentChargeKey, num);
            num->release();
        }

        // the maximum capacity of the battery:
        tempLong32 = bMaxCapacity;
        num = OSNumber::withNumber(tempLong32, 32);
        if( num) {
            dict->setObject(kIOBatteryCapacityKey, num);
            num->release();
        }
        
        // the current voltage of the battery:
        tempLong32 = bVoltage;
        num = OSNumber::withNumber(tempLong32, 32);
        if( num) {
            dict->setObject(kIOBatteryVoltageKey, num);
            num->release();
        }
        
        // and the current amperage of the battery:
        tempLong32 = -bCurrent;
        if ( tempLong32 < 0 ) { 
            tempLong32 = 0;		// battery is charging, not draining
        }
        num = OSNumber::withNumber(tempLong32, 32);
        if( num) {
            dict->setObject(kIOBatteryAmperageKey, num);
            num->release();
        }
    }

    // setup flags and values and calculate capacity percentages and time remaining

    if (batteryInfo[1] & (1<<SMBusBatInstalled))
        bFlags |= kBatteryInstalled;

    if (batteryInfo[1] & (1<<SMBusChargerInPlace)) {
        bFlags |= kACInstalled;

        if (bCurrent > 0)
            bFlags |= kBatteryCharging;
    }

    //  kprintf ("cap: %d, maxcap: %d, volt: %d, current: %d, charging: %d\n",
    //   bCurCapacity, bMaxCapacity, bVoltage, bCurrent, (bFlags & kACInstalled) ? 0xFF : 0x00);

    bTimeRemaining = 0;
    if (0 == (bFlags & kACInstalled)) {
        if (bCurrent < 0) {
            bTimeRemaining = (bCurCapacity * kSecondsPerHour ) / -bCurrent;
            if (bTimeRemaining < kTimeRemainingForWarning)
                bFlags |= kBatteryAtWarn;
        }

        if (bCurCapacity == 0) {
            bFlags |= kBatteryDepleted;
        }
    }
}
