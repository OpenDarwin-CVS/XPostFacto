/*

Copyright (c) 2002 - 2003
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

#include "ATABus.h"

#include "XPFLog.h"
#include "OFAliases.h"
#include "XPFApplication.h"

ATABusList ATABus::gATABusList;
bool ATABus::gHasBeenInitialized = false;
int ATABus::gBusCount = 0;

#define kConnectorProperty "AAPL,connector"
#define kRegProperty "reg"
#define kDeviceTypeProperty "device_type"

void
ATABus::Initialize ()
{
	if (gHasBeenInitialized) return;
	
	gHasBeenInitialized = true;
		
	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	// We iterate through, and collected all entries with a device type "ata"
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
				if (!strcmp (deviceType, "ata") || !strcmp (deviceType, "ATA") || !strcmp (deviceType, "ide")) {
					// skip if FrmTek
					err = RegistryPropertyGetSize (&entry, "name", &propSize);
					char *name = NewPtr (propSize + 1);
					ThrowIfOSErr_AC (RegistryPropertyGet (&entry, "name", name, &propSize));
					name[propSize] = '\0';
					
					if (!strstr (name, "FrmTk")) {
						ATABus *newBus = new ATABus (&entry);
						gATABusList.InsertLast (newBus);	
						gBusCount++;
					}
					
					DisposePtr (name);
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


ATABus::~ATABus ()
{
	RegistryEntryIDDispose (&fRegEntryID);
}

#define kBusIDPropName "AAPL,bus-id"

ATABus::ATABus (RegEntryID *regEntry)
{
	ThrowIfNULL_AC (regEntry);
	RegistryEntryIDCopy (regEntry, &fRegEntryID);

	#if qLogging
		gLogFile << "Creating new ATA Bus" << endl_AC;
	#endif
	
	char alias [256];
	char shortAlias [256];
	OFAliases::AliasFor (regEntry, alias, shortAlias);
	fOpenFirmwareName.CopyFrom (alias);
	fShortOpenFirmwareName.CopyFrom (shortAlias);
	
	RegPropertyValueSize propSize;
	OSErr err = RegistryPropertyGetSize (regEntry, kBusIDPropName, &propSize);
  	if ((err == noErr) && (propSize == sizeof (fBusNumber))) {
		ThrowIfOSErr_AC (RegistryPropertyGet (regEntry, kBusIDPropName, &fBusNumber, &propSize));
	} else {
		fBusNumber = 0;
	}
			
	#if qLogging
		gLogFile << "OpenFirmwareName: ";
		gLogFile.WriteCharBytes ((char *) &fOpenFirmwareName[1], fOpenFirmwareName[0]);
		gLogFile << endl_AC;
		gLogFile << "ShortOpenFirmwareName: ";
		gLogFile.WriteCharBytes ((char *) &fShortOpenFirmwareName[1], fShortOpenFirmwareName[0]);
		gLogFile << endl_AC;
	#endif
	
}

ATABus*
ATABus::BusWithNumber (int number)
{
	ATABus *retVal = NULL;
	for (ATABusIterator iter (&gATABusList); iter.Current(); iter.Next()) {
		if (iter->fBusNumber == number) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}

ATABus* 
ATABus::BusWithRegEntryID (RegEntryID *otherRegEntry)
{
	ATABus *retVal = NULL;
	for (ATABusIterator iter (&gATABusList); iter.Current(); iter.Next()) {
		if (RegistryEntryIDCompare (&iter->fRegEntryID, otherRegEntry)) {
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
			gLogFile << "Creating new ATABus with RegEntryID" << endl_AC;
		#endif
		retVal = new ATABus (otherRegEntry);
		gATABusList.InsertLast (retVal);
	}
	return retVal;
}

