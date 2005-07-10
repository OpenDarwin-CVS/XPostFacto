/*

Copyright (c) 2002 - 2005
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
#include "XPFSynchronizeCommand.h"
#include "XPFAuthorization.h"
#include "XPFErrors.h"
#include "HFSPlusArchive.h"
#include "XPFUpdate.h"
#include "XPFUpdateWindow.h"
#include "vers_rsrc.h"
#include "XPFBus.h"
#include "XPFBootableDevice.h"

#include <string.h>

#ifdef __MACH__
	#include <sys/types.h>
	#include <sys/wait.h>
#else
	#include <PCI.h>
#endif

#include <stdio.h>

class XPFPollVolumesBehavior : public TBehavior {

	public:
	
		XPFPollVolumesBehavior () {
			SetIdleFreq (10 * 60);
		}

		virtual bool DoIdle (IdlePhase phase) {
			if (phase == idleContinue) MountedVolume::Initialize ();
			return false;
		}
};

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
		fUsePatchedRagePro (false),
		fAutoBoot (true),
		fBootInSingleUserMode (false),
		fBootInVerboseMode (false),
		fThrottle (0),
		fTargetDisk (NULL),
		fMacOS9Disk (NULL),
		fInstallCD (NULL),
		fInputDevice (NULL),
		fOutputDevice (NULL),
		fOptionsWindow (NULL),
		fDebug (0),
		fRebootInMacOS9 (false),
		fShowHelpTags (true),
		fUseROMNDRV (false),
		fUseShortStrings (true),
		fUseShortStringsForInstall (true),
		fTooBigForNVRAM (false),
		fTooBigForNVRAMForInstall (false),
		fForceAskSave (false),
		fRestartOnClose (false),
		fRegisteredVersion (0),
		fNoInputDevice (false),
		fNoOutputDevice (false)
{
	SetAskOnClose (true);
}

XPFPrefs::~XPFPrefs ()
{
	if (fOptionsWindow) delete fOptionsWindow;
}

void
XPFPrefs::Close ()
{
#ifndef __MACH__
	// Sync with changes to balloon help
	setShowHelpTags (HMGetBalloons ());
#endif

	// Do the superclass Close, so that we save if we need to save etc.
	Inherited::Close ();
	
	bool synchronizationFailed = false;
	
	try {	
		// Now, we check NVRAM and do whatever copying is necessary
		XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
		nvram->readFromNVRAM ();
		
		char *bootArgs = nvram->getStringValue ("boot-args");
		char *bootDevice = nvram->getStringValue ("boot-device");
		
		if (!strstr (bootDevice, "/AAPL,ROM")) {
			MountedVolume *bootDisk = MountedVolume::WithOpenFirmwarePath (bootDevice);
			MountedVolume *rootDisk = bootDisk;
			
			char *rdString = strstr (bootArgs, "rd=*");
			if (rdString) {
				char rootPath[256];
				strcpy (rootPath, rdString + strlen ("rd=*"));
				char *pos = strchr (rootPath, ' ');
				if (pos) *pos = 0;
				
				rootDisk = MountedVolume::WithOpenFirmwarePath (rootPath);
				if (!rootDisk) rootDisk = bootDisk;
			}	
						
			if (rootDisk && (rootDisk != bootDisk)) {
				XPFUpdate update (rootDisk, bootDisk);
				if (update.getRequiresSynchronization ()) {
					PerformCommand (TH_new XPFSynchronizeCommand (&update));
				}
				
				// If the root disk was not writeable, then force an update if necessary
				// The XPFUpdate class checks and updates /Library/Extensions in the helper
				// directory in this case.
				if (!rootDisk->getIsWriteable () && update.getRequiresAction ()) {
					PerformCommand (TH_new XPFUpdateCommand (&update));
				}
			}
		}
	}
	
	catch (CException_AC &ex) {
		synchronizationFailed = true;
		ErrorAlert (ex.GetError (), ex.GetExceptionMessage ());
	}
		
#ifdef __MACH__
	restartStartupItem ();
#endif

	if (synchronizationFailed) {
		// If synchronization fails, then we put up a warning message and set up
		// reboot in Mac OS 9.
		
		// FIXME -- this will not work on some newer "New World" machines which cannot
		// boot in Mac OS 9. I'm not sure how to identify them, though. And they are not
		// the machines that people are likely to be using with XPostFacto :-)
		fRebootInMacOS9 = true;
		ErrorAlert (kSynchronizationFailed, 0);
		try {
			writePrefsToNVRAM (false);
		}
		catch (CException_AC &ex) {
			ErrorAlert (ex.GetError (), ex.GetExceptionMessage ());
		}
		fRebootInMacOS9 = false; // so we don't save the pref that way
	} else {
		if (fRestartOnClose) tellFinderToRestart ();
	}
	
	if (!gApplication->GetDone ()) gApplication->DoMenuCommand (cQuit);
}

// We check for updates when we write prefs to NVRAM (i.e. at quitting time).
// We also check at launch, but don't offer to do the update then -- we defer it
// to quitting time.

void
XPFPrefs::checkForUpdates (bool forInstall)
{
	XPFUpdate update (fTargetDisk, fTargetDisk->getHelperDisk (), forInstall ? fInstallCD : NULL);
		
	if (update.getRequiresAction ()) {
		if (!forInstall && fTargetDisk->getIsWriteable ()) {
			MAParamText ("$VOLUME$", fTargetDisk->getVolumeName ());
			
			XPFUpdateWindow *dialog = (XPFUpdateWindow *) TViewServer::fgViewServer->NewTemplateWindow (kUpdateWindow, NULL);
			dialog->setUpdateItemList (update.getItemList ());
			
			IDType result = dialog->PoseModally ();
			dialog->Close ();
			
			if (result == 'upda') PerformCommand (TH_new XPFUpdateCommand (&update));
		}	
	}
}

short
XPFPrefs::PoseConfirmDialog (bool forInstall, bool quitting)
{
	short retVal = kStdCancelItemIndex;
	
	MountedVolume *bootDisk = NULL;
	CStr255_AC version;
	
	if (fRebootInMacOS9) {
		bootDisk = fMacOS9Disk;

		if (!bootDisk) return kNoButton;
		if (bootDisk->getMacOS9BootStatus () != kStatusOK) return kNoButton;
		
		version = "Mac OS 9.x";  // FIXME -- would be nice to grab real version
	} else {
		bootDisk = forInstall ? fInstallCD : fTargetDisk;

		if (!bootDisk) return kNoButton;
		if (!forInstall && (bootDisk->getBootStatus () != kStatusOK)) return kNoButton;
		
		version = "Mac OS X ";
		version += bootDisk->getMacOSXVersion ();
	}	

	MAParamText ("$OS$", version);
	MAParamText ("$VOLUME$", bootDisk->getVolumeName ());
	if (forInstall) MAParamText ("$TARGET$", fTargetDisk->getVolumeName ());
	
	IDType dialogID;
	if (forInstall) {
		dialogID = quitting ? kInstallDialog : kInstallNowDialog;
	} else {
		dialogID = quitting ? kRestartDialog : kRestartNowDialog;
	}
	
	TWindow *dialog = TViewServer::fgViewServer->NewTemplateWindow (dialogID, NULL);
	
	if (getTooBigForNVRAM (forInstall)) {
		TView *change = dialog->FindSubView ('chan');
		if (change) change->SetActiveState (false, true);
	}
	
	IDType result = dialog->PoseModally ();
	switch (result) {
		case 'chan':
			retVal = kStdOkItemIndex;
			break;
			
		case 'canc':
			retVal = kStdCancelItemIndex;
			break;
			
		case 'dont':
			retVal = kNoButton;
			break;
	}
	
	dialog->Close ();

	return retVal;
}

short 
XPFPrefs::PoseSaveDialog ()
{
	// We only use the install dialog if we can't start up from the target disk already
	bool useInstallDialog = (fInstallCD != NULL) && !fRebootInMacOS9 && (fTargetDisk->getBootStatus () != kStatusOK);
	
	short retVal = PoseConfirmDialog (useInstallDialog, true);
		
	if (retVal == kStdOkItemIndex) {
		if (useInstallDialog) {
			PerformCommand (TH_new XPFInstallCommand (this));
		} else {
			PerformCommand (TH_new XPFRestartCommand (this, false));
		}		
	}
	
	return retVal;
}

void
XPFPrefs::getPrefsFromNVRAM ()
{		
	bool nvramBootsInto9 = false;
	MountedVolume *bootVolumeMacOS9 = NULL;
	
	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();

	char *bootCommand = nvram->getStringValue ("boot-command");
	char *bootDevice = nvram->getStringValue ("boot-device");
	char *bootFile = nvram->getStringValue ("boot-file");
	char *inputDevice = nvram->getStringValue ("input-device");
	char *outputDevice = nvram->getStringValue ("output-device");	
	char *bootArgs = nvram->getStringValue ("boot-args");
	
	// Check to see whether NVRAM is set to boot back into Mac OS 9
	if (strstr (bootDevice, "/AAPL,ROM")) {
		nvramBootsInto9 = true;
	} else if (strstr (bootCommand, "mac-boot")) {
		// For New World, we have to check what the blessed folder is
		if (!strstr (bootArgs, "rd=")) {
			bootVolumeMacOS9 = MountedVolume::WithOpenFirmwarePath (bootDevice);
			if (bootVolumeMacOS9) nvramBootsInto9 = bootVolumeMacOS9->getBlessedFolderID () == bootVolumeMacOS9->getMacOS9SystemFolderNodeID ();
		}
	}

	// We use the pref to reboot into Mac OS X even if the NVRAM says to boot in 9
	// But we need to force save in that case
	if (nvramBootsInto9 != fRebootInMacOS9) fForceAskSave = true;

	// For most settings, we use what is in our preferences rather than what is in NVRAM,
	// unless there is a setting in NVRAM that is likely meant to override our file setting.
	// For any mismatch, we force the app to ask about saving. 
	
	if (fAutoBoot != nvram->getBooleanValue ("auto-boot?")) fForceAskSave = true;
	
	if (nvramBootsInto9) {
		// If we know the boot volume, we can check it against the prefs. This is only useful
		// on New World -- on Old World, we won't know the boot volume. We'll use the boot
		// volume from NVRAM if we know it.
		if (bootVolumeMacOS9 && (bootVolumeMacOS9 != fMacOS9Disk)) {
			setMacOS9Disk (bootVolumeMacOS9);
			fForceAskSave = true;
		}
		
	} else {
		// These are only interesting if NVRAM was set to boot into Mac OS X
		
		if (fBootInSingleUserMode != (strstr (bootArgs, "-s") != 0)) fForceAskSave = true;
		if (fBootInVerboseMode != (strstr (bootArgs, "-v") != 0)) fForceAskSave = true;
			
		char *debugString = strstr (bootArgs, "debug=");
		if (debugString) {
			debugString += strlen ("debug=");
			UInt32 debug = strtoul (debugString, NULL, 0);
			// If there is an actual debug string in NVRAM, assume we want to use it.
			if (fDebug != debug) {
				fDebug = debug;
				fForceAskSave = true;
			}
		} else {
			// If no debug string, then force save if we have a debug value
			if (fDebug) fForceAskSave = true;
		}
		
		char *romndrvstring = strstr (bootArgs, "romndrv=");
		if (romndrvstring) {
			romndrvstring += strlen ("romndrv=");
			UInt32 romndrv = strtoul (romndrvstring, NULL, 0);
			// If there is a romndrv= string, assume we want to use it
			if (fUseROMNDRV != romndrv) {
				fUseROMNDRV = romndrv;
				fForceAskSave = true;
			}
		} else {
			// If no romndrv= string, then force save if we have a romndrv value
			if (fUseROMNDRV) fForceAskSave = true;
		}
		
		// For the target disk and helper disk, we use what is in NVRAM rather than preferences file
		// But, once again, we ask to save preferences if they are different
		
		MountedVolume *bootDisk = MountedVolume::WithOpenFirmwarePath (bootDevice);
		MountedVolume *rootDisk = bootDisk;
		
		char *rdString = strstr (bootArgs, "rd=*");
		if (rdString) {
			char rootPath[1024];
			strcpy (rootPath, rdString + strlen ("rd=*"));
			char *pos = strchr (rootPath, ' ');
			if (pos) *pos = 0;
			
			rootDisk = MountedVolume::WithOpenFirmwarePath (rootPath);
			if (!rootDisk) rootDisk = bootDisk;
		}	
				
		// Only honour the rootDisk if it is not an installer -- if it is an installer, then force
		// the user to confirm changes no matter what
		if (rootDisk && (rootDisk->getBootStatus () == kStatusOK) && (rootDisk->getInstallerStatus () != kStatusOK)) {
			if (rootDisk != fTargetDisk) fForceAskSave = true;
			setTargetDisk (rootDisk);
			if (bootDisk && (bootDisk != rootDisk)) {
				if (bootDisk->getHelperStatus() == kStatusOK) {
					if (bootDisk != fTargetDisk->getHelperDisk ()) fForceAskSave = true;
					fTargetDisk->setHelperDisk (bootDisk);
				} else {
					fForceAskSave = true;
				}
			}
			
			// We check at launch for available updates to the current configuration
			// We don't offer the updates yet (because it's a bit startling to be offered
			// an update before you've really done anything), but we do force a save, so that
			// we can be sure that the update will be offered later.
			if (!fForceAskSave) {
				XPFUpdate update (fTargetDisk, fTargetDisk->getHelperDisk (), NULL);
				if (update.getRequiresAction ()) fForceAskSave = true;
			}
		} else {
			// If we can't set the target disk to the current root, then we'd ask
			// user about saving (since our pref won't match what's in NVRAM).
			fForceAskSave = true;
		}
	}
	
	// For input and output devices, we'll use the NVRAM setting if there is no pref setting
	XPFIODevice *input = XPFIODevice::InputDeviceWithOpenFirmwareName (inputDevice);
	XPFIODevice *output = XPFIODevice::OutputDeviceWithOpenFirmwareName (outputDevice);
	
	if (input) {
		// If we don't have an input-device in prefs, use the one from NVRAM
		if (!fInputDevice) {
			gLogFile << "No input device, so setting" << endl_AC;
			fInputDevice = input;
			fForceAskSave = true;
		} else {
			if (fInputDevice != input) fForceAskSave = true;
		}
	} else {
		// If we have an input device that is not in NVRAM, then force save
		if (fInputDevice) fForceAskSave = true;
	}
	
	if (output) {
		// If we don't have an output-device in prefs, use the one from NVRAM
		if (!fOutputDevice) {
			fOutputDevice = output;
			fForceAskSave = true;
		} else {
			if (fOutputDevice != output) fForceAskSave = true;
		}
	} else {
		// If we have an output device that is not in NVRAM, then force save
		if (fOutputDevice) fForceAskSave = true;
	}
	
	// throttle
	if (!XPFPlatform::GetPlatform()->getIsNewWorld ()) {
		char *nvramrc = nvram->getStringValue ("nvramrc");

		UInt32 throttleVal = 0;
		char *throttle = strstr (nvramrc, "$I");
		if (throttle) {
			throttle = strstr (throttle + 2, "$I");
			if (throttle) {
				do {throttle--;} while (*throttle == ' ');
				do {throttle--;} while (*throttle != ' ');
				throttleVal = strtoul (throttle, NULL, 16);
			}
		}
		if (throttleVal & 0x1) {
			throttleVal >>= 1; // accounts for the on/off bit at the end
		} else {
			throttleVal = 0;
		}
		
		if (throttleVal) {
			if (fThrottle != throttleVal) {
				fThrottle = throttleVal;
				fForceAskSave = true;
			}
		} else {
			if (fThrottle) fForceAskSave = true;
		}
	}
}

void 
XPFPrefs::installXPFPartitionInfo ()
{
#ifdef __MACH__
	// FIXME -- Should I be trying to kextunload it when I quit?
	FSRef tmpDirectory;
	OSErr err = FSFindFolder (kOnAppropriateDisk, kTemporaryFolderType, true, &tmpDirectory);
	if (err == noErr) {
		CResourceStream_AC stream ('hfsB', 300);
		HFSPlusArchive archive (&stream, NULL, this);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (&tmpDirectory));
		
		char path[1024];
		err = FSRefMakePath (&tmpDirectory, (UInt8 *) path, 1023);
		if (err == noErr) {
			strcat (path, "/XPFPartitionInfo.kext");
			pid_t pid = fork ();
			if (pid) {
				if (pid != -1) {
					int status;
					waitpid (pid, &status, 0);
				}
			} else {
				execl ("/sbin/kextload", "kextload", path, NULL);
				ThrowException_AC (kInternalError, 0);	// the execl shouldn't return
			}
		}
	}
#endif
}

void
XPFPrefs::DoInitialState ()
{
	Inherited::DoInitialState ();
	
	installXPFPartitionInfo ();

	MountedVolume::Initialize ();
	
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		iter->AddDependent (this);
	}
	
	gApplication->AddDependent (this);
	
	fMacOS9Disk = MountedVolume::GetDefaultMacOS9Disk ();
	fTargetDisk = MountedVolume::GetDefaultRootDisk ();
	fInstallCD = MountedVolume::GetDefaultInstallerDisk ();
	if (fInstallCD && !fTargetDisk) fTargetDisk = MountedVolume::GetDefaultInstallTarget ();

	checkStringLength ();	
	
	suspendStartupItem ();

#ifdef __MACH__
	gApplication->AddBehavior (new XPFPollVolumesBehavior);
#endif
}

void
XPFPrefs::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	
	switch (theChange) {
				
		case cNewMountedVolume:
			volume->AddDependent (this);
			if (fInstallCD == NULL) setInstallCD (MountedVolume::GetDefaultInstallerDisk ());
			
			// Need to check if it has a helper. Not an efficient method, but easy to write ...
			for (TemplateAutoList_AC <XPFHelperItem>::Iterator iter (&fHelperList); iter.Current (); iter.Next ()) {
				if (volume == MountedVolume::WithInfo (&iter->target)) {
					MountedVolume *helper = MountedVolume::WithInfo (&iter->helper);
					if (helper) {
						volume->setHelperDisk (helper);
					} else {
						// presumably it needs a helper, so we'll pick another
						volume->setHelperDisk (volume->getDefaultHelperDisk ());
					}				
					break;
				}
			}

			break;
			
		case cDeleteMountedVolume:
			if (volume == fMacOS9Disk) setMacOS9Disk (MountedVolume::GetDefaultMacOS9Disk ());
			if (volume == fTargetDisk) setTargetDisk (MountedVolume::GetDefaultRootDisk ());
			if (volume == fInstallCD) setInstallCD (MountedVolume::GetDefaultInstallerDisk ());
			break;
		
		case cSetHelperDisk:
			if (volume == fTargetDisk) {
				Changed (cSetHelperDiskForTarget, volume);
			} else {
				SetChangeCount (GetChangeCount () + 1);
			}
			
			// Need to update our internal list. First, we find the current entry and delete it.
			for (TemplateAutoList_AC <XPFHelperItem>::Iterator iter (&fHelperList); iter.Current (); iter.Next ()) {
				if (volume == MountedVolume::WithInfo (&iter->target)) {
					fHelperList.Delete (iter.Current ());
					break;
				}
			}

			// Then, if there is a helper, we record it.
			MountedVolume *helper = volume->getHelperDisk ();
			if (helper) {
				XPFHelperItem *item = new XPFHelperItem;
				memcpy (&item->target, volume->getVolumeInfo(), sizeof (item->target));
				memcpy (&item->helper, helper->getVolumeInfo(), sizeof (item->helper));
				fHelperList.InsertLast (item);
			}
			
			break;
			
		case cSetBus:
			// Bump the change count
			SetChangeCount (GetChangeCount () + 1);

			// Need to update our internal list. First, we find the current entry and delete it.
			XPFBootableDevice *device = volume->getBootableDevice ();
			if (!device) break;
			
			for (TemplateAutoList_AC <XPFDeviceHint>::Iterator iter (&fDeviceHints); iter.Current (); iter.Next ()) {
				if (device == XPFBootableDevice::DeviceWithDeviceIdent (iter->deviceIdent)) {
					fDeviceHints.Delete (iter.Current ());
					break;
				}
			}

			// Then, if there is a hint, we record it.
			XPFBus *bus = device->getBus ();
			if (bus != device->getDefaultBus ()) {
				XPFDeviceHint *hint = new XPFDeviceHint;
				hint->deviceIdent = device->getDeviceIdent ();
				hint->busOFName.CopyFrom (bus->getOpenFirmwareName (false));
				fDeviceHints.InsertLast (hint);
			}
			
			// Broadcast so that the settings display picks this up
			Changed (theChange, changeData);
			
			break;
						
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

void
XPFPrefs::RegainControl ()
{
	TDocument::RegainControl ();	// we skip TFileBasedDocument
	MountedVolume::Initialize ();
	if (!fInstallCD) setInstallCD (MountedVolume::GetDefaultInstallerDisk ());
}

void
XPFPrefs::DoRead (TFile* aFile, bool forPrinting)
{
	#pragma unused (forPrinting)
	
	CFileStream_AC fileStream (aFile);

	UInt32 sig, prefsVersion;
	CStr255_AC device;
	FSVolumeInfo volInfo, helperInfo;
	
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
		if (rootDisk && (rootDisk->getBootStatus () == kStatusOK)) setTargetDisk (rootDisk, false);
			
		UInt32 helpers;
		fileStream >> helpers;
		
		// We need to keep track of helper preferences separately from the actual
		// MountedVolume instances, because Firewire disks can come and go.
		// So we maintain our own list, and keep it synchronized with changes
		// (see the DoUpdate method).

		while (helpers > 0) {
			helpers--;

			fileStream.ReadBytes (&volInfo, sizeof (volInfo));
			fileStream.ReadBytes (&helperInfo, sizeof (helperInfo));
			
			MountedVolume *vol = MountedVolume::WithInfo (&volInfo);
			MountedVolume *helper = MountedVolume::WithInfo (&helperInfo);
			
			if (vol && !helper) {
				// Presumably it needs a helper, so we'll use the default
				helper = vol->getDefaultHelperDisk ();
				if (helper) memcpy (&helperInfo, helper->getVolumeInfo(), sizeof (helperInfo));
				fForceAskSave = true;
			}
			
			XPFHelperItem *helperItem = new XPFHelperItem;
			BlockMoveData (&volInfo, &helperItem->target, sizeof (volInfo));
			BlockMoveData (&helperInfo, &helperItem->helper, sizeof (helperInfo));
			fHelperList.InsertLast (helperItem);
			
			if (vol) vol->setHelperDisk (helper, false);
		}
		
		fileStream >> fUseROMNDRV;
		fileStream >> fRegisteredVersion;
		fileStream >> fUsePatchedRagePro;
		fileStream >> fRebootInMacOS9;
		
		// Restore non-default bus selections
		UInt32 deviceHints;
		fileStream >> deviceHints;
		
		while (deviceHints > 0) {
			deviceHints--;
			
			XPFDeviceHint *hint = new XPFDeviceHint;
								
			fileStream >> hint->deviceIdent;
			hint->busOFName = fileStream.ReadString (255);
			
			fDeviceHints.InsertLast (hint);

#ifndef __MACH__
			XPFBootableDevice *device = XPFBootableDevice::DeviceWithDeviceIdent (hint->deviceIdent);
			XPFBus *bus = XPFBus::WithOpenFirmwareName ((CChar255_AC) hint->busOFName);
		
			if (bus && device) device->setBus (bus);
#endif
		}
		
		fileStream >> fShowHelpTags;
		implementShowHelpTags ();
		
		fileStream >> fNoInputDevice;
		fileStream >> fNoOutputDevice;

		fileStream.ReadBytes (&volInfo, sizeof (volInfo));
		// Don't use the pref if we can't find the disk
		MountedVolume *macOS9Disk = MountedVolume::WithInfo (&volInfo);
		if (macOS9Disk && (macOS9Disk->getMacOS9BootStatus () == kStatusOK)) setMacOS9Disk (macOS9Disk, false);
	}
	
	catch (...) {
	}
	
	// Now, get prefs from NVRAM to see what's changed (if anything)
	getPrefsFromNVRAM ();
	
	// If there is no input device or output device, then pick a default
	// Unless the user has deliberately chosen not to have one
	if (!fInputDevice && !fNoInputDevice) {
		setInputDevice (XPFIODevice::GetDefaultInputDevice ());
		if (fInputDevice) fForceAskSave = true;
	}
	
	if (!fOutputDevice && !fNoOutputDevice) {
		setOutputDevice (XPFIODevice::GetDefaultOutputDevice ());
		if (fOutputDevice) fForceAskSave = true;
	}
	
	// Then patch the NVRAM so that checkStringLength () works properly
	XPFPlatform *platform = XPFPlatform::GetPlatform ();
	if (platform->getCanPatchNVRAM ()) platform->patchNVRAM ();
	
	checkStringLength ();
	
	// And make sure that auto-boot is on if there is no input device or output device
	if (!fAutoBoot && (!fInputDevice || !fOutputDevice)) {
		fAutoBoot = true;
		fForceAskSave = true;
	}
}

void
XPFPrefs::writePrefsToNVRAM (bool forInstall)
{
	if (((XPFApplication *) gApplication)->getDebugOptions () & kDisableNVRAMWriting) return;

	XPFPlatform *platform = XPFPlatform::GetPlatform ();
	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
	
	if (fRebootInMacOS9) {
		if (fMacOS9Disk) fMacOS9Disk->blessMacOS9SystemFolder ();	
	} else {
		if (!forInstall && fTargetDisk && platform->getIsNewWorld ()) fTargetDisk->blessCoreServicesFolder ();
		checkForUpdates (forInstall);
	}
	
	CChar255_AC bootDevice = getBootDevice (forInstall);
	CChar255_AC bootFile = getBootFile (forInstall);
	CChar255_AC inputDevice = getInputDevice (forInstall);
	CChar255_AC outputDevice = getOutputDevice (forInstall);
	CChar255_AC bootCommand = getBootCommand ();
	CChar255_AC bootArgs = getBootArgs (forInstall);
	
	nvram->setStringValue ("boot-command", bootCommand);
	nvram->setStringValue ("boot-args", bootArgs);
	nvram->setStringValue ("boot-device", bootDevice);
	nvram->setStringValue ("boot-file", bootFile);
	nvram->setStringValue ("input-device", inputDevice);
	nvram->setStringValue ("output-device", outputDevice);
	
	nvram->setBooleanValue ("auto-boot?", fAutoBoot);	

	// adjust throttle
	if (!platform->getIsNewWorld ()) {
		if (!platform->getCanPatchNVRAM ()) ThrowException_AC (kErrorWritingNVRAM, 0);	
		char nvramrc [2048];
		strcpy (nvramrc, nvram->getStringValue ("nvramrc"));
		char *ictc = strstr (nvramrc, "11 $I");
		if (ictc) {
			char throttleValue [12];
			sprintf (throttleValue, "%X", fThrottle ? (fThrottle << 1) | 1 : 0);
			ictc [0] = ' ';
			ictc [1] = ' ';
			BlockMoveData (throttleValue, ictc, strlen (throttleValue));
		} 
		nvram->setStringValue ("nvramrc", nvramrc);
	}
	
	#if qLogging
		gLogFile << "Writing NVRAM ..." << endl_AC;
		gLogFile << "boot-device: " << bootDevice << endl_AC;
		gLogFile << "boot-command: " << bootCommand << endl_AC;
		gLogFile << "boot-args: " << bootArgs << endl_AC;
		gLogFile << "input-device: " << inputDevice << endl_AC;
		gLogFile << "output-device: " << outputDevice << endl_AC;
	#endif
	
	OSErr err = nvram->writeToNVRAM ();
	if (err != noErr) {
		gLogFile << "Error: " << err << " while writing NVRAM" << endl_AC;
		ThrowException_AC (kErrorWritingNVRAM, 0);
	}
}

void 
XPFPrefs::SaveFile (CommandNumber itsCommandNumber, TFile* theSaveFile, CSaveOptions *itsOptions)
{
	XPFSetUID myUID (0);
	Inherited::SaveFile (itsCommandNumber, theSaveFile, itsOptions);
}

void
XPFPrefs::DoWrite (TFile* aFile, bool makingCopy)
{
	#pragma unused (makingCopy)
	
	XPFSetUID myUID (0);
	
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
	
	device = getInputDevice (false);
	fileStream.WriteString (device);
		
	device = getOutputDevice (false);
	fileStream.WriteString (device);
	
	fUseShortStrings = save;
	
	FSVolumeInfo info;
	BlockZero_AC (info);
	if (fTargetDisk) memcpy (&info, fTargetDisk->getVolumeInfo (), sizeof (info));
	fileStream.WriteBytes (&info, sizeof (info));
	
	// Now, we need to store the choices for Helper Disk (which are per-volume). 
	
	fileStream << fHelperList.GetSize ();

	for (TemplateAutoList_AC <XPFHelperItem>::Iterator iter (&fHelperList); iter.Current (); iter.Next ()) {
		fileStream.WriteBytes (&iter->target, sizeof (iter->target));
		fileStream.WriteBytes (&iter->helper, sizeof (iter->helper));
	}	
	
	fileStream << fUseROMNDRV;
	fileStream << fRegisteredVersion;
	fileStream << fUsePatchedRagePro;
	fileStream << fRebootInMacOS9;
	
	// Now, we store the non-default bus selections (if any)	
	fileStream << fDeviceHints.GetSize ();

	for (TemplateAutoList_AC <XPFDeviceHint>::Iterator iter (&fDeviceHints); iter.Current (); iter.Next ()) {
		fileStream << iter->deviceIdent;
		fileStream.WriteString (iter->busOFName);
	}
	
	fileStream << fShowHelpTags;
	
	// Record if the user has selected no input/output device
	fNoInputDevice = !fInputDevice;
	fNoOutputDevice = !fOutputDevice;
	
	fileStream << fNoInputDevice;
	fileStream << fNoOutputDevice;
	
	BlockZero_AC (info);
	if (fMacOS9Disk) memcpy (&info, fMacOS9Disk->getVolumeInfo (), sizeof (info));
	fileStream.WriteBytes (&info, sizeof (info));
}

void 
XPFPrefs::DoMakeViews (bool forPrinting)
{
	((XPFApplication *) gApplication)->CloseSplashWindow ();
	
	Inherited::DoMakeViews (forPrinting);
	
	// Not the most elegant place, but we need to do it after reading from NVRAM,
	// and before Close ()
	if (fForceAskSave) SetChangeCount (GetChangeCount () + 1);
	
	checkRegistration ();
}

void
XPFPrefs::checkRegistration ()
{
	if (getIsRegistered ()) return;
	TWindow *dialog = TViewServer::fgViewServer->NewTemplateWindow (kRegisterWindow, NULL);
	IDType result = dialog->PoseModally ();
	dialog->Close ();
	
	if (result == 'regi') {
		dialog = TViewServer::fgViewServer->NewTemplateWindow (kRegisterThankYou, NULL);
		dialog->PoseModally ();
		dialog->Close ();
		setIsRegistered (true);
	}
}

void 
XPFPrefs::DoSetupMenus ()
{
	Inherited::DoSetupMenus ();
	
	Enable (cInstallBootX, fTargetDisk != NULL);
	Enable (cInstallExtensions, fTargetDisk != NULL);
	Enable (cInstallStartupItem, fTargetDisk != NULL);
	Enable (cInstallEverything, fTargetDisk != NULL);
	Enable (cShowOptionsWindow, true);
	Enable (cRecopyHelperFiles, fTargetDisk && fTargetDisk->getHelperDisk () != NULL);
	Enable (cUninstall, fTargetDisk && fTargetDisk->getIsWriteable ());
	Enable (cBlessMacOS9SystemFolder, fTargetDisk && fTargetDisk->getMacOS9SystemFolderNodeID ());
	Enable (cEmptyCache, fTargetDisk != NULL);
	Enable (cCheckPermissions, fTargetDisk != NULL);
	Enable (cFixSymlinks, fTargetDisk && fTargetDisk->getIsWriteable () && fTargetDisk->getSymlinkStatus () != kSymlinkStatusCannotFix);
}

void 
XPFPrefs::DoMenuCommand (CommandNumber aCommandNumber)
{
	XPFUpdate *update = NULL;

	switch (aCommandNumber) 
	{			
		case cInstallBootX:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFInstallBootXCommand (update));
			break;
			
		case cInstallExtensions:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFInstallExtensionsCommand (update));
			break;
		
		case cInstallStartupItem:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFInstallStartupCommand (update));
			break;
			
		case cInstallEverything:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFInstallBootXCommand (update));
			PerformCommand (TH_new XPFInstallExtensionsCommand (update));
			PerformCommand (TH_new XPFInstallStartupCommand (update));
			break;
						
		case cRecopyHelperFiles:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFRecopyHelperFilesCommand (update));
			break;
			
		case cBlessMacOS9SystemFolder:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFBlessMacOS9SystemFolderCommand (update));
			break;
			
		case cUninstall:
			MAParamText ("$VOLUME$", fTargetDisk->getVolumeName ());
				
			TWindow *dialog = TViewServer::fgViewServer->NewTemplateWindow (kUninstallDialog, NULL);
			IDType result = dialog->PoseModally ();
			dialog->Close ();
				
			if (result == 'chan') {
				update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
				PerformCommand (TH_new XPFUninstallCommand (update));			
			}
			break;
			
		case cEmptyCache:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFEmptyCacheCommand (update));
			break;
			
		case cCheckPermissions:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFCheckPermissionsCommand (update));
			break;
			
		case cFixSymlinks:
			update = new XPFUpdate (fTargetDisk, fTargetDisk->getHelperDisk ());
			PerformCommand (TH_new XPFFixSymlinksCommand (update));
			break;
			
		case cShowOptionsWindow:
			if (fOptionsWindow) {
				fOptionsWindow->Show (true, false);
				fOptionsWindow->Select ();
			} else {
				fOptionsWindow = TViewServer::fgViewServer->NewTemplateWindow (kOptionsWindow, this);
				fOptionsWindow->Open ();
			}
			break;
			
		default:
			Inherited::DoMenuCommand (aCommandNumber);
			break;
	}
	
	if (update) delete update;
}

void
XPFPrefs::DoEvent (EventNumber eventNumber,
							TEventHandler* source,
							TEvent* event)
{
	switch (eventNumber) {
		case cShowOptionsWindow:
			if (fOptionsWindow) {
				fOptionsWindow->Show (true, false);
				fOptionsWindow->Select ();
			} else {
				fOptionsWindow = TViewServer::fgViewServer->NewTemplateWindow (kOptionsWindow, this);
				fOptionsWindow->Open ();
			}
			break;
				
		default:
			Inherited::DoEvent (eventNumber, source, event);
			break;
	}
}

void
XPFPrefs::checkStringLength ()
{
	if (XPFPlatform::GetPlatform()->getIsNewWorld()) {
		fTooBigForNVRAM = false;
		fTooBigForNVRAMForInstall = false;
		return;
	}

	unsigned nvramPatchLength = strlen (XPFNVRAMSettings::GetSettings ()->getStringValue ("nvramrc"));

	unsigned len = getBootCommand ().Length () + 
				   getBootArgs (false).Length () +
				   getBootFile (false).Length () +
				   getBootDevice (false).Length () +
				   getInputDevice (false).Length () +
				   getOutputDevice (false).Length () +
				   nvramPatchLength;

	fTooBigForNVRAM = (len >= kOFStringCapacity);

	len = getBootCommand ().Length () + 
		  getBootArgs (true).Length () +
		  getBootFile (true).Length () +
		  getBootDevice (true).Length () +
		  getInputDevice (true).Length () +
		  getOutputDevice (true).Length () +
		  nvramPatchLength;
						  
	fTooBigForNVRAMForInstall = (len >= kOFStringCapacity);
}

void 
XPFPrefs::Changed(ChangeID_AC theChange, void* changeData)
{
	checkStringLength ();
	
	Inherited::Changed (theChange, changeData);
}

void
XPFPrefs::suspendStartupItem ()
{
#ifdef __MACH__
	XPFSetUID myUID (0);
	system ("/Library/StartupItems/XPFStartupItem/XPFStartupItem suspend");
#endif
}

void
XPFPrefs::restartStartupItem ()
{
#ifdef __MACH__
	XPFSetUID myUID (0);
	system ("/Library/StartupItems/XPFStartupItem/XPFStartupItem restart");
#endif
}

void
XPFPrefs::tellFinderToRestart ()
{
	if (((XPFApplication *) gApplication)->getDebugOptions () & kDisableRestart) return;

	AEDesc finderAddr;
	AppleEvent myRestart, nilReply;
	AEEventClass eventClass;

#ifdef __MACH__
	eventClass = kCoreEventClass;
	ProcessSerialNumber psn = {0, kSystemProcess};
	ThrowIfOSErr_AC (AECreateDesc (typeProcessSerialNumber, &psn, sizeof (psn), &finderAddr));
#else
	eventClass = kAEFinderEvents;
	OSType fndrSig = 'MACS';
    ThrowIfOSErr_AC (AECreateDesc (typeApplSignature, &fndrSig, sizeof(fndrSig), &finderAddr));
#endif

   	ThrowIfOSErr_AC (AECreateAppleEvent (eventClass, kAERestart, &finderAddr, kAutoGenerateReturnID,
                              kAnyTransactionID, &myRestart));
    ThrowIfOSErr_AC (AESend (&myRestart, &nilReply, kAENoReply + kAECanSwitchLayer + kAEAlwaysInteract,
                  kAENormalPriority, kAEDefaultTimeout, NULL, NULL));
	AEDisposeDesc (&myRestart);
	AEDisposeDesc (&finderAddr);
}

#pragma mark ----> Accessors for NVRAM values

CStr255_AC 
XPFPrefs::getInputDevice (bool forInstall)
{
	CStr255_AC result;
	if (fInputDevice) result.CopyFrom (fInputDevice->getOpenFirmwareName (getUseShortStrings (forInstall)));
	return result;
}

CStr255_AC
XPFPrefs::getOutputDevice (bool forInstall)
{
	CStr255_AC result;
	if (fOutputDevice) result.CopyFrom (fOutputDevice->getOpenFirmwareName (getUseShortStrings (forInstall)));
	return result;
}

CStr255_AC
XPFPrefs::getBootDevice (bool forInstall)
{
	CStr255_AC retVal;
	MountedVolume *bootDevice = fRebootInMacOS9 ? getMacOS9Disk () : getTargetDisk ();
	if (!bootDevice) return retVal;

	if (fRebootInMacOS9) {
		if (XPFPlatform::GetPlatform()->getIsNewWorld()) {
			retVal = bootDevice->getOpenFirmwareName (getUseShortStrings (false));
		} else {
			retVal = "/AAPL,ROM";
		}
	} else {
		if (bootDevice->getHelperDisk ()) bootDevice = bootDevice->getHelperDisk ();		
		retVal = bootDevice->getOpenFirmwareName (getUseShortStrings (forInstall));
		if (XPFPlatform::GetPlatform()->getIsNewWorld()) retVal += ",\\BootX.bootinfo";
	}
	
	return retVal;
}

CStr255_AC
XPFPrefs::getBootArgs (bool forInstall)
{
	if (fRebootInMacOS9) return CStr255_AC ("");
	if (!getTargetDisk ()) return CStr255_AC ("");	
	if (forInstall && !getInstallCD ()) return CStr255_AC ("");
	
	CStr255_AC bootArgs;
	
	if (fDebug) {
		char debugValue [32];
		sprintf (debugValue, " debug=0x%X", fDebug);
		bootArgs += debugValue;
	}
			
	if (fBootInVerboseMode) bootArgs += (" -v");
	if (fBootInSingleUserMode) bootArgs += (" -s");	
	if (fUseROMNDRV) bootArgs += (" romndrv=1");

	MountedVolume *rootDisk = forInstall ? fInstallCD : fTargetDisk;
	
	if (!fBootInVerboseMode && !rootDisk->getHasFinder ()) bootArgs += " -v";
	
	if (forInstall || getTargetDisk ()->getHelperDisk ()) {
		bootArgs += " rd=*";
		bootArgs += rootDisk->getOpenFirmwareName (getUseShortStrings (forInstall));
	}
		
	return bootArgs;
}

CStr255_AC 
XPFPrefs::getBootCommand ()
{
	if (XPFPlatform::GetPlatform()->getIsNewWorld()) {
		return (CStr255_AC) "mac-boot";
	} else {
		return fRebootInMacOS9 ? (CStr255_AC) "boot" : (CStr255_AC) "0 bootr";
	}
}

CStr255_AC
XPFPrefs::getBootFile (bool forInstall)
{
	if (fRebootInMacOS9) return CStr255_AC ("");
	if (forInstall) return CStr255_AC ("-h -i");
	if (!getTargetDisk ()) return CStr255_AC ("");
	if (getTargetDisk ()->getHelperDisk ()) return CStr255_AC ("-h");
	return CStr255_AC ("");
}

#pragma mark ----> Convenience Functions

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

#pragma mark ----> Accessors

bool 
XPFPrefs::getIsRegistered ()
{
	Handle registerVersion = GetResource ('vers', 4);
	ThrowIfResError_AC ();
	return VERS_compare (fRegisteredVersion, **registerVersion) >= 0;
}

void 
XPFPrefs::setIsRegistered (bool val)
{
	UInt32 newVal;
	if (val) {
		Handle registerVersion = GetResource ('vers', 4);
		ThrowIfResError_AC ();
		newVal = **registerVersion;
	} else {
		newVal = 0;
	}
	
	if (newVal != fRegisteredVersion) {
		fRegisteredVersion = newVal;
		Changed (cSetIsRegistered, &val);
	}
}

void
XPFPrefs::implementShowHelpTags ()
{
#ifdef __MACH__
	HMSetHelpTagsDisplayed (fShowHelpTags);
#else
	HMSetBalloons (fShowHelpTags);
#endif
}

void
XPFPrefs::setShowHelpTags (bool val, bool callChanged)
{
	if (val != fShowHelpTags) {
		fShowHelpTags = val;
		implementShowHelpTags ();
		if (callChanged) Changed (cSetShowHelpTags, &val);
	}
}

void 
XPFPrefs::setInstallCD (MountedVolume *theDisk, bool callChanged)
{
	if (fInstallCD != theDisk) {
		fInstallCD = theDisk;
		if (callChanged) Changed (cSetInstallCD, fInstallCD);
		if (fInstallCD == NULL) {
			// Switch the target back to something bootable if it wasn't
			if (fTargetDisk && (fTargetDisk->getBootStatus () != kStatusOK)) {
				setTargetDisk (MountedVolume::GetDefaultRootDisk (), callChanged);
			}
		} else {
			// If we had no target, now set it to something that can be installed to
			if (!fTargetDisk) setTargetDisk (MountedVolume::GetDefaultInstallTarget (), callChanged);
		}
	}
}

void
XPFPrefs::setInputDevice (XPFIODevice *val, bool callChanged) {
	if (fInputDevice != val) {
		fInputDevice = val;
		if (callChanged) {
			Changed (cSetInputDevice, val);
			if (val == NULL) setAutoBoot (true);
		}
	}
}

void
XPFPrefs::setOutputDevice (XPFIODevice *val, bool callChanged) {
	if (fOutputDevice != val) {
		fOutputDevice = val;
		if (callChanged) {
			Changed (cSetOutputDevice, val);
			if (val == NULL) setAutoBoot (true);
		}
	}
}

#define ACCESSOR(method,type)										\
	void XPFPrefs::set##method (type val, bool callChanged) {		\
		if (f##method != val) {										\
			f##method = val;										\
			if (callChanged) Changed (cSet##method, &val);			\
		}															\
	}									
	
#define POINTER_ACCESSOR(method,type)								\
	void XPFPrefs::set##method (type *val, bool callChanged) {		\
		if (f##method != val) {										\
			f##method = val;										\
			if (callChanged) Changed (cSet##method, val);			\
		}															\
	}		
	
#define DEBUG_ACCESSORS(methodName)												\
	void																		\
	XPFPrefs::set##methodName (bool val, bool callChanged)						\
	{																			\
		if ((fDebug & k##methodName) != (val ? k##methodName : 0)) {			\
			if (val) fDebug |= k##methodName; else fDebug &= ~k##methodName;	\
			if (callChanged) Changed (cSet##methodName, &val);					\
		}																		\
	}																			\
																				\
	bool																		\
	XPFPrefs::get##methodName ()												\
	{																			\
		return (fDebug & k##methodName) ? true : false;							\
	}
							
POINTER_ACCESSOR (TargetDisk, MountedVolume)
POINTER_ACCESSOR (MacOS9Disk, MountedVolume)
	
ACCESSOR (UseShortStrings, bool)
ACCESSOR (UseShortStringsForInstall, bool)
ACCESSOR (BootInVerboseMode, bool)
ACCESSOR (BootInSingleUserMode, bool)
ACCESSOR (AutoBoot, bool)
ACCESSOR (RebootInMacOS9, bool)
ACCESSOR (EnableCacheEarly, bool)
ACCESSOR (UsePatchedRagePro, bool)
ACCESSOR (Throttle, unsigned)
ACCESSOR (UseROMNDRV, bool)

DEBUG_ACCESSORS (DebugBreakpoint)
DEBUG_ACCESSORS (DebugPrintf)
DEBUG_ACCESSORS (DebugNMI)
DEBUG_ACCESSORS (DebugKprintf)
DEBUG_ACCESSORS (DebugDDB)
DEBUG_ACCESSORS (DebugSyslog)
DEBUG_ACCESSORS (DebugARP)
DEBUG_ACCESSORS (DebugOldGDB)
DEBUG_ACCESSORS (DebugPanicText)
