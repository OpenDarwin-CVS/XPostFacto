/*

Copyright (c) 2001, 2002
Other World Computing
All rights reserved

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer as the first lines of
   each file.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

3. Redistributions in binary form must retain the link to Other World
   Computing's web site in the application's "About Box."

This software is provided by Other World Computing ``as is'' and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed. In no event
shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or
profits; or business interruption) however caused and on any theory of
liability, whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

*/

#include "SCSIBus.h"

#include <Devices.h>
#include <Files.h>
#include "XPFNameRegistry.h"
#include <HFSVolumes.h>
#include <DriverGestalt.h>
#include <stdio.h>

#include "XPFLog.h"
#include "OFAliases.h"
#include "XPFApplication.h"

SCSIBusList SCSIBus::scsiBusList;
bool SCSIBus::hasBeenInitialized = false;

#define kConnectorProperty "AAPL,connector"
#define kRegProperty "reg"
#define kDeviceTypeProperty "device_type"

#define kSCSIBusNumberUnknown -1

void
SCSIBus::Initialize ()
{
	if (hasBeenInitialized) return;
	
	hasBeenInitialized = true;
	
	if ((Ptr) RegistryEntryIDInit == (Ptr) kUnresolvedCFragSymbolAddress) return;
	
	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	// We iterate through, and collected all entries with a device type 
	// of "scsi" or "scsi-2" or "scsi-3" or "ata"
    while (true) {
        err = RegistryEntryIterate (&cookie, iterOp, &entry, &done);
        if (!done && (err == noErr)) {
  			RegPropertyValueSize propSize;
  			err = RegistryPropertyGetSize (&entry, kDeviceTypeProperty, &propSize);
  			if (err == noErr) {
				char *deviceType = NewPtr (propSize);
				ThrowIfNULL_AC (deviceType);
				ThrowIfOSErr_AC (RegistryPropertyGet (&entry, kDeviceTypeProperty, deviceType, &propSize));
				if (!strcmp (deviceType, "scsi") || !strcmp (deviceType, "scsi-2") ||
						!strcmp (deviceType, "scsi-3") || !strcmp (deviceType, "ata")) {
					SCSIBus *newBus = new SCSIBus (&entry, deviceType);
					scsiBusList.InsertLast (newBus);	
				}
				DisposePtr (deviceType);				
			}
	        RegistryEntryIDDispose (&entry);
        } else {
        	break;
        }
        iterOp = kRegIterContinue;
    }
    RegistryEntryIterateDispose (&cookie);
}

SCSIBus* 
SCSIBus::BusWithNumber (int number)
{
	SCSIBus *retVal = NULL;
	for (SCSIBusIterator iter (&scsiBusList); iter.Current(); iter.Next()) {
		if (iter->busNumber == number) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}

SCSIBus* 
SCSIBus::BusWithRegEntryID (RegEntryID *otherRegEntry)
{
	SCSIBus *retVal = NULL;
	for (SCSIBusIterator iter (&scsiBusList); iter.Current(); iter.Next()) {
		if (RegistryEntryIDCompare (&iter->regEntry, otherRegEntry)) {
			retVal = iter.Current ();
			#if qLogging
				gLogFile << "Found bus WithRegEntryID" << endl_AC;
			#endif
			break;
		}
	}
	if (retVal == NULL) {
		// in theory, this shouldn't happen, because we should have already
		// collected all of the possible registry entries. But if it does, we'll
		// just add this one.
		#if qLogging
			gLogFile << "Creating new SCSIBus with RegEntryID" << endl_AC;
		#endif
		retVal = new SCSIBus (otherRegEntry, "");
		scsiBusList.InsertLast (retVal);
	}
	return retVal;
}

SCSIBus::~SCSIBus ()
{
	RegistryEntryIDDispose (&regEntry);
}

SCSIBus::SCSIBus (RegEntryID *scsiEntry, char *deviceType)
{
	fIsATABus = !strcmp (deviceType, "ata");
	busNumber = kSCSIBusNumberUnknown;

	ThrowIfNULL_AC (scsiEntry);
	RegistryEntryIDCopy (scsiEntry, &regEntry);

	#if qLogging
		gLogFile << "Creating new SCSI Bus" << endl_AC;
	#endif

	char alias [256];
	OFAliases::AliasFor (&regEntry, alias);
	openFirmwareName.CopyFrom (alias);
				
	// Now, we assign the next available bus number. I am making the assumption that the
	// iteration order here is the same as the iteration order when the SCSI Manager is
	// handing out bus numbers. I can think of a couple of reasons why this might actually
	// be true, but I'll just have to test it. It does seem to be true, in my limited testing.

	if (!fIsATABus) {
		if (openFirmwareName == "scsi") {
			busNumber = 1;
		} else if (openFirmwareName == "scsi-int") {
			busNumber = 0;
		} else {
			static int nextPCIBusNumber = 2;
			busNumber = nextPCIBusNumber++;
		}
	}
	
	#if qLogging
		gLogFile << "OpenFirmwareName: ";
		gLogFile.WriteCharBytes ((char *) &openFirmwareName[1], openFirmwareName[0]);
		gLogFile << endl_AC;
		gLogFile << "Bus Number: " << busNumber << endl_AC;
	#endif
	
}
