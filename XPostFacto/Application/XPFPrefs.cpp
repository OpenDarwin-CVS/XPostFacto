/*

Copyright (c) 2002, 2003
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

#include "MountedVolume.h"
#include "XPFPrefs.h"
#include "XPFLog.h"
#include "XPostFacto.h"
#include "NVRAM.h"
#include "XPFIODevice.h"
#include "XPFStrings.h"
#include "XPFApplication.h"
#include "XPFUpdateCommands.h"
#include "XPFInstallCommand.h"
#include "XPFRestartCommand.h"

#ifndef __MACH__
	#include <PCI.h>
#endif

#include <stdio.h>

#define Inherited TFileBasedDocument

UInt32 kPrefsSignature = 'usuX';
UInt32 kPrefsVersion = 1;

const UInt32 kDebugBreakpoint	= 1 << 0;
const UInt32 kDebugPrintf		= 1 << 1;
const UInt32 kDebugNMI			= 1 << 2;
const UInt32 kDebugKprintf		= 1 << 3;
const UInt32 kDebugDDB			= 1 << 4;
const UInt32 kDebugSyslog		= 1 << 5;
const UInt32 kDebugARP			= 1 << 6;
const UInt32 kDebugOldGDB		= 1 << 7;
const UInt32 kDebugPanicText	= 1 << 8;

#define kDeviceTypeProperty "device_type"

XPFPrefs::XPFPrefs (TFile* itsFile)
	: TFileBasedDocument (itsFile),
		fEnableCacheEarly (false),
		fAutoBoot (true),
		fBootInSingleUserMode (false),
		fBootInVerboseMode (false),
		fThrottle (0),
		fTargetDisk (NULL),
		fInstallCD (NULL),
		fInputDevice (NULL),
		fOutputDevice (NULL),
		fDebug (0)
{
	fAskOnClose = false;
}

XPFPrefs::~XPFPrefs ()
{

}

#ifdef __MACH__

void
XPFPrefs::getPrefsFromNVRAM ()
{
	static bool done = false;
	if (done) return;
	done = true;
		
	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
	
	setAutoBoot (nvram->getBooleanValue ("auto-boot?"));
	
	char *bootCommand = nvram->getStringValue ("boot-command");
	char *bootDevice = nvram->getStringValue ("boot-device");
	char *bootFile = nvram->getStringValue ("boot-file");
	char *inputDevice = nvram->getStringValue ("input-device");
	char *outputDevice = nvram->getStringValue ("output-device");
	char *nvramrc = nvram->getStringValue ("nvramrc");
	
	setBootInSingleUserMode ((strstr (bootCommand, " -s")) != 0);
	setBootInVerboseMode ((strstr (bootCommand, " -v")) != 0);
	
	char *debugString = strstr (bootCommand, "debug=");
	if (debugString) {
		debugString += strlen ("debug=");
		fDebug = strtoul (debugString, NULL, 0);
	}
	
	MountedVolume *bootDisk = MountedVolume::WithOpenFirmwarePath (bootDevice);
	MountedVolume *rootDisk = bootDisk;
	
	char *rdString = strstr (bootCommand, "rd=*");
	if (rdString) {
		char str[256];
		strcpy (str, rdString + strlen ("rd=*"));
		char *pos = strchr (str, ' ');
		if (pos) *pos = 0;
		
		MountedVolume *rootDisk = MountedVolume::WithOpenFirmwarePath (rdString);
		if (!rootDisk) rootDisk = bootDisk;
	}	
	
	if (rootDisk && (rootDisk->getBootStatus () == kStatusOK)) {
		setTargetDisk (rootDisk);
	}
	
	// input device
	setInputDevice (inputDevice);
	
	// output device
	setOutputDevice (outputDevice);
	
	// throttle
	char *throttle = strstr (nvramrc, "$I");
	if (throttle) {
		throttle = strstr (throttle + 2, "$I");
		if (throttle) {
			do {throttle--;} while (*throttle == ' ');
			do {throttle--;} while (*throttle != ' ');
			setThrottle (strtoul (throttle, NULL, 0));
		}
	}
}

#endif 		// __MACH__

void
XPFPrefs::DoInitialState ()
{
	Inherited::DoInitialState ();

	MountedVolume::Initialize ();
	fTargetDisk = MountedVolume::GetDefaultRootDisk ();
	fInstallCD = MountedVolume::GetDefaultInstallerDisk ();
	
	checkStringLength ();
}

void
XPFPrefs::RegainControl ()
{
	Inherited::RegainControl ();
	MountedVolume::Initialize ();
}

void
XPFPrefs::DoRead (TFile* aFile, bool forPrinting)
{
	#pragma unused (forPrinting)
	
	CFileStream_AC fileStream (aFile);

	UInt32 sig, prefsVersion;
	CStr255_AC device;
	FSVolumeInfo volInfo;
	
	DoInitialState ();
			
	try {	
		fileStream 	>> sig;
		if (sig != kPrefsSignature) return;
		
		fileStream >> prefsVersion;
		if (prefsVersion != 1) return;
		
		fileStream 	>> fBootInSingleUserMode
					>> fBootInVerboseMode
					>> fAutoBoot
					>> fThrottle
					>> fDebug
					>> fEnableCacheEarly;
		
		device = fileStream.ReadString (255);
		setInputDevice (CChar255_AC (device), false);
		
		device = fileStream.ReadString (255);
		setOutputDevice (CChar255_AC (device), false);
		
		fileStream.ReadBytes (&volInfo, sizeof (volInfo));
		// Don't use the pref if we can't find the disk
		MountedVolume *rootDisk = MountedVolume::WithInfo (&volInfo);
		if (rootDisk) fTargetDisk = rootDisk;
	}
	
	catch (...) {
	}
	
	checkStringLength ();
}

void
XPFPrefs::DoWrite (TFile* aFile, bool makingCopy)
{
	#pragma unused (makingCopy)
	
	CFileStream_AC fileStream (aFile);

	fileStream	<< kPrefsSignature
				<< kPrefsVersion
				<< fBootInSingleUserMode
				<< fBootInVerboseMode
				<< fAutoBoot
				<< fThrottle
				<< fDebug
				<< fEnableCacheEarly;
			
	bool save = fUseShortStrings;
	fUseShortStrings = false;
	CStr255_AC device;
	
	device = getInputDevice ();
	fileStream.WriteString (device);
		
	device = getOutputDevice ();
	fileStream.WriteString (device);
	
	fUseShortStrings = save;
	
	FSVolumeInfo info = fTargetDisk->getVolumeInfo ();
	fileStream.WriteBytes (&info, sizeof (info));
}

void 
XPFPrefs::DoMakeViews (bool forPrinting)
{
	#pragma unused (forPrinting)
	
	try {
		TViewServer::fgViewServer->NewTemplateWindow (kGridWindow, this);
		TViewServer::fgViewServer->NewTemplateWindow (kAdditionalSettingsWindow, this);
	}
	
	catch (...) {
		((XPFApplication *) gApplication)->CloseSplashWindow ();
		throw;
	}
	
	((XPFApplication *) gApplication)->CloseSplashWindow ();
}


void 
XPFPrefs::DoSetupMenus ()
{
	Inherited::DoSetupMenus ();
	
	Enable (cInstallBootX, true);
	Enable (cInstallExtensions, true);
	Enable (cInstallStartupItem, true);
}

void 
XPFPrefs::DoMenuCommand (CommandNumber aCommandNumber)
{
	switch (aCommandNumber) 
	{				
		case cInstallBootX:
			PostCommand (TH_new XPFInstallBootXCommand (this));
			break;
			
		case cInstallExtensions:
			PostCommand (TH_new XPFInstallExtensionsCommand (this));
			break;
		
		case cInstallStartupItem:
			PostCommand (TH_new XPFInstallStartupCommand (this));
			break;
				
		default:
			Inherited::DoMenuCommand (aCommandNumber);
			break;
	}
}

void
XPFPrefs::checkStringLength ()
{
	unsigned nvramPatchLength = strlen (XPFNVRAMSettings::GetSettings ()->getStringValue ("nvramrc"));
	unsigned len;
	bool save;
	
	save = fUseShortStrings;
	fUseShortStrings = false;
	len = 	getBootCommand ().Length () + 
			getBootFile ().Length () +
			getBootDevice ().Length () +
			getInputDevice ().Length () +
			getOutputDevice ().Length () +
			nvramPatchLength;
	fUseShortStrings = save;
	setUseShortStrings (len >= 1948);

	save = fUseShortStringsForInstall;
	fUseShortStringsForInstall = false;				
	len =  	getBootCommandForInstall ().Length () + 
			getBootFileForInstall ().Length () +
			getBootDeviceForInstall ().Length () +
			getInputDevice ().Length () +
			getOutputDevice ().Length () +
			nvramPatchLength;
	fUseShortStringsForInstall = save;
	setUseShortStringsForInstall (len >= 1948);
}

void 
XPFPrefs::Changed(ChangeID_AC theChange, void* changeData)
{
	checkStringLength ();
	
	Inherited::Changed (theChange, changeData);
}

// Functions for getting the values we need to write to NVRAM.

CStr255_AC 
XPFPrefs::getInputDevice ()
{
	CStr255_AC result;
	if (fInputDevice) {
		if (fUseShortStrings) {
			result.CopyFrom (fInputDevice->getShortOpenFirmwareName ());
		} else {
			result.CopyFrom (fInputDevice->getOpenFirmwareName ());
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getOutputDevice ()
{
	CStr255_AC result;
	if (fOutputDevice) {
		if (fUseShortStrings) {
			result.CopyFrom (fOutputDevice->getShortOpenFirmwareName ());
		} else {
			result.CopyFrom (fOutputDevice->getOpenFirmwareName ());
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getInputDeviceForInstall ()
{
	CStr255_AC result;
	if (fInputDevice) {
		if (fUseShortStringsForInstall) {
			result.CopyFrom (fInputDevice->getShortOpenFirmwareName ());
		} else {
			result.CopyFrom (fInputDevice->getOpenFirmwareName ());
		}
	}
	return result;
}

CStr255_AC
XPFPrefs::getOutputDeviceForInstall ()
{
	CStr255_AC result;
	if (fOutputDevice) {
		if (fUseShortStringsForInstall) {
			result.CopyFrom (fOutputDevice->getShortOpenFirmwareName ());
		} else {
			result.CopyFrom (fOutputDevice->getOpenFirmwareName ());
		}
	}
	return result;
}


CStr255_AC
XPFPrefs::getBootDevice ()
{
	MountedVolume *bootDevice = getTargetDisk ();
	if (!bootDevice) return "";
	
	if (bootDevice->getHelperDisk ()) bootDevice = bootDevice->getHelperDisk ();		
	
	if (fUseShortStrings) {
		return bootDevice->getShortOpenFirmwareName();
	} else {
		return bootDevice->getOpenFirmwareName();
	}
}

CStr255_AC
XPFPrefs::getBootDeviceForInstall ()
{
	MountedVolume *bootDevice = getTargetDisk ();
	if (!bootDevice) return "";
	
	if (bootDevice->getHelperDisk ()) bootDevice = bootDevice->getHelperDisk ();		
	
	if (fUseShortStringsForInstall) {
		return bootDevice->getShortOpenFirmwareName ();
	} else {
		return bootDevice->getOpenFirmwareName ();
	}
}

CStr255_AC 
XPFPrefs::getBootCommandBase ()
{
	if (!getTargetDisk ()) return "";
	
	CStr255_AC bootCommand ("0 bootr");
	
	if (fDebug) {
		char debugValue [32];
		sprintf (debugValue, " debug=0x%X", fDebug);
		bootCommand += debugValue;
	}
			
	if (fBootInVerboseMode) bootCommand += (" -v");
	if (fBootInSingleUserMode) bootCommand += (" -s");
	
	return bootCommand;
}

CStr255_AC 
XPFPrefs::getBootCommand ()
{
	if (!getTargetDisk ()) return "";	
	CStr255_AC bootCommand = getBootCommandBase ();
	
	if (!fBootInVerboseMode && !getTargetDisk ()->getHasFinder ()) bootCommand += " -v";
	
	if (getTargetDisk ()->getHelperDisk ()) {
		bootCommand += " rd=*";
		if (fUseShortStrings) {
			bootCommand += getTargetDisk ()->getShortOpenFirmwareName ();
		} else {
			bootCommand += getTargetDisk ()->getOpenFirmwareName ();
		}
	}
		
	return bootCommand;
}

CStr255_AC
XPFPrefs::getBootCommandForInstall ()
{
	if (!getInstallCD ()) return "";
	CStr255_AC bootCommand = getBootCommandBase ();
	
	if (!fBootInVerboseMode && !getInstallCD ()->getHasFinder ()) bootCommand += " -v";
	
	bootCommand += " rd=*";
	if (fUseShortStringsForInstall) {
		bootCommand += fInstallCD->getShortOpenFirmwareName ();
	} else {
		bootCommand += fInstallCD->getOpenFirmwareName();	
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
	if (!getTargetDisk ()) return "";
	if (getTargetDisk ()->getHelperDisk ()) {
		return CStr255_AC ("-h");
	} else {
		return CStr255_AC ("");
	}
}

// Accessors

void
XPFPrefs::setInputDevice (XPFIODevice *val)
{
	if (fInputDevice != val) {
		fInputDevice = val;
		Changed (cSetInputDevice, val);
	}
}

void
XPFPrefs::setOutputDevice (XPFIODevice *val)
{
	if (fOutputDevice != val) {
		fOutputDevice = val;
		Changed (cSetOutputDevice, val);
	}
}

void 
XPFPrefs::setUseShortStrings (bool newVal)
{
	if (fUseShortStrings != newVal) {
		fUseShortStrings = newVal;
		Changed (cSetUseShortStrings, &fUseShortStrings);
	}
}

void 
XPFPrefs::setUseShortStringsForInstall (bool newVal)
{
	if (fUseShortStringsForInstall != newVal) {
		fUseShortStringsForInstall = newVal;
		Changed (cSetUseShortStringsForInstall, &fUseShortStringsForInstall);
	}
}

void 
XPFPrefs::setTargetDisk (MountedVolume *theDisk)
{
	if (fTargetDisk != theDisk) {
		fTargetDisk = theDisk;
		Changed (cSetTargetDisk, fTargetDisk);
	}
}

void 
XPFPrefs::setInstallCD (MountedVolume *theDisk)
{
	if (fInstallCD != theDisk) {
		fInstallCD = theDisk;
		Changed (cSetInstallCD, fInstallCD);
	}
}

void 
XPFPrefs::setBootInVerboseMode (bool val)
{
	if (fBootInVerboseMode != val) {
		fBootInVerboseMode = val;
		Changed (cSetVerboseMode, &fBootInVerboseMode);
	}
}

void 
XPFPrefs::setBootInSingleUserMode (bool val)
{
	if (fBootInSingleUserMode != val) {
		fBootInSingleUserMode = val;
		Changed (cSetSingleUserMode, &fBootInSingleUserMode);
	}
}

void
XPFPrefs::setAutoBoot (bool val)
{
	if (fAutoBoot != val) {
		fAutoBoot = val;
		Changed (cSetAutoBoot, &fAutoBoot);
	}
}

void
XPFPrefs::setEnableCacheEarly (bool val)
{
	if (fEnableCacheEarly != val) {
		fEnableCacheEarly = val;
		Changed (cSetEnableCacheEarly, &fEnableCacheEarly);
	}
}

void 
XPFPrefs::setThrottle (unsigned throttle)
{
	if (fThrottle != throttle) {
		fThrottle = throttle;
		Changed (cSetThrottle, &fThrottle);
	}
}

// Debug accessors. Better than repeating it all 8 times!

#define DEBUG_ACCESSORS(methodName)												\
	void																		\
	XPFPrefs::set##methodName (bool val) 										\
	{																			\
		if ((fDebug & k##methodName) != (val ? k##methodName : 0)) {			\
			if (val) fDebug |= k##methodName; else fDebug &= ~k##methodName;	\
			Changed (cSet##methodName, &val);									\
		}																		\
	}																			\
																				\
	bool																		\
	XPFPrefs::get##methodName ()												\
	{																			\
		return (fDebug & k##methodName) ? true : false;							\
	}
		
DEBUG_ACCESSORS (DebugBreakpoint);
DEBUG_ACCESSORS (DebugPrintf);
DEBUG_ACCESSORS (DebugNMI);
DEBUG_ACCESSORS (DebugKprintf);
DEBUG_ACCESSORS (DebugDDB);
DEBUG_ACCESSORS (DebugSyslog);
DEBUG_ACCESSORS (DebugARP);
DEBUG_ACCESSORS (DebugOldGDB);
DEBUG_ACCESSORS (DebugPanicText);

// Convenience functions for setting input and output devices based on their labels.
// I.e. what would be displayed in a popup menu, for instance.

void
XPFPrefs::setInputDevice (char *label, bool callChanged)
{
	XPFIODevice *newVal = XPFIODevice::InputDeviceWithLabel (label);
	if (newVal == NULL) newVal = XPFIODevice::InputDeviceWithOpenFirmwareName (label);
	if (newVal == NULL) newVal = XPFIODevice::InputDeviceWithShortOpenFirmwareName (label);
	if (callChanged) {
		setInputDevice (newVal);
	} else {
		fInputDevice = newVal;
	}
}

void
XPFPrefs::setOutputDevice (char *label, bool callChanged)
{
	XPFIODevice *newVal = XPFIODevice::OutputDeviceWithLabel (label);
	if (newVal == NULL) newVal = XPFIODevice::OutputDeviceWithOpenFirmwareName (label);
	if (newVal == NULL) newVal = XPFIODevice::OutputDeviceWithShortOpenFirmwareName (label);
	if (callChanged) {
		setOutputDevice (newVal);
	} else {
		fOutputDevice = newVal;
	}
}

// Convenience functions to get the index of an input or output device in its list.

unsigned
XPFPrefs::getInputDeviceIndex ()
{
	if (!fInputDevice) return 0;
	return XPFIODevice::GetInputDeviceList ()->GetIdentityItemNo (fInputDevice);
}

unsigned
XPFPrefs::getOutputDeviceIndex ()
{
	if (!fOutputDevice) return 0;
	return XPFIODevice::GetOutputDeviceList ()->GetIdentityItemNo (fOutputDevice);
}
