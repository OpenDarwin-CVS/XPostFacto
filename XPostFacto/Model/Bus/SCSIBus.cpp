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

#include "XPFLog.h"
#include "OFAliases.h"

SCSIBusList SCSIBus::gSCSIBusList;
bool SCSIBus::gHasBeenInitialized = false;

#define kConnectorProperty "AAPL,connector"
#define kRegProperty "reg"
#define kDeviceTypeProperty "device_type"

#define kSCSIBusNumberUnknown -1

int SCSIBus::gBusCount = 0;

void
SCSIBus::Initialize ()
{
	if (gHasBeenInitialized) return;
	
	gHasBeenInitialized = true;
	
	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	// We iterate through, and collected all entries with a device type 
	// of "scsi" or "scsi-2" or "scsi-3"
    while (true) {
        err = RegistryEntryIterate (&cookie, iterOp, &entry, &done);
        if (!done && (err == noErr)) {
  			RegPropertyValueSize propSize;
  			err = RegistryPropertyGetSize (&entry, kDeviceTypeProperty, &propSize);
  			if (err == noErr) {
				char *deviceType = NewPtr (propSize + 1);
				ThrowIfNULL_AC (deviceType);
				ThrowIfOSErr_AC (RegistryPropertyGet (&entry, kDeviceTypeProperty, deviceType, &propSize));
				deviceType[propSize] = '\0';
				if (!strcmp (deviceType, "scsi") || !strcmp (deviceType, "scsi-2") ||
						!strcmp (deviceType, "scsi-3")) {
					SCSIBus *newBus = new SCSIBus (&entry);
					gSCSIBusList.InsertLast (newBus);	
					gBusCount++;
				}
				DisposePtr (deviceType);				
			} else {
				// It had no device_type. So it might be the UltraTek100+ with the 2.3.5 firmware
				// If so, pretend that it is a SCSI bus
				err = RegistryPropertyGetSize (&entry, "name", &propSize);
				if (err == noErr) {
					char *name = NewPtr (propSize + 1);
					ThrowIfOSErr_AC (RegistryPropertyGet (&entry, "name", name, &propSize));
					name[propSize] = '\0';	
					if (!strcmp (name, "UltraTek100+")) {
						SCSIBus *newBus = new SCSIBus (&entry);
						gSCSIBusList.InsertLast (newBus);	
						gBusCount++;
					}	
					DisposePtr (name);
				}
			}
	        RegistryEntryIDDispose (&entry);
        } else {
        	break;
        }
        iterOp = kRegIterContinue;
    }
    RegistryEntryIterateDispose (&cookie);
    
    // Now, we look to see whether there is an internal and external built-in bus.
    // That is, whether bus number 0 and 1 have been assigned. If not, we adjust 
    // the other bus numbers.
        
    int offset = 0;
    for (int x = 0; true; x++) {
    	SCSIBus *bus = BusWithNumber (x);
    	if (bus) {
    		bus->fBusNumber -= offset;
    	} else {
    		offset++;
    		if (x > 1) break;
    	}
    }
}

SCSIBus* 
SCSIBus::BusWithNumber (int number)
{
	SCSIBus *retVal = NULL;
	for (SCSIBusIterator iter (&gSCSIBusList); iter.Current(); iter.Next()) {
		if (iter->fBusNumber == number) {
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
	for (SCSIBusIterator iter (&gSCSIBusList); iter.Current(); iter.Next()) {
		if (RegistryEntryIDCompare (&iter->fRegEntry, otherRegEntry)) {
			retVal = iter.Current ();
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
		retVal = new SCSIBus (otherRegEntry);
		gSCSIBusList.InsertLast (retVal);
	}
	return retVal;
}

SCSIBus*
SCSIBus::BusWithOpenFirmwareName (CStr255_AC *ofName)
{
	SCSIBus *retVal = NULL;
	for (SCSIBusIterator iter (&gSCSIBusList); iter.Current(); iter.Next()) {
		if (iter->fOpenFirmwareName == *ofName) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}

SCSIBus::~SCSIBus ()
{
	RegistryEntryIDDispose (&fRegEntry);
}

SCSIBus::SCSIBus (RegEntryID *scsiEntry)
{
	fBusNumber = kSCSIBusNumberUnknown;
	fIsActuallyATABus = false;

	ThrowIfNULL_AC (scsiEntry);
	RegistryEntryIDCopy (scsiEntry, &fRegEntry);

	char alias [256];
	char shortAlias [256];
	OFAliases::AliasFor (&fRegEntry, alias, shortAlias);
	fOpenFirmwareName.CopyFrom (alias);
	fShortOpenFirmwareName.CopyFrom (shortAlias);
				
	// Now, we assign the next available bus number. I am making the assumption that the
	// iteration order here is the same as the iteration order when the SCSI Manager is
	// handing out bus numbers. I can think of a couple of reasons why this might actually
	// be true, but I'll just have to test it. It does seem to be true, in my limited testing.

	if (fOpenFirmwareName == "scsi") {
		fBusNumber = 1;
	} else if (fOpenFirmwareName == "scsi-int") {
		fBusNumber = 0;
	} else {
		static int nextPCIBusNumber = 2;
		fBusNumber = nextPCIBusNumber++;
	}	
	
	// Now, as an inelegant workaround, if this bus has a function number, then swap bus numbers
	// with the one that has function number 0
	
	char test = fOpenFirmwareName[fOpenFirmwareName.Length() - 1];
	if (test == ',') {
		CStr255_AC otherName = fOpenFirmwareName.Copy (1, fOpenFirmwareName.Length() - 2);
		SCSIBus *otherBus = BusWithOpenFirmwareName (&otherName);
		if (otherBus) {
			int otherBusNumber = otherBus->fBusNumber;
			otherBus->fBusNumber = fBusNumber;
			fBusNumber = otherBusNumber;
		}	
	}	
	
	// Now, see if this is actually an ATA bus
	// This doesn't work for all ATA cards in Mac OS 9, because I can't always tell whether
	// Mac OS X is going to treat them as ATA or SCSI. It depends on the card's firmware.
	// There probably is still a clue in the name registry, but I may have to special case 
	// each card, since the clues may be different.
	
	RegEntryIter cookie;
    RegEntryID foundEntry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterChildren;
    OSErr err = RegistryEntryIterateCreate (&cookie);
    RegistryEntryIterateSet (&cookie, scsiEntry);
    RegistryEntryIDInit (&foundEntry);

    while (true) {
        err = RegistryEntryIterate (&cookie, iterOp, &foundEntry, &done);
        if (!done && (err == noErr)) {		   	
  			RegPropertyValueSize propSize;
  			err = RegistryPropertyGetSize (&foundEntry, "ATA-Channel", &propSize);
  			if ((err == noErr) && (propSize == 4)) {
 			   	fIsActuallyATABus = true;

				unsigned channel;
				ThrowIfOSErr_AC (RegistryPropertyGet (&foundEntry, "ATA-Channel", &channel, &propSize));
				OFAliases::AliasFor (&foundEntry, alias, shortAlias);

				if (channel == 0) {
					fATAOpenFirmwareName0.CopyFrom (alias);
					fATAShortOpenFirmwareName0.CopyFrom (shortAlias);
				} else if (channel == 1) {
					fATAOpenFirmwareName1.CopyFrom (alias);
					fATAShortOpenFirmwareName1.CopyFrom (shortAlias);				
				}	
				
			}
	        RegistryEntryIDDispose (&foundEntry);
        } else {
        	break;
        }
        iterOp = kRegIterContinue;
    }
    RegistryEntryIterateDispose (&cookie);

	#if qLogging
		gLogFile << "SCSI Bus OpenFirmwareName: ";
		gLogFile.WriteCharBytes ((char *) &fShortOpenFirmwareName[1], fShortOpenFirmwareName[0]);
		gLogFile << endl_AC;
		gLogFile << "Bus Number: " << fBusNumber << endl_AC;
		if (fIsActuallyATABus) {
			gLogFile << "ATA Channel 0 Name: ";
			gLogFile.WriteCharBytes ((char *) &fATAShortOpenFirmwareName0[1], fATAShortOpenFirmwareName0[0]);
			gLogFile << endl_AC;
			gLogFile << "ATA Channel 1 Name: ";
			gLogFile.WriteCharBytes ((char *) &fATAShortOpenFirmwareName1[1], fATAShortOpenFirmwareName1[0]);
			gLogFile << endl_AC;
		}
	#endif

}
