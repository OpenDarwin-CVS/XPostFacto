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
 * 24 Nov 1998 suurballe  Created.
 */

#include <IOKit/power/IOPwrController.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/pwr_mgt/IOPMPowerSourceList.h>

class OpenPMU;
class IOPlatformExpert;

class OpenPMUPwrController : public IOService
{
    OSDeclareDefaultStructors(OpenPMUPwrController)

private:

    OpenPMU *            PMUdriver;
    IOPMPowerSourceList * powerSources;
    UInt8                 bootEnvIntData;
    UInt8                 lastEnvIntData;

    void        getInitialSwitchState (void);

    // This is the time when we did the last battery check.
    AbsoluteTime lastPolling;

    // this is the minimal distance between two battery check.
    // probably 3 seconds sounds more or less right.
    enum {
        kMinPollingTime = 3000
    };

public:

    IOPMrootDomain *      rootPowerDomain;
    IOPlatformExpert *    pe;
    
    bool 	init (OSDictionary * regEntry, OpenPMU * driver);
    bool 	start (IOService * provider);
    void        handleEnvInterrupt (UInt8 envIntData);
    void 	sendPowerNotificationToRootDomain (UInt8 command);
    void        updatePowerSources (void);
    bool        powerAvailable (void);

    // serializer for the status of the power sources:
    static bool serializeBatteryInfo(void *target, void *ref, OSSerialize *s);
};




