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
#include "XPFSynchronizeCommand.h"
#include "XPFAuthorization.h"
#include "XPFErrors.h"
#include "HFSPlusArchive.h"
#include "XPFUpdate.h"
#include "XPFUpdateWindow.h"

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
		fAutoBoot (true),
		fBootInSingleUserMode (false),
		fBootInVerboseMode (false),
		fThrottle (0),
		fTargetDisk (NULL),
		fInstallCD (NULL),
		fInputDevice (NULL),
		fOutputDevice (NULL),
		fOptionsWindow (NULL),
		fDebug (0),
		fRebootInMacOS9 (false),
		fUseROMNDRV (false),
		fUseShortStrings (true),
		fUseShortStringsForInstall (true),
		fTooBigForNVRAM (false),
		fTooBigForNVRAMForInstall (false),
		fForceAskSave (false),
		fRestartOnClose (false)
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
	// First, do the superclass Close, so that we save if we need to save etc.
	Inherited::Close ();
	
	try {	
		// Now, we check NVRAM and do whatever copying is necessary
		XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
		nvram->readFromNVRAM ();
		
		char *bootCommand = nvram->getStringValue ("boot-command");
		char *bootDevice = nvram->getStringValue ("boot-device");
		
		// If we're rebooting into Mac OS 9, then no copying necessary
		if (!strstr (bootDevice, "/AAPL,ROM")) {	
			MountedVolume *bootDisk = MountedVolume::WithOpenFirmwarePath (bootDevice);
			MountedVolume *rootDisk = bootDisk;
			
			char *rdString = strstr (bootCommand, "rd=*");
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
			}
		}
	}
	
	catch (CException_AC &ex) {
		ErrorAlert (ex.GetError (), ex.GetExceptionMessage ());
	}
		
#ifdef __MACH__
	restartStartupItem ();
#endif

	if (fRestartOnClose) tellFinderToRestart ();

	if (!gApplication->GetDone ()) gApplication->DoMenuCommand (cQuit);
}

// We check for updates when we read prefs from NVRAM (i.e. at launch time) and
// when we write prefs to NVRAM (i.e. at quitting time).

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
		} else {
			PerformCommand (TH_new XPFUpdateCommand (&update));
		}	
	}
}

short
XPFPrefs::PoseConfirmDialog (bool forInstall, bool quitting)
{
	short retVal = kStdCancelItemIndex;
	
	MountedVolume *bootDisk = forInstall ? fInstallCD : fTargetDisk;
	
	CStr255_AC version (fRebootInMacOS9 ? "Mac OS 9" : "Mac OS X ");
	if (!fRebootInMacOS9) {
		if (!bootDisk) return kNoButton;
		if (!forInstall && (bootDisk->getBootStatus () != kStatusOK)) return kNoButton;
		version += bootDisk->getMacOSXVersion ();
	}	
	
	MAParamText ("$OS$", version);
	MAParamText ("$VOLUME$", fRebootInMacOS9 ? "a Mac OS 9 volume" : bootDisk->getVolumeName ());
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
	bool useInstallDialog = (fInstallCD != NULL) && (fTargetDisk->getBootStatus () != kStatusOK);
	
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
	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();

	char *bootCommand = nvram->getStringValue ("boot-command");
	char *bootDevice = nvram->getStringValue ("boot-device");
	char *bootFile = nvram->getStringValue ("boot-file");
	char *inputDevice = nvram->getStringValue ("input-device");
	char *outputDevice = nvram->getStringValue ("output-device");
	char *nvramrc = nvram->getStringValue ("nvramrc");

	// Check to see whether NVRAM is set to boot back into Mac OS 9
	if (strstr (bootDevice, "/AAPL,ROM")) {
#ifdef __MACH__
		// If we're in Mac OS X now, we set the prefs to reboot in 9. We don't need to update the change
		// count because we don't need to rewrite NVRAM if the user just quits.
		setRebootInMacOS9 (true, false);
#else
		// If we're in Mac OS 9, we leave the pref to reboot in X, since that is probably what the user
		// wants. But we force asking save if we have a target disk already
		if (fTargetDisk) fForceAskSave = true;
#endif
		// In either case, we bail out here, since the rest of the NVRAM settings won't be interesting
		// if we're booting in 9.
		return;	
	}
	
	// For most settings, we use what is in our preferences rather than what is in NVRAM.
	// But, we ask about saving those preferences if NVRAM is different 
	
	if (fAutoBoot != nvram->getBooleanValue ("auto-boot?")) fForceAskSave = true;;	
	if (fBootInSingleUserMode != (strstr (bootCommand, " -s") != 0)) fForceAskSave = true;
	if (fBootInVerboseMode != (strstr (bootCommand, " -v") != 0)) fForceAskSave = true;
	
	// we set -c if the user does *not* want to enable the cache early
	// If no -c, we don't assume anything
	if (fEnableCacheEarly != (strstr (bootCommand, " -c") == 0)) fForceAskSave = true;
	
	char *debugString = strstr (bootCommand, "debug=");
	UInt32 debug = 0;
	if (debugString) {
		debugString += strlen ("debug=");
		debug = strtoul (debugString, NULL, 0);
	}
	if (fDebug != debug) fForceAskSave = true;
	
	char *romndrvstring = strstr (bootCommand, "romndrv=");
	UInt32 romndrv = 0;
	if (romndrvstring) {
		romndrvstring += strlen ("romndrv=");
		romndrv = strtoul (romndrvstring, NULL, 0);
	}
	if (fUseROMNDRV != romndrv) fForceAskSave = true;
	
	// For the target disk and helper disk, we use what is in NVRAM rather than preferences file
	// But, once again, we ask to save preferences if they are different
	
	MountedVolume *bootDisk = MountedVolume::WithOpenFirmwarePath (bootDevice);
	MountedVolume *rootDisk = bootDisk;
	
	char *rdString = strstr (bootCommand, "rd=*");
	if (rdString) {
		char rootPath[256];
		strcpy (rootPath, rdString + strlen ("rd=*"));
		char *pos = strchr (rootPath, ' ');
		if (pos) *pos = 0;
		
		rootDisk = MountedVolume::WithOpenFirmwarePath (rootPath);
		if (!rootDisk) rootDisk = bootDisk;
	}	
	
	if (rootDisk && (rootDisk->getBootStatus () == kStatusOK)) {
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
		// We do it here because we get here where we have a MountedVolume for the current root
		checkForUpdates (false);
	} else {
		// If we can't set the target disk to the current root, then we'd ask
		// user about saving (since our pref won't match what's in NVRAM).
		fForceAskSave = true;
	}
	
	// input device and output device
	if (getInputDevice (false) != inputDevice) fForceAskSave = true;
	if (getOutputDevice (false) != outputDevice) fForceAskSave = true;
	
	// throttle
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
	gLogFile << fThrottle << " " << throttleVal << endl_AC;
	if (fThrottle != throttleVal) fForceAskSave = true;
}

void 
XPFPrefs::installXPFPartitionInfo ()
{
#ifdef __MACH__
	FSRef tmpDirectory;
	OSErr err = FSFindFolder (kOnAppropriateDisk, kTemporaryFolderType, true, &tmpDirectory);
	if (err == noErr) {
		CResourceStream_AC stream ('hfsB', 300);
		HFSPlusArchive archive (&stream, NULL, this);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (&tmpDirectory));
		
		char path[256];
		err = FSRefMakePath (&tmpDirectory, (UInt8 *) path, 256);
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
	
	fTargetDisk = MountedVolume::GetDefaultRootDisk ();
	fInstallCD = MountedVolume::GetDefaultInstallerDisk ();
	if (fInstallCD && !fTargetDisk) fTargetDisk = MountedVolume::GetDefaultInstallTarget ();

	checkStringLength ();	
	
	suspendStartupItem ();

#ifdef __MACH__
	AddBehavior (new XPFPollVolumesBehavior);
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
			break;
			
		case cDeleteMountedVolume:
			if (volume == fTargetDisk) setTargetDisk (MountedVolume::GetDefaultRootDisk ());
			if (volume == fInstallCD) setInstallCD (MountedVolume::GetDefaultInstallerDisk ());
			break;
		
		case cSetHelperDisk:
			if (volume == fTargetDisk) {
				Changed (cSetHelperDiskForTarget, volume);
			} else {
				SetChangeCount (GetChangeCount () + 1);
			}
			break;
			
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

void
XPFPrefs::RegainControl ()
{
	Inherited::RegainControl ();
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
		if (rootDisk && (rootDisk->getBootStatus () == kStatusOK)) setTargetDisk (rootDisk, false);
			
		UInt32 helpers;
		fileStream >> helpers;
		
		while (helpers > 0) {
			helpers--;

			fileStream.ReadBytes (&volInfo, sizeof (volInfo));
			MountedVolume *vol = MountedVolume::WithInfo (&volInfo);
			
			if (vol) {
				vol->readHelperFromStream (&fileStream);
			} else {
				// skip over the helper
				fileStream.ReadBytes (&volInfo, sizeof (volInfo));
			}
		}
		
		fileStream >> fUseROMNDRV;
	}
	
	catch (...) {
	}
	
	// Now, get prefs from NVRAM to see what's changed (if anything)
	getPrefsFromNVRAM ();
	
	// Then patch the NVRAM so that checkStringLength () works properly
	XPFPlatform *platform = ((XPFApplication *) gApplication)->getPlatform ();
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

	if (!fRebootInMacOS9) checkForUpdates (forInstall);

	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
	
	CChar255_AC bootDevice (getBootDevice (forInstall));
	CChar255_AC bootFile (getBootFile (forInstall));
	CChar255_AC bootCommand (getBootCommand (forInstall));
	CChar255_AC inputDevice (getInputDevice (forInstall));
	CChar255_AC outputDevice (getOutputDevice (forInstall));
	
	nvram->setStringValue ("boot-device", bootDevice);
	nvram->setStringValue ("boot-file", bootFile);
	nvram->setStringValue ("boot-command", bootCommand);
	nvram->setBooleanValue ("auto-boot?", fAutoBoot);	
	nvram->setStringValue ("input-device", inputDevice);
	nvram->setStringValue ("output-device", outputDevice);
	
	XPFPlatform *platform = ((XPFApplication *) gApplication)->getPlatform ();
	if (!platform->getCanPatchNVRAM ()) ThrowException_AC (kErrorWritingNVRAM, 0);
		
	// adjust throttle
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
	
	#if qLogging
		gLogFile << "Restarting ..." << endl_AC;
		gLogFile << "boot-device: " << bootDevice << endl_AC;
		gLogFile << "boot-command: " << bootCommand << endl_AC;
		gLogFile << "input-device: " << inputDevice << endl_AC;
		gLogFile << "output-device: " << outputDevice << endl_AC;
	#endif
	
	OSErr err = nvram->writeToNVRAM ();
	if (err != noErr) ThrowException_AC (kErrorWritingNVRAM, 0);
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
	
	FSVolumeInfo info = fTargetDisk->getVolumeInfo ();
	fileStream.WriteBytes (&info, sizeof (info));
	
	// Now, we need to store the choices for Helper Disk (which are per-volume). 
	
	UInt32 helpers = 0;

	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		if (iter->getHelperDisk ()) helpers++;
	}

	fileStream << helpers;

	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		if (iter->getHelperDisk ()) {
			info = iter->getVolumeInfo ();
			fileStream.WriteBytes (&info, sizeof (info));
			info = iter->getHelperDisk ()->getVolumeInfo ();
			fileStream.WriteBytes (&info, sizeof (info));
		}
	}	
	
	// And the ROMNDRV
	
	fileStream << fUseROMNDRV;
}

void 
XPFPrefs::DoMakeViews (bool forPrinting)
{
	((XPFApplication *) gApplication)->CloseSplashWindow ();
	
	Inherited::DoMakeViews (forPrinting);
	
	// Not the most elegant place, but we need to do it after reading from NVRAM,
	// and before Close ()
	if (fForceAskSave) SetChangeCount (GetChangeCount () + 1);
}

void 
XPFPrefs::DoSetupMenus ()
{
	Inherited::DoSetupMenus ();
	
	Enable (cInstallBootX, true);
	Enable (cInstallExtensions, true);
	Enable (cInstallStartupItem, true);
	Enable (cShowOptionsWindow, true);
	Enable (cRecopyHelperFiles, fTargetDisk && fTargetDisk->getHelperDisk () != NULL);
	Enable (cUninstall, fTargetDisk && fTargetDisk->getIsWriteable ());
	Enable (cBlessMacOS9SystemFolder, fTargetDisk && fTargetDisk->getMacOS9SystemFolderNodeID ()); 
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
	// Figure out the lengths

	unsigned nvramPatchLength = strlen (XPFNVRAMSettings::GetSettings ()->getStringValue ("nvramrc"));

	unsigned len = getBootCommand (false).Length () + 
				   getBootFile (false).Length () +
				   getBootDevice (false).Length () +
				   getInputDevice (false).Length () +
				   getOutputDevice (false).Length () +
				   nvramPatchLength;

	fTooBigForNVRAM = (len >= kOFStringCapacity);

	len = getBootCommand (true).Length () + 
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
	if (fRebootInMacOS9) return CStr255_AC ("/AAPL,ROM");

	MountedVolume *bootDevice = getTargetDisk ();
	if (!bootDevice) return CStr255_AC ("");
	
	if (bootDevice->getHelperDisk ()) bootDevice = bootDevice->getHelperDisk ();		
	
	return bootDevice->getOpenFirmwareName (getUseShortStrings (forInstall));
}

CStr255_AC 
XPFPrefs::getBootCommandBase ()
{
	if (!getTargetDisk ()) return CStr255_AC ("");
	
	CStr255_AC bootCommand ("0 bootr");
	
	if (fDebug) {
		char debugValue [32];
		sprintf (debugValue, " debug=0x%X", fDebug);
		bootCommand += debugValue;
	}
			
	if (fBootInVerboseMode) bootCommand += (" -v");
	if (fBootInSingleUserMode) bootCommand += (" -s");
	
	// We set -c if the user does *not* want to enable the cache early
	if (!fEnableCacheEarly) bootCommand += (" -c");
	if (fUseROMNDRV) bootCommand += (" romndrv=1");
	
	return bootCommand;
}

CStr255_AC 
XPFPrefs::getBootCommand (bool forInstall)
{
	if (fRebootInMacOS9) return CStr255_AC ("boot");

	if (!getTargetDisk ()) return CStr255_AC ("");	
	if (forInstall && !getInstallCD ()) return CStr255_AC ("");

	MountedVolume *rootDisk = forInstall ? fInstallCD : fTargetDisk;

	CStr255_AC bootCommand = getBootCommandBase ();
	
	if (!fBootInVerboseMode && !rootDisk->getHasFinder ()) bootCommand += " -v";
	
	if (forInstall || getTargetDisk ()->getHelperDisk ()) {
		bootCommand += " rd=*";
		bootCommand += rootDisk->getOpenFirmwareName (getUseShortStrings (forInstall));
	}
		
	return bootCommand;
}

CStr255_AC
XPFPrefs::getBootFile (bool forInstall)
{
	if (fRebootInMacOS9) return CStr255_AC ("");
	if (forInstall) return CStr255_AC ("-h");
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
	
ACCESSOR (UseShortStrings, bool)
ACCESSOR (UseShortStringsForInstall, bool)
ACCESSOR (BootInVerboseMode, bool)
ACCESSOR (BootInSingleUserMode, bool)
ACCESSOR (AutoBoot, bool)
ACCESSOR (RebootInMacOS9, bool)
ACCESSOR (EnableCacheEarly, bool)
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
