/*

Copyright (c) 2003
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

#include "XPFRestartCommand.h"

#include "MountedVolume.h"
#include "XPFPrefs.h"
#include "NVRAM.h"
#include "FSRefCopying.h"
#include "XPFLog.h"
#include "XPFApplication.h"
#include "XPFErrors.h"
#include "stdio.h"
#include "XPFProgressWindow.h"
#include "XPFStrings.h"
#include "XPFAuthorization.h"
#include "XPFFSRef.h"

#define Inherited XPFThreadedCommand

XPFRestartCommand::XPFRestartCommand (XPFPrefs *prefs)
	: XPFThreadedCommand (prefs)
{
	fBootDevice = prefs->getBootDevice ();
	fBootCommand = prefs->getBootCommand ();
	fBootFile = prefs->getBootFile ();
	fInputDevice = prefs->getInputDevice ();
	fOutputDevice = prefs->getOutputDevice ();
	fUseShortStrings = prefs->getUseShortStrings ();
	fAutoBoot = prefs->getAutoBoot ();
	fThrottle = prefs->getThrottle ();
}

void
XPFRestartCommand::tellFinderToRestart ()
{
	if (fDebugOptions & kDisableRestart) return;

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

void
XPFRestartCommand::adjustThrottle (XPFNVRAMSettings *nvram) {
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

void 
XPFRestartCommand::DoItThreaded ()
{
	if (fProgressMin == 0) {
		setDescription (CStr255_AC (kXPFStringsResource, kRestarting));
		fProgressMax = 1000;	
	}
	
	float scale = (float) (fProgressMax - fProgressMin) / (float) fProgressMax;
	unsigned progbase = fProgressMin;

	if (!fAutoBoot) {
		// Make sure that input-device and output-device are specified if not auto-booting
		if (!fInputDevice.Length () || !fOutputDevice.Length ()) {
			ThrowException_AC (kMustSpecifyIODevices, 0);
		}
	}

	MountedVolume *rootDisk = fTargetDisk;
	MountedVolume *bootDisk;
	if (fHelperDisk) {
		bootDisk = fHelperDisk;
	} else {
		bootDisk = fTargetDisk;
	}
	
	fProgressMax = progbase + scale * 50;
	
	if (rootDisk->getIsWriteable ()) {
		if (!rootDisk->hasCurrentExtensions ()) {
			installExtensionsWithRootDirectory (rootDisk->getRootDirectory ());
		}
				
		fProgressMin = fProgressMax;
		fProgressMax = progbase + scale * 100;

		if (!rootDisk->hasCurrentStartupItems ()) {
			installStartupItemWithRootDirectory (rootDisk->getRootDirectory ());
		}
	}
	
	fProgressWindow->setProgressValue (progbase + scale * 100);
	
	if (bootDisk->getIsWriteable ()) {
		setCopyingFile ("\pBootX");
		bootDisk->installBootXIfNecessary ();
	}
	
	fProgressWindow->setProgressValue (progbase + scale * 150);

	// Now, see if we need to copy stuff from the root-disk to the boot-disk
	if ((rootDisk != bootDisk) && !(fDebugOptions & kDisableCopyToHelper)) {	
			
		// Get the .XPostFacto directory
		FSRef helperDir;
		ThrowIfOSErr_AC (XPFFSRef::getOrCreateXPFDirectory (bootDisk->getRootDirectory (), &helperDir));

		// Now, get the directory which corresponds to the root disk
		char rootName[255];
		rootDisk->getShortOpenFirmwareName ().CopyTo (rootName);
		rootName[strlen(rootName) + 1] = 0; // extra termination byte
				
		// And write out each directory
		char *end;
		char *pos = rootName;
		while (*pos) {
			while (*pos == '/') pos++;
			end = pos;		
			while ((*end != 0) && (*end != '/')) {
				if (*end == ':') *end = ';';
				end++;
			}
			*end = 0;
			ThrowIfOSErr_AC (XPFFSRef::getOrCreateDirectory (&helperDir, pos, 0755, &helperDir));
			pos = end + 1;
		}
			
		// Now, copy the mach_kernel file
		// Note the FSRefFileCopy will skip the copy if the files are the same
		FSRef kernelOnRoot;
		ThrowIfOSErr_AC (XPFFSRef::getKernelFSRef (rootDisk->getRootDirectory (), &kernelOnRoot));
		setCopyingFile ("\pmach_kernel");

		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (FSRefFileCopy (&kernelOnRoot, &helperDir, NULL, NULL, 0, false));

		fProgressWindow->setProgressValue (progbase + scale * 200);		
		
		// Copy the Extensions and Extensions.mkext
		// Note that the FSRefCopy* routines skip copies that aren't necessary
		FSRef helperSystemLibraryRef;
		FSRef rootSystemLibraryExtensionsRef, rootSystemLibraryExtensionsCacheRef;

		ThrowIfOSErr_AC (XPFFSRef::getOrCreateSystemLibraryDirectory (&helperDir, &helperSystemLibraryRef));
	
		// Check for the Extensions.mkext.
		OSErr rootErr = XPFFSRef::getExtensionsCacheFSRef (rootDisk->getRootDirectory (), &rootSystemLibraryExtensionsCacheRef);

		if (rootErr == fnfErr) {
			// It doesn't exist on the root. So make sure it doesn't exist on the helper.
			FSRef helperExtensionsCacheRef;
			OSErr helperErr = XPFFSRef::getExtensionsCacheFSRef (&helperDir, &helperExtensionsCacheRef);
			if (helperErr == noErr) ThrowIfOSErr_AC (FSDeleteObject (&helperExtensionsCacheRef));
		} else {
			ThrowIfOSErr_AC (rootErr);
			setCopyingFile ("\pExtensions.mkext");
			ThrowIfOSErr_AC (FSRefFileCopy (&rootSystemLibraryExtensionsCacheRef, &helperSystemLibraryRef, NULL, NULL, 0, false));			
		}	
		
		fProgressMin = progbase + scale * 250;
		fProgressMax = progbase + scale * 900;		

		// Check for the extensions directory. It should exist :-)
		ThrowIfOSErr_AC (XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (rootDisk->getRootDirectory (), &rootSystemLibraryExtensionsRef));
		ThrowIfOSErr_AC (FSRefFilteredDirectoryCopy (&rootSystemLibraryExtensionsRef, &helperSystemLibraryRef, NULL, NULL, 0, true, 
								NULL, CopyFilterGlue, this));

		fProgressMin = fProgressMax;
		fProgressMax = progbase + scale * 950;
		
		// If the root disk was not writeable, then we need to install the extensions in the secondary location
		// on the helper, so that BootX will pick them up
		if (!rootDisk->getIsWriteable ()) {
			installSecondaryExtensionsWithRootDirectory (&helperDir);
		}
	}		
	
	fProgressWindow->setProgressValue (progbase + scale * 950);	

	XPFNVRAMSettings *nvram = XPFNVRAMSettings::GetSettings ();
	
	CChar255_AC bootDevice (fBootDevice);
	CChar255_AC bootFile (fBootFile);
	CChar255_AC bootCommand (fBootCommand);
	CChar255_AC inputDevice (fInputDevice);
	CChar255_AC outputDevice (fOutputDevice);
	
	nvram->setStringValue ("boot-device", bootDevice);
	nvram->setStringValue ("boot-file", bootFile);
	nvram->setStringValue ("boot-command", bootCommand);
	nvram->setBooleanValue ("auto-boot?", fAutoBoot);	
	nvram->setStringValue ("input-device", inputDevice);
	nvram->setStringValue ("output-device", outputDevice);
	
	if (bootDisk->getIsWriteable ()) adjustThrottle (nvram);
	
	XPFPlatform *platform = ((XPFApplication *) gApplication)->getPlatform ();
	if (platform->getCanPatchNVRAM ()) {
		platform->patchNVRAM ();
	} else {
		ThrowException_AC (kErrorWritingNVRAM, 0);
	}
		
	#if qLogging
		gLogFile << "Restarting ..." << endl_AC;
		gLogFile << "Boot-device: " << bootDevice << endl_AC;
		gLogFile << "Boot-command: " << bootCommand << endl_AC;
		gLogFile << "input-device: " << inputDevice << endl_AC;
		gLogFile << "output-device: " << outputDevice << endl_AC;
	#endif
	
	fYielder.YieldIfEvent ();
	
	if (!(fDebugOptions & kDisableNVRAMWriting) && !fRunner->ShutdownRequested ()) {
		if (nvram->writeToNVRAM () == noErr) {	
			tellFinderToRestart ();
		} else {
			ThrowException_AC (kErrorWritingNVRAM, 0);
		}
	}
	
	fProgressWindow->setFinished ();
}
