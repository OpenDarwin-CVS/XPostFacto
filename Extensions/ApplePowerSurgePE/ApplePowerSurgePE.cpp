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

#include "ApplePowerSurgePE.h"
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <libkern/c++/OSMetaClass.h>

#include "XPFCPUSettings.h"

#define kIOPropertyPhysicalInterconnectLocationKey	"Physical Interconnect Location"
#define kIOPropertyInternalKey						"Internal"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super ApplePlatformExpert

OSDefineMetaClassAndStructors(ApplePowerSurgePE, ApplePlatformExpert);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool 
ApplePowerSurgePE::start(IOService *provider)
{
	// We fix the processor settings made by the NVRAM patch, just in case our
	// custom version of BootX is not installed

	XPFfixProcessorSettings ();
	
	if (!super::start (provider)) return false;

	setChipSetType(kChipSetTypePowerSurge);
  
	_pePMFeatures     = kStdDesktopPMFeatures;
	_pePrivPMFeatures = kStdDesktopPrivPMFeatures;
	_peNumBatteriesSupported = kStdDesktopNumBatteries;
  	
	return true;
}

bool
ApplePowerSurgePE::platformAdjustService (IOService *service)
{
	OSData *data;
	data = OSDynamicCast(OSData, service->getProperty ("name"));
	if (data && data->isEqualTo ("control", 7)) {
		data = OSDynamicCast(OSData, service->getProperty ("linebytes"));
		if (!data) return false;
	}

	if (service->metaCast ("IOSCSIParallelInterfaceProtocolTransport")) {
		service->setProperty (kIOPropertyPhysicalInterconnectLocationKey, kIOPropertyInternalKey);
		return true;
	}
	
	return super::platformAdjustService (service);
}

void
ApplePowerSurgePE::processTopLevel (IORegistryEntry *root)
{
	// Need to adjust the device tree to put the cpu node under a cpus node
	// This is required because Classic looks for it there
	// Added by ryan.rempel@utoronto.ca

	IORegistryEntry *cpus = root->childFromPath ("cpus", gIODTPlane);
	if (cpus) {
		cpus->release ();
        super::processTopLevel (root);
		return;
	}

	cpus = new IORegistryEntry;
	if (cpus) {
        if (!cpus->init ()) {
			cpus->release();
		} else {
			cpus->attachToParent (root, gIODTPlane);

			unsigned property = 1;
			cpus->setProperty ("#address-cells", &property, 4);
			property = 0;
			cpus->setProperty ("#size-cells", &property, 4);
			cpus->setName ("cpus");
			cpus->setLocation ("0");

			OSIterator *children = root->getChildIterator (gIODTPlane);
			if (children) {
				IORegistryEntry *next;
				OSString *cpuMatch = OSString::withCString ("cpu");
				while ((next = (IORegistryEntry *) children->getNextObject ())) {
					if (IODTCompareNubName (next, cpuMatch, NULL)) {
						next->attachToParent (cpus, gIODTPlane);
						next->detachFromParent (root, gIODTPlane);
					}
				}
				cpuMatch->release ();
				children->release ();
			}
		}
	}
    super::processTopLevel (root);
}
	
IOReturn
ApplePowerSurgePE::callPlatformFunction(const OSSymbol *functionName,
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
ApplePowerSurgePE::getDefaultBusSpeeds(long *numSpeeds,
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
ApplePowerSurgePE::PMInstantiatePowerDomains(void)
{
	root = (IOPMrootDomain *) IOPMrootDomain::metaClass->alloc ();
	if (!root) return;
	root->init ();
    root->attach(this);
    root->start(this);
    root->youAreRoot();
    root->setSleepSupported(kRootDomainSleepNotSupported);
}

void
ApplePowerSurgePE::PMRegisterDevice(IOService *theNub, IOService *theDevice)
{
    while ((theNub != NULL) && (theNub->addPowerChild(theDevice) != IOPMNoErr)) {
        theNub = theNub->getProvider();
    }

    if (theNub == NULL) {
        root->addPowerChild(theDevice);
    }
}
