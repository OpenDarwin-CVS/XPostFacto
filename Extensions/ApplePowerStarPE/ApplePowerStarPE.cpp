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
 * Copyright (c) 1999-2001 Apple Computer, Inc.  All rights reserved.
 *
 *  DRI: Josh de Cesare
 *
 */

#include "ApplePowerStarPE.h"

#include "XPFCPUSettings.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/pwr_mgt/RootDomain.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super ApplePlatformExpert

OSDefineMetaClassAndStructors(ApplePowerStarPE, ApplePlatformExpert);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool ApplePowerStarPE::start(IOService *provider)
{
    long machineType;

	// We fix the processor settings made by the NVRAM patch, just in case our
	// custom version of BootX is not installed

	XPFfixProcessorSettings ();
	
	if (!super::start(provider)) return false;
	
    setChipSetType(kChipSetTypePowerStar);
        
    // Set the machine type.
    if (IODTMatchNubWithKeys(provider, "'AAPL,3400/2400'")) {
        // Test O'Hare Machine ID to determine Comet vs. Hooper.
        if (ml_phys_read(0xF3000034) & 0x20000000) {
            machineType = kPowerStarTypeComet;
        } else {
            machineType = kPowerStarTypeHooper;
        }
    } else if (IODTMatchNubWithKeys(provider, "'AAPL,3500'")) {
        machineType = kPowerStarTypeKanga;
    } else if (IODTMatchNubWithKeys(provider, "'AAPL,e407'")) {
        machineType = kPowerStarTypeAlchemy;
    } else if (IODTMatchNubWithKeys(provider, "'AAPL,e411'")) {
        machineType = kPowerStarTypeGazelle;
    } else {
        machineType = kPowerStarTypeOther;
    }
        
    setMachineType(machineType);
    
    switch (getMachineType()) {
        case kPowerStarTypeComet :
            discardMediaBay(provider);
            break;
        
        case kPowerStarTypeHooper :
        case kPowerStarTypeKanga :
            configureEthernet(provider);
            break;
    }
    
    switch (getMachineType()) {
        case kPowerStarTypeComet :
        case kPowerStarTypeHooper :
        case kPowerStarTypeKanga :
            _hasPMU = true;
            _pePMFeatures     = kStdPowerBookPMFeatures;
            _pePrivPMFeatures = kStdPowerBookPrivPMFeatures;
            _peNumBatteriesSupported = 1;
            break;
        
        case kPowerStarTypeAlchemy :
        case kPowerStarTypeGazelle :
        case kPowerStarTypeOther :
            _pePMFeatures     = kStdDesktopPMFeatures;
            _pePrivPMFeatures = kStdDesktopPrivPMFeatures;
            _peNumBatteriesSupported = kStdDesktopNumBatteries;
            break;
    }

    return true;
}

void
ApplePowerStarPE::processTopLevel (IORegistryEntry *root)
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

bool ApplePowerStarPE::platformAdjustService(IOService *service)
{
    OSData	*tmpData;
    IOService	*battery;
    
    if (!strcmp(service->getName(), "chips65550")) {
        service->setProperty("Ignore VBL", (void *) "", 0);
        return true;
    }
    
    if (_hasPMU && !strcmp(service->getName(), "adb")) {
        service->setProperty("compatible", (void *) "pmu", 3);
        return true;
	}
    
    return true;
}

void ApplePowerStarPE::discardMediaBay(IOService *provider)
{
    IORegistryEntry *entry;
    
    entry = provider->childFromPath("bandit/ohare/media-bay", gIODTPlane);
    if (entry != 0) {
        entry->detachAbove(gIODTPlane);
        entry->release();
    }
    
    entry = provider->childFromPath("bandit/ohare/ata@21000", gIODTPlane);
    if (entry != 0) {
        entry->detachAbove(gIODTPlane);
        entry->release();
    }
}

void ApplePowerStarPE::configureEthernet(IOService *provider)
{
    OSCollectionIterator *nodeList;
    IORegistryEntry      *node, *enet, *ohare;
    OSArray              *interruptNames, *interruptSources;
    OSSymbol             *interruptControllerName;
    OSData               *tempData;
    long                 tempSource;
    
    enet = 0;
    ohare = 0;
    
    // Find the node for DEC21041.
    nodeList = IODTFindMatchingEntries(provider, kIODTRecursive,
                                       "'pci1011,14'");
    if (nodeList) {
        while ((node = (IORegistryEntry *)nodeList->getNextObject())) {
            enet = node;
        }
        nodeList->release();
    }
    
    if (enet == 0) return;
    
    // Set the 'Network Connection' property to '10BaseT'.
    enet->setProperty("Network Connection", "10BaseT");
    
    // Add a 'built-in' property so IONetworkStack will treat it as built in.
    enet->setProperty("built-in", (void *) "", 0);
    
    // If it is there, find the node for the second ohare.
    nodeList = IODTFindMatchingEntries(provider, kIODTRecursive,
                                       "'pci106b,7'");
    if (nodeList) {
        while ((node = (IORegistryEntry *)nodeList->getNextObject())) {
            ohare = node;
        }
        nodeList->release();
    }
    
    if (ohare == 0) return;
    
    interruptNames = OSDynamicCast(OSArray, enet->getProperty(gIOInterruptControllersKey));
    interruptControllerName = (OSSymbol *)OSSymbol::withCStringNoCopy("SecondaryInterruptController");
    interruptNames->setObject(0, interruptControllerName);
    interruptControllerName->release();
    
    interruptSources = OSDynamicCast(OSArray, enet->getProperty(gIOInterruptSpecifiersKey));
    tempSource = 28;
    tempData = OSData::withBytes(&tempSource, sizeof(tempSource));
    interruptSources->setObject(0, tempData);
    tempData->release();
}


IOReturn
ApplePowerStarPE::callPlatformFunction(const OSSymbol *functionName,
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
ApplePowerStarPE::getDefaultBusSpeeds(long *numSpeeds,
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
ApplePowerStarPE::PMInstantiatePowerDomains(void)
{
	root = new IOPMrootDomain;
    root->init();
    root->attach(this);
    root->start(this);
    root->youAreRoot();

	switch (getMachineType()) {
        case kPowerStarTypeComet :
        case kPowerStarTypeHooper :
        case kPowerStarTypeKanga :
// eventually I will change this to kRootDomainSleepSupported, once it works
			root->setSleepSupported(kRootDomainSleepNotSupported);
            break;

        case kPowerStarTypeAlchemy :
        case kPowerStarTypeGazelle :
        case kPowerStarTypeOther :
			root->setSleepSupported(kRootDomainSleepNotSupported);
            break;
	}
}

void
ApplePowerStarPE::PMRegisterDevice(IOService *theNub, IOService *theDevice)
{
    while ((theNub != NULL) && (theNub->addPowerChild(theDevice) != IOPMNoErr)) {
        theNub = theNub->getProvider();
    }

    if (theNub == NULL) {
        root->addPowerChild(theDevice);
    }
}
