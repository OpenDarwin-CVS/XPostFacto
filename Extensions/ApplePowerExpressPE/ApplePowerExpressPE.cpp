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

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/pwr_mgt/RootDomain.h>

#include "ApplePowerExpressPE.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super ApplePlatformExpert

OSDefineMetaClassAndStructors(ApplePowerExpressPE, ApplePlatformExpert);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
ApplePowerExpressPE::start(IOService *provider)
{
	OSData *tmpData;

	if (!super::start (provider)) return false;
	
	setChipSetType(kChipSetTypePowerExpress);

	tmpData = OSDynamicCast(OSData, getProperty("senses"));
	if (tmpData) senseArray = (long *)tmpData->getBytesNoCopy();

	_pePMFeatures     = kStdDesktopPMFeatures;
	_pePrivPMFeatures = kStdDesktopPrivPMFeatures;
	_peNumBatteriesSupported = kStdDesktopNumBatteries;

	return true;
}

bool
ApplePowerExpressPE::platformAdjustService(IOService *service)
{
	long           cnt, numInterrupts, sourceNumbers[2];
	OSData         *tmpData;
	OSArray        *controllers, *specifiers;
	const OSSymbol *controller;

	// Fix up the interrupt data.
	controllers = OSDynamicCast(OSArray, service->getProperty(gIOInterruptControllersKey));
	specifiers = OSDynamicCast(OSArray, service->getProperty(gIOInterruptSpecifiersKey));
	if (controllers && specifiers) {
		numInterrupts = specifiers->getCount();
		for (cnt = 0; cnt < numInterrupts; cnt++) {
			// Only change interrupts for MPIC.
			controller = OSDynamicCast(OSSymbol, controllers->getObject(cnt));
			if (controller == gIODTDefaultInterruptController) {
				tmpData = OSDynamicCast(OSData, specifiers->getObject(cnt));
				if (tmpData && (tmpData->getLength() == 4)) {
					sourceNumbers[0] = *(long *)tmpData->getBytesNoCopy();
					sourceNumbers[1] = senseArray[sourceNumbers[0]];
					tmpData = OSData::withBytes(sourceNumbers, 2 * sizeof(long));
					if (tmpData) {
						specifiers->setObject(cnt, tmpData);
						tmpData->release();
					}
				}
			}
		}
	}

	if (IODTMatchNubWithKeys(service, "open-pic")) {
		controller = OSSymbol::withCStringNoCopy("InterruptControllerName");
		service->setProperty(controller, (OSObject *)gIODTDefaultInterruptController);
		return true;
	}

	if (IODTMatchNubWithKeys(service, "bmac")) {
		return false;
	}

	return true;
}

IOReturn
ApplePowerExpressPE::callPlatformFunction(const OSSymbol *functionName,
										bool waitForFunction,
										void *param1, void *param2,
										void *param3, void *param4)
{
	if (functionName == gGetDefaultBusSpeedsKey) {
		getDefaultBusSpeeds((long *)param1, (unsigned long **)param2);
		return kIOReturnSuccess;
	}

	return super::callPlatformFunction(functionName, waitForFunction,
									param1, param2, param3, param4);
}

void
ApplePowerExpressPE::getDefaultBusSpeeds(long *numSpeeds,
									   unsigned long **speedList)
{
	if ((numSpeeds == 0) || (speedList == 0)) return;

	// This is a brain-dead implementation, because I don't know what
	// I'm supposed to return. But at least it lets AppleVIA finish its
	// start method. (I'm told AppleVIA will be fixed soon).
	// Ryan Rempel (ryan.rempel@utoronto.ca)

	*numSpeeds = 0;
	*speedList = 0;
}

void
ApplePowerExpressPE::PMInstantiatePowerDomains(void)
{
	root = new IOPMrootDomain;
    root->init();
    root->attach(this);
    root->start(this);
    root->youAreRoot();
    root->setSleepSupported(kRootDomainSleepNotSupported);
}

void
ApplePowerExpressPE::PMRegisterDevice(IOService *theNub, IOService *theDevice)
{
    while ((theNub != NULL) && (theNub->addPowerChild(theDevice) != IOPMNoErr)) {
        theNub = theNub->getProvider();
    }

    if (theNub == NULL) {
        root->addPowerChild(theDevice);
    }
}

