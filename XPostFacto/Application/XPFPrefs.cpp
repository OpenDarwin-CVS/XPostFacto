/*

Copyright (c) 2002
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

// The idea is that this class is the "model-controller". It keeps track of the 
// data model and the user's settings.

#include "MountedVolume.h"
#include "XPFPrefs.h"
#include "XPFLog.h"
#include "XPFNameRegistry.h"
#include "OFAliases.h"
#include "XPostFacto.h"
#include "PCI.h"
#include "NVRAM.h"

#include <stdio.h>

#define kDeviceTypeProperty "device_type"

XPFPrefs::XPFPrefs () 
{
	fSetupL2Cache = false;
	fL2CRValue = 0;
	fAutoBoot = true;
	fBootInSingleUserMode = false;
	fBootInVerboseMode = false;
	fReinstallBootX = false;
	fReinstallExtensions = false;
	fInputDeviceIndex = 0;
	fOutputDeviceIndex = 0;
	fThrottle = 8;
	fForceShortStrings = false;
	
	fNextInputDevice = cFirstInputDevice;
	fNextOutputDevice = cFirstOutputDevice;
	fNextHelperVolume = cFirstHelperDisk;

	fBootDisk = NULL;
	fInstallDisk = NULL;
	fCachedCreationDate = 0;
	fHelperCreationDate = 0;
	
	fDirty = false;
	
	fDebug.breakpoint = false;
	fDebug.printf = false;
	fDebug.nmi = false;
	fDebug.kprintf = false;
	fDebug.ddb = false;
	fDebug.syslog = false;
	fDebug.arp = false;
	fDebug.oldgdb = false;
	fDebug.panicText = false;
}

void
XPFPrefs::getPrefsFromFile ()
{
	static bool doneOnce = false;
	if (doneOnce) return;
	doneOnce = true;
	
	fPrefs = new CFile_AC ('pref', '????', true);
	SInt16 prefsVRefNum;
	SInt32 prefsDirID;
	try {
		ThrowIfOSErr_AC (FindFolder (kOnSystemDisk, kPreferencesFolderType, true, &prefsVRefNum, &prefsDirID));
		ThrowIfOSErr_AC (fPrefs->SpecifyWithTrio (prefsVRefNum, prefsDirID, "\p:Unsupported UtilityX Prefs"));
			
		if (fPrefs->GetFileSpec().Exists()) fPrefs->RenameFile ("XPostFacto Prefs");
			
		ThrowIfOSErr_AC (fPrefs->SpecifyWithTrio (prefsVRefNum, prefsDirID, "\p:XPostFacto Prefs"));
			
		if (!fPrefs->GetFileSpec().Exists()) ThrowIfOSErr_AC (fPrefs->CreateCFile ());
			
		fPrefs->SetPermissions(fsRdWrPerm, fsRdWrPerm);
		ThrowIfOSErr_AC (fPrefs->OpenFile ());
		
		long size = sizeof (fCachedCreationDate);
		OSErr err = fPrefs->ReadData (&fCachedCreationDate, size);
		if (err != noErr) fCachedCreationDate = 0;
					
		size = sizeof (fBootInSingleUserMode);
		err = fPrefs->ReadData (&fBootInSingleUserMode, size);
		if (err != noErr) fBootInSingleUserMode = false;
		err = fPrefs->ReadData (&fBootInVerboseMode, size);
		if (err != noErr) fBootInVerboseMode = false;
		err = fPrefs->ReadData (&fAutoBoot, size);
		if (err != noErr) fAutoBoot = true;
		
		bool setupCache;
		err = fPrefs->ReadData (&setupCache, size);
/*		if (err == noErr) {
			setSetupL2Cache (setupCache);
		} else {
			setSetupL2Cache (false);
		}
*/			
		CStr255_AC device;
		size = 1;
		err = fPrefs->ReadData (&device[0], size);
		if ((err == noErr) && device.Length ()) {
			size = device.Length ();
			fPrefs->ReadData (&device[1], size);
			for (unsigned x = 1; x <= fInputDevices.GetSize (); x++) {
				if (device == fInputDevices.At (x)) {
					fInputDeviceIndex = x;
					break;
				}
			} 
		}
		size = 1;
		err = fPrefs->ReadData (&device[0], size);
		if ((err == noErr) && device.Length ()) {
			size = device.Length ();
			fPrefs->ReadData (&device[1], size);
			for (unsigned x = 1; x <= fOutputDevices.GetSize (); x++) {
				if (device == fOutputDevices.At (x)) {
					fOutputDeviceIndex = x;
					break;
				}
			} 
		}
		
		size = sizeof (fThrottle);
		err = fPrefs->ReadData (&fThrottle, size);
		if (err != noErr) fThrottle = 8;
		
		size = sizeof (fDebug);
		err = fPrefs->ReadData (&fDebug, size);
		if (err != noErr) {
			fDebug.breakpoint = false;
			fDebug.printf = false;
			fDebug.nmi = false;
			fDebug.kprintf = false;
			fDebug.ddb = false;
			fDebug.syslog = false;
			fDebug.arp = false;
			fDebug.oldgdb = false;
		}
		
		size = sizeof (fHelperCreationDate);
		err = fPrefs->ReadData (&fHelperCreationDate, size);
		if (err != noErr) fHelperCreationDate = 0;

	}
	catch (...) {
	}

	fBootDisk = MountedVolume::WithCreationDate (fCachedCreationDate);
	if (fBootDisk == NULL) fBootDisk = MountedVolume::GetVolumeList()->First ();
	
	fHelperDisk = MountedVolume::WithCreationDate (fHelperCreationDate);
	if (fHelperDisk == NULL) {
		for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current(); iter.Next()) {
			if (iter->getIsOnBootableDevice () && !iter->getRequiresBootHelper ()) {
				if (fHelperDisk) {
					if (iter->getFreeBytes () > fHelperDisk->getFreeBytes ()) {
						fHelperDisk = iter.Current ();
					} 
				} else {
					fHelperDisk = iter.Current ();
				}
			}
		}
	}

	fInstallDisk = MountedVolume::GetVolumeList()->First ();
}

void
XPFPrefs::writePrefsToFile ()
{
	fPrefs->SetPosition (0);
	unsigned int creationDate = fBootDisk->getCreationDate ();
	long size = sizeof (creationDate);
	fPrefs->WriteData (&creationDate, size);
	size = sizeof (fBootInSingleUserMode);
	fPrefs->WriteData (&fBootInSingleUserMode, size);
	fPrefs->WriteData (&fBootInVerboseMode, size);
	fPrefs->WriteData (&fAutoBoot, size);
	fPrefs->WriteData (&fSetupL2Cache, size);
			
	bool save = fUseShortStrings;
	fUseShortStrings = false;
	CStr255_AC device;
	device = getInputDevice ();
	size = device.Length () + 1;
	fPrefs->WriteData (device, size);
	device = getOutputDevice ();
	size = device.Length () + 1;
	fPrefs->WriteData (device, size);
	fUseShortStrings = save;
	
	size = sizeof (fThrottle);
	fPrefs->WriteData (&fThrottle, size);
	
	size = sizeof (fDebug);
	fPrefs->WriteData (&fDebug, size);
	
	size = sizeof (creationDate);
	creationDate = fHelperDisk->getCreationDate ();
	fPrefs->WriteData (&creationDate, size);
}

XPFPrefs::~XPFPrefs ()
{
	if (fPrefs) {
		if (fDirty) writePrefsToFile ();
		fPrefs->CloseFile ();
		fPrefs->FlushVolume ();

		delete fPrefs;
	}
	
	for (int x = 1; x <= fInputDevices.GetSize (); x++) {
		DisposePtr (fInputDevices.At (x));
	} 

	for (int x = 1; x <= fOutputDevices.GetSize (); x++) {
		DisposePtr (fOutputDevices.At (x));
	}

	for (int x = 1; x <= fShortInputDevices.GetSize (); x++) {
		DisposePtr (fShortInputDevices.At (x));
	} 

	for (int x = 1; x <= fShortOutputDevices.GetSize (); x++) {
		DisposePtr (fShortOutputDevices.At (x));
	}
}

void
XPFPrefs::checkStringLength ()
{
	if (fForceShortStrings) {
		fUseShortStrings = true;
		fUseShortStringsForInstall = true;
	} else {
		fUseShortStrings = false;
		unsigned nvramPatchLength = strlen (NVRAMVariables::GetVariables ()->getNVRAMRC ());
		unsigned len = 	getBootCommand ().Length () + 
						getBootFile ().Length () +
						getBootDevice ().Length () +
						getInputDevice ().Length () +
						getOutputDevice ().Length () +
						nvramPatchLength;
		fUseShortStrings = (len >= 1948);
				
		len =  	getBootCommandForInstall ().Length () + 
				getBootFileForInstall ().Length () +
				getBootDeviceForInstall ().Length () +
				getInputDevice ().Length () +
				getOutputDevice ().Length () +
				nvramPatchLength;
		fUseShortStringsForInstall = (len >= 1948);
	}
}

void 
XPFPrefs::Changed(ChangeID_AC theChange, void* changeData)
{
	fDirty = true;
	
	checkStringLength ();
	
	MDependable_AC::Changed (theChange, changeData);
}

void
XPFPrefs::Initialize ()
{
	#if qLogging
		gLogFile << "Reinitializing ..." << endl_AC;
	#endif
	
	initializeInputAndOutputDevices ();
	
	MountedVolume::Initialize ();
	
	initializeHelperMenu ();
	
	getPrefsFromFile ();

	unsigned int bootCreationDate = fBootDisk->getCreationDate ();
	unsigned int installCreationDate = fInstallDisk->getCreationDate ();
	
	setBootDisk (MountedVolume::WithCreationDate (bootCreationDate));
	setInstallDisk (MountedVolume::WithCreationDate (installCreationDate));
	if (fBootDisk == NULL) setBootDisk (MountedVolume::GetVolumeList()->First ());
	if (fInstallDisk == NULL) setInstallDisk (MountedVolume::GetVolumeList()->First ());
	
	UInt32 version = 0;	
	Gestalt (gestaltSystemVersion, (SInt32 *) &version);

	if (version < 0x01000) Changed (cSetBootDisk, NULL);
}

void 
XPFPrefs::addInputOutputDevice (RegEntryID *entry, TemplateList_AC <char> *list)
{
	char alias[256];
	char shortAlias[256];
	char label[128];
	char displayType[128];

	OFAliases::AliasFor (entry, alias, shortAlias);
	if (list == &fOutputDevices) {
		if (!strcmp (alias, "kbd")) return;
		if (!strcmp (alias, "/offscreen-display")) return;
	}
			
#if qLogging
	gLogFile << "IO Device: " << alias << endl_AC;
	gLogFile << "IO Short Device: " << shortAlias << endl_AC;
#endif
	
	RegPropertyValueSize propSize;
	OSErr err = RegistryPropertyGetSize (entry, "AAPL,connector", &propSize);
	if (err == noErr) {
		RegistryPropertyGet (entry, "AAPL,connector", label, &propSize);
		label[propSize] = '\0';
	} else {
		err = RegistryPropertyGetSize (entry, "name", &propSize);
		if (err == noErr) {
			RegistryPropertyGet (entry, "name", label, &propSize);
			label[propSize] = '\0';
		} else {
			strcpy (label, alias);
		}
	}
	
	if (!strcmp (label, "infrared")) {
		return;
	}
	
	err = RegistryPropertyGetSize (entry, "display-type", &propSize);
	if (err == noErr) {
		RegistryPropertyGet (entry, "display-type", displayType, &propSize);
		displayType[propSize] = '\0';
		if (!strcmp (displayType, "NONE")) return;
		strcat (label, " (");
		strcat (label, displayType);
		strcat (label, ")");
	}

	char *temp = NewPtr (strlen (alias) + 1);
	strcpy (temp, alias);
	list->InsertLast (temp);
	
	temp = NewPtr (strlen (shortAlias) + 1);
	strcpy (temp, shortAlias);
	
	if (list == &fInputDevices) {
		fShortInputDevices.InsertLast (temp);
		TMenuBarManager::fgMenuBarManager->AddMenuItem (label, mInputDevice, 999, fNextInputDevice++);
	} else {
		fShortOutputDevices.InsertLast (temp);
		TMenuBarManager::fgMenuBarManager->AddMenuItem (label, mOutputDevice, 999, fNextOutputDevice++);
	}
}

void
XPFPrefs::initializeHelperMenu ()
{
	static bool doneOnce = false;
	if (doneOnce) return;
	doneOnce = true;

	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getIsOnBootableDevice () && !iter->getRequiresBootHelper ()) {
			TMenuBarManager::fgMenuBarManager->AddMenuItem (iter->getVolumeName (), mHelper, 999, fNextHelperVolume++);
		}		
	}
}

unsigned int 
XPFPrefs::getHelperVolumeIndex ()
{
	unsigned index = 0;
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getIsOnBootableDevice () && !iter->getRequiresBootHelper ()) {
			index++;
			if (iter.Current () == fHelperDisk) return index;
		}		
	}
	return 0;
}

void
XPFPrefs::setHelperVolumeIndex (unsigned index)
{
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getIsOnBootableDevice () && !iter->getRequiresBootHelper ()) {
			index--;
			if (index == 0) setHelperDisk (iter.Current ());
		}		
	}
}

void
XPFPrefs::initializeInputAndOutputDevices ()
{
	static bool doneOnce = false;
	if (doneOnce) return;
	doneOnce = true;

	if ((Ptr) RegistryEntryIterate == (Ptr) kUnresolvedCFragSymbolAddress) return;

	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	try {
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
					if (!strcmp (deviceType, "serial")) {
						addInputOutputDevice (&entry, &fInputDevices);
						addInputOutputDevice (&entry, &fOutputDevices);
					} else if (!strcmp (deviceType, "display")) {
						addInputOutputDevice (&entry, &fOutputDevices);
					} else if (!strcmp (deviceType, "keyboard")) {
						addInputOutputDevice (&entry, &fInputDevices);
					}
					DisposePtr (deviceType);				
				}
		        RegistryEntryIDDispose (&entry);
	        } else {
	        	break;
	        }
	        iterOp = kRegIterContinue;
	    }
	} 
	catch (...) {
	}
	RegistryEntryIterateDispose (&cookie);
}


CStr255_AC 
XPFPrefs::getInputDevice ()
{
	CStr255_AC result;
	if (fInputDeviceIndex) {
		if (fUseShortStrings) {
			result.CopyFrom (fShortInputDevices.At (fInputDeviceIndex));
		} else {
			result.CopyFrom (fInputDevices.At (fInputDeviceIndex));
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getOutputDevice ()
{
	CStr255_AC result;
	if (fOutputDeviceIndex) {
		if (fUseShortStrings) {
			result.CopyFrom (fShortOutputDevices.At (fOutputDeviceIndex));
		} else {
			result.CopyFrom (fOutputDevices.At (fOutputDeviceIndex));		
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getInputDeviceForInstall ()
{
	CStr255_AC result;
	if (fInputDeviceIndex) {
		if (fUseShortStringsForInstall) {
			result.CopyFrom (fShortInputDevices.At (fInputDeviceIndex));
		} else {
			result.CopyFrom (fInputDevices.At (fInputDeviceIndex));
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getOutputDeviceForInstall ()
{
	CStr255_AC result;
	if (fOutputDeviceIndex) {
		if (fUseShortStringsForInstall) {
			result.CopyFrom (fShortOutputDevices.At (fOutputDeviceIndex));
		} else {
			result.CopyFrom (fOutputDevices.At (fOutputDeviceIndex));		
		}
	}
	return result;
}

void 
XPFPrefs::setBootDisk (MountedVolume *theDisk)
{
	if (fBootDisk != theDisk) {
		fBootDisk = theDisk;
		fCachedCreationDate = theDisk->getCreationDate ();
		Changed (cSetBootDisk, NULL);
	}
}

void 
XPFPrefs::setInstallDisk (MountedVolume *theDisk)
{
	if (fInstallDisk != theDisk) {
		fInstallDisk = theDisk;
		Changed (cSetInstallDisk, NULL);
	}
}

void
XPFPrefs::setHelperDisk (MountedVolume *theDisk)
{
	if (fHelperDisk != theDisk) {
		fHelperDisk = theDisk;
		Changed (cSetHelperDisk, NULL);
	}
}

void 
XPFPrefs::setBootInVerboseMode (bool val)
{
	if (fBootInVerboseMode != val) {
		fBootInVerboseMode = val;
		Changed (cToggleVerboseMode, NULL);
	}
}


void
XPFPrefs::setReinstallBootX (bool val)
{
	if (fReinstallBootX != val) {
		fReinstallBootX = val;
		Changed (cReinstallBootX, NULL);
	}
}

void
XPFPrefs::setReinstallExtensions (bool val)
{
	if (fReinstallExtensions != val) {
		fReinstallExtensions = val;
		Changed (cReinstallExtensions, NULL);
	}
}

void 
XPFPrefs::setBootInSingleUserMode (bool val)
{
	if (fBootInSingleUserMode != val) {
		fBootInSingleUserMode = val;
		Changed (cToggleSingleUserMode, NULL);
	}
}

void
XPFPrefs::setSetupL2Cache (bool val)
{
#pragma unused (val)
/*	if (!fHasL2Cache) return;
	fPrefs->setSetupL2Cache (val);
	if (fSetupL2Cache && !fPrefs->getL2CRValue) fPrefs->setL2CRValue (getCurrentL2CRValue ());
	if (fMainWindow) fMainWindow->UpdateUI ();
*/
}

void
XPFPrefs::setAutoBoot (bool val)
{
	if (fAutoBoot != val) {
		fAutoBoot = val;
		Changed (cToggleAutoBoot, NULL);
	}
}

void 
XPFPrefs::setOutputDeviceIndex (unsigned index)
{
	if (fOutputDeviceIndex != index) {
		fOutputDeviceIndex = index;
		Changed (cFirstOutputDevice, NULL);
	}
}

void 
XPFPrefs::setInputDeviceIndex (unsigned index)
{
	if (fInputDeviceIndex != index) {
		fInputDeviceIndex = index;
		Changed (cFirstInputDevice, NULL);
	}
}

void 
XPFPrefs::setThrottle (unsigned throttle)
{
	if (fThrottle != throttle) {
		fThrottle = throttle;
		Changed (cThrottleBase, NULL);
	}
}

CStr255_AC
XPFPrefs::getBootDevice ()
{
	MountedVolume *bootDevice = getBootDisk ();
	if (bootDevice->getRequiresBootHelper ()) {
		bootDevice = getHelperDisk ();		
	}
	
	if (fUseShortStrings) {
		return bootDevice->getShortOpenFirmwareName();
	} else {
		return bootDevice->getOpenFirmwareName();
	}
}

CStr255_AC
XPFPrefs::getBootDeviceForInstall ()
{
	MountedVolume *bootDevice = getInstallDisk ();
	if (bootDevice->getRequiresBootHelper ()) {
		bootDevice = getHelperDisk ();		
	}
	
	if (fUseShortStringsForInstall) {
		return bootDevice->getShortOpenFirmwareName ();
	} else {
		return bootDevice->getOpenFirmwareName ();
	}
}

CStr255_AC 
XPFPrefs::getBootCommandBase ()
{
	CStr255_AC bootCommand ("0 bootr");
	if (fBootInVerboseMode || !getBootDisk ()->getHasFinder ()) bootCommand += (" -v");
	if (fBootInSingleUserMode) bootCommand += (" -s");
	
	unsigned debug = 0;
	if (fDebug.breakpoint)	debug |= 1 << 0;
	if (fDebug.printf) 		debug |= 1 << 1;
	if (fDebug.nmi)			debug |= 1 << 2;
	if (fDebug.kprintf)		debug |= 1 << 3;
	if (fDebug.ddb)			debug |= 1 << 4;
	if (fDebug.syslog) 		debug |= 1 << 5;
	if (fDebug.arp)			debug |= 1 << 6;
	if (fDebug.oldgdb)		debug |= 1 << 7;
	if (fDebug.panicText)	debug |= 1 << 8;
	
	if (debug) {
		char debugValue [32];
		sprintf (debugValue, " debug=0x%X", debug);
		bootCommand += debugValue;
	}
			
#if 0
	if (fSetupL2Cache) {
		char str [128];
		sprintf (str, " L2CR=0x%X", fL2CRValue);
		bootCommand += str;
	}
#endif
	return bootCommand;

}

CStr255_AC 
XPFPrefs::getBootCommand ()
{
	CStr255_AC bootCommand = getBootCommandBase ();
	
	if (getBootDisk ()->getRequiresBootHelper ()) {
		bootCommand += " rd=*";
		if (fUseShortStrings) {
			bootCommand += fBootDisk->getShortOpenFirmwareName ();
		} else {
			bootCommand += fBootDisk->getOpenFirmwareName ();
		}
	}
		
	return bootCommand;
}

CStr255_AC
XPFPrefs::getBootCommandForInstall ()
{
	CStr255_AC bootCommand = getBootCommandBase ();
	
	bootCommand += " rd=*";
	if (fUseShortStringsForInstall) {
		bootCommand += fBootDisk->getShortOpenFirmwareName ();
	} else {
		bootCommand += fBootDisk->getOpenFirmwareName();	
	}

	return bootCommand;
}

CStr255_AC
XPFPrefs::getBootFileForInstall ()
{
	CStr255_AC bootFile ("-i");
	return bootFile;
}

CStr255_AC
XPFPrefs::getBootFile ()
{
	if (getBootDisk ()->getRequiresBootHelper ()) {
		return CStr255_AC ("-h");
	} else {
		return CStr255_AC ("");
	}
}

void
XPFPrefs::setDebugBreakpoint (bool val)
{
	if (fDebug.breakpoint != val) {
		fDebug.breakpoint = val;
		Changed (cToggleDebugBreakpoint, NULL);
	}
}

void
XPFPrefs::setDebugPrintf (bool val)
{
	if (fDebug.printf != val) {
		fDebug.printf = val;
		Changed (cToggleDebugPrint, NULL);
	}
}

void
XPFPrefs::setDebugNMI (bool val)
{
	if (fDebug.nmi != val) {
		fDebug.nmi = val;
		Changed (cToggleDebugNMI, NULL);
	}
}

void
XPFPrefs::setDebugKprintf (bool val)
{
	if (fDebug.kprintf != val) {
		fDebug.kprintf = val;
		Changed (cToggleDebugkprintf, NULL);
	}
}

void
XPFPrefs::setDebugDDB (bool val)
{
	if (fDebug.ddb != val) {
		fDebug.ddb = val;
		Changed (cToggleDebugUseDDB, NULL);
	}
}

void
XPFPrefs::setDebugSyslog (bool val)
{
	if (fDebug.syslog != val) {
		fDebug.syslog = val;
		Changed (cToggleDebugSystemLog, NULL);
	}
}

void
XPFPrefs::setDebugARP (bool val)
{
	if (fDebug.arp != val) {
		fDebug.arp = val;
		Changed (cToggleDebugARP, NULL);
	}
}

void
XPFPrefs::setDebugOldGDB (bool val)
{
	if (fDebug.oldgdb != val) {
		fDebug.oldgdb = val;
		Changed (cToggleDebugOldGDB, NULL);
	}
}

void
XPFPrefs::setDebugPanicText (bool val)
{
	if (fDebug.panicText != val) {
		fDebug.panicText = val;
		Changed (cToggleDebugPanicText, NULL);
	}
}


