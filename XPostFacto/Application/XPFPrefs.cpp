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
	
	fNextInputDevice = cFirstInputDevice;
	fNextOutputDevice = cFirstOutputDevice;

	fBootDisk = NULL;
	fInstallDisk = NULL;
	fCachedCreationDate = 0;
	
	fDirty = false;
	
	fDebug.breakpoint = false;
	fDebug.printf = false;
	fDebug.nmi = false;
	fDebug.kprintf = false;
	fDebug.ddb = false;
	fDebug.syslog = false;
	fDebug.arp = false;
	fDebug.oldgdb = false;
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

	}
	catch (...) {
	}

	fBootDisk = MountedVolume::WithCreationDate (fCachedCreationDate);
	if (fBootDisk == NULL) fBootDisk = MountedVolume::GetVolumeList()->First ();

	fInstallDisk = MountedVolume::GetVolumeList()->First ();
}

void
XPFPrefs::writePrefsToFile ()
{
	fPrefs->SetPosition (0);
	const unsigned int creationDate = fBootDisk->getCreationDate ();
	long size = sizeof (creationDate);
	fPrefs->WriteData (&creationDate, size);
	size = sizeof (fBootInSingleUserMode);
	fPrefs->WriteData (&fBootInSingleUserMode, size);
	fPrefs->WriteData (&fBootInVerboseMode, size);
	fPrefs->WriteData (&fAutoBoot, size);
	fPrefs->WriteData (&fSetupL2Cache, size);
			
	CStr255_AC device;
	device = getInputDevice ();
	size = device.Length () + 1;
	fPrefs->WriteData (device, size);
	device = getOutputDevice ();
	size = device.Length () + 1;
	fPrefs->WriteData (device, size);
	
	size = sizeof (fThrottle);
	fPrefs->WriteData (&fThrottle, size);
	
	size = sizeof (fDebug);
	fPrefs->WriteData (&fDebug, size);
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
}

void 
XPFPrefs::Changed(ChangeID_AC theChange, void* changeData)
{
	fDirty = true;
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
	
	getPrefsFromFile ();

	unsigned int bootCreationDate = fBootDisk->getCreationDate ();
	unsigned int installCreationDate = fInstallDisk->getCreationDate ();
	
	setBootDisk (MountedVolume::WithCreationDate (bootCreationDate));
	setInstallDisk (MountedVolume::WithCreationDate (installCreationDate));
	if (fBootDisk == NULL) setBootDisk (MountedVolume::GetVolumeList()->First ());
	if (fInstallDisk == NULL) setInstallDisk (MountedVolume::GetVolumeList()->First ());
}


void 
XPFPrefs::addInputOutputDevice (RegEntryID *entry, TemplateList_AC <char> *list)
{
	char alias[256];
	OFAliases::AliasFor (entry, alias);
	if (list == &fOutputDevices) {
		if (!strcmp (alias, "kbd")) return;
		if (!strcmp (alias, "/offscreen-display")) return;
	}
	
	char *label = NULL;
	RegPropertyValueSize propSize;
	OSErr err = RegistryPropertyGetSize (entry, "AAPL,connector", &propSize);
	if (err == noErr) {
		label = NewPtr (propSize + 1);
		RegistryPropertyGet (entry, "AAPL,connector", label, &propSize);
		label[propSize] = '\0';
	} else {
		err = RegistryPropertyGetSize (entry, "name", &propSize);
		if (err == noErr) {
			label = NewPtr (propSize + 1);
			RegistryPropertyGet (entry, "name", label, &propSize);
			label[propSize] = '\0';
		} else {
			label = NewPtr (strlen (alias) + 1);
			strcpy (label, alias);
		}
	}
	if (strcmp (label, "infrared")) {
		char *shortAlias = NewPtr (strlen (alias) + 1);
		strcpy (shortAlias, alias);
		if (list == &fInputDevices) {
			fInputDevices.InsertLast (shortAlias);
			TMenuBarManager::fgMenuBarManager->AddMenuItem (label, mInputDevice, 999, fNextInputDevice++);
		} else {
			fOutputDevices.InsertLast (shortAlias);
			TMenuBarManager::fgMenuBarManager->AddMenuItem (label, mOutputDevice, 999, fNextOutputDevice++);
		}
	}
	DisposePtr (label);
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
	if (fInputDeviceIndex) result.CopyFrom (fInputDevices.At (fInputDeviceIndex));
	return result;
}

CStr255_AC
XPFPrefs::getOutputDevice ()
{
	CStr255_AC result;
	if (fOutputDeviceIndex) result.CopyFrom (fOutputDevices.At (fOutputDeviceIndex));
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
	return getBootDisk()->getOpenFirmwareName();
}

CStr255_AC
XPFPrefs::getBootDeviceForInstall ()
{
	return fInstallDisk->getOpenFirmwareName ();
}

CStr255_AC
XPFPrefs::getBootCommandForInstall ()
{
	CStr255_AC bootCommand = getBootCommand ();
	bootCommand += " rd=*";
	bootCommand += fBootDisk->getOpenFirmwareName();	
	return bootCommand;
}

CStr255_AC
XPFPrefs::getBootFileForInstall ()
{
	CStr255_AC bootFile (",\\tmp\\mach_kernel");
	return bootFile;
}

CStr255_AC
XPFPrefs::getBootFile ()
{
	return CStr255_AC ("");
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

CStr255_AC 
XPFPrefs::getBootCommand ()
{
	CStr255_AC bootCommand ("0 bootr");
	if (fBootInVerboseMode) bootCommand += (" -v");
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
	
	if (debug) {
		char debugValue [12];
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


