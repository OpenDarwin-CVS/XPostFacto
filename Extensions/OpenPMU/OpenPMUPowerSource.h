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
#include <IOKit/pwr_mgt/IOPMPowerSource.h>

class OpenPMU;

// SMBus Battery flags

enum {
  SMBusChargerInPlace   = 0,
  SMBusHighCharge       = 1,
  SMBusBatInstalled     = 2
};

struct SMBusBatteryRecord {
  UInt8             batteryDataType;
  UInt8             batteryFlags;
  UInt16            batteryCapacity;
  UInt16            batteryTotalCap;
  SInt16            batteryCurrent;
  UInt16            batteryVoltage;
};
typedef struct SMBusBatteryRecord SMBusBatteryRecord;

#define kTimeRemainingForWarning (10*60) // 10 minutes to battery depleted 

// our battery (power source) object

class OpenPMUPowerSource : public IOPMPowerSource
{
    OSDeclareDefaultStructors(OpenPMUPowerSource)

private:
    // keeps a reference to the pmu driver, so we cn enqueue requests
    // about the state if this power source.
    OpenPMU *  PMUdriver;

    // The state of the power source is also kept in a local dictionary.
    OSDictionary *dict;
    
public:
    bool        init ( OpenPMU * driver, unsigned short whichBatteryIndex );
    void        free (void);

    // current status as dictionary:
    OSDictionary *currentStatus();
    
    // calculations
    void        updateStatus (void);
};



