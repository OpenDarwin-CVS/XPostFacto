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

#include "XPFVersion.h"

#include "XPFApplication.h"
#include "XPFWindow.h"
#include "XPFAboutBox.h"
#include "XPFFatalErrorWindow.h"

#include "UMenuMgr.h"

#include "NVRAM.h"
#include "MountedVolume.h"

#include "FileCopy.h"
#include "MoreFilesExtras.h"
#include "DirectoryCopy.h"
#include "FSRefCopying.h"
#include "HFSPlusArchive.h"
#include "MoreFiles.h"
#include "XPFLog.h"
#include "XPFErrors.h"
#include "OFAliases.h"
#include "L2Cache.h"
#include "XPFPrefs.h"
#include "XPFStrings.h"

#include "XPFNameRegistry.h"

// Toolbox

#include <Sound.h>

#include <stdio.h>
#include <stdlib.h>

#include "Configuration_AC.h"
#include <Gestalt.h>
#include "FSRefCopying.h"
#include <Files.h>
#include <UnicodeConverter.h>
#include <CodeFragments.h>

#include <InternetConfig.h>

#include "ThreadUtilities_AC.h"

#include "XPostFacto.h"

enum {
  gestaltCPU7400 = 0x010C				
};

//----------------------------------------------------------------------------------------
// Constants:

static pascal Boolean 
copyFilterGlue (const FSRef *src)
{
	return ((XPFApplication *) gApplication)->copyFilter (src);
}
				
//========================================================================================
// CLASS XPFApplication
//========================================================================================

#undef Inherited
#define Inherited TApplication

//----------------------------------------------------------------------------------------
// XPFApplication constructor
//----------------------------------------------------------------------------------------

XPFApplication::XPFApplication() :
TApplication('usu1', kSignature)
{
	fMainWindow = NULL;
	fAboutBox = NULL;
	fWantsNavigationServices = true;
							
	fCopyInProgress = false;
	
	long cpu;
	OSErr err = Gestalt (gestaltNativeCPUfamily, &cpu);
	if (err == noErr) {
		fHasL2Cache = (cpu == gestaltCPU750) || (cpu == gestaltCPU7400);
	} else {
		fHasL2Cache = false;
	}
	
	REGISTER_CLASS_AC (XPFWindow);
	REGISTER_CLASS_AC (XPFAboutBox);
	REGISTER_CLASS_AC (XPFFatalErrorWindow);

	fPrefs = new XPFPrefs;
	
}

//----------------------------------------------------------------------------------------
// XPFApplication destructor
//----------------------------------------------------------------------------------------


XPFApplication::~XPFApplication()
{
	#if qLogging
		gLogFile << endl_AC << "============" << endl_AC;
	#endif

	if (fPrefs) delete fPrefs;
	if (fAboutBox) delete fAboutBox;
	if (fMainWindow) delete fMainWindow;	
}


void 
XPFApplication::DoAboutBox()
{
	if (!fAboutBox) {
		fAboutBox = (XPFAboutBox *) TViewServer::fgViewServer->NewTemplateWindow (1005, NULL);
		fAboutBox->Open ();
	} else {
		fAboutBox->Show (true, false);
		fAboutBox->Select ();
	}
}

void
XPFApplication::launchURL (CStr255_AC theURL)
{
	long start = 0, end = theURL[0];
	ICInstance inst;
	ThrowIfOSErr_AC (ICStart (&inst, 'usuX'));
	ThrowIfOSErr_AC (ICLaunchURL (inst, "\p", &theURL[1], theURL[0], &start, &end));
	ICStop (inst);
}

void
XPFApplication::DoShowHelpFile ()
{
	// First, we write out the help text to a temporary file
	Handle helpText = GetResource ('TEXT', kHelpID);
	if (helpText) {
		DetachResource (helpText);
		CFile_AC helpFile ('TEXT', '????', true);
		CFSSpec_AC helpSpec;
		ThrowIfOSErr_AC (FindFolder (kOnAppropriateDisk, kTemporaryFolderType, kCreateFolder, &helpSpec.vRefNum, &helpSpec.parID));
		helpSpec.SetName ("XPostFacto Help.html");
		helpFile.Specify (helpSpec);
		helpFile.DeleteCFile ();
		helpFile.CreateAndOpen ();
		HLock (helpText);
		long helpTextSize = GetHandleSize (helpText);
		OSErr err = helpFile.WriteData (*helpText, helpTextSize);
		HUnlock (helpText);
		ThrowIfOSErr_AC (err);
		ThrowIfOSErr_AC (helpFile.CloseFile ());
		DisposeIfHandle_AC (helpText);
		
		// Now, we open the file with a browser
		FSRef helpFileRef;
		ThrowIfOSErr_AC (FSpMakeFSRef (&helpSpec, &helpFileRef));
		CFURLRef urlRef = CFURLCreateFromFSRef (NULL, &helpFileRef);
		CFStringRef urlStringRef = CFURLGetString (urlRef);
		Str255 urlString;
		CFStringGetPascalString (urlStringRef, urlString, 255, CFStringGetSystemEncoding());
		CFRelease (urlRef);

		launchURL (urlString);		
	}
}

void 
XPFApplication::reportFatalError (CStr255_AC error)
{
	XPFFatalErrorWindow *window = (XPFFatalErrorWindow *) TViewServer::fgViewServer->NewTemplateWindow(1006, NULL);
	window->Open ();
	window->SetText (error);
}

void
XPFApplication::initializeThrottleMenu ()
{
	char label[8];
	for (int x = 1; x <= kNumThrottleOptions; x++) {
		sprintf (label, "%d", x);
		TMenuBarManager::fgMenuBarManager->AddMenuItem (label, mThrottle, 999, cThrottleBase + x);
	}
}

TDocument* 
XPFApplication::OpenNew(CommandNumber itsCommandNumber)
{
	#pragma unused (itsCommandNumber)

	initializeThrottleMenu ();
		
	// Check for a few things
	
	#if qLogging
		gLogFile << "XPostFacto Version " << kXPFVersion << endl_AC;
	#endif
	
	CStr255_AC message;
	
	UInt32 version = 0;
	
	Gestalt (gestaltSystemVersion, (SInt32 *) &version);
	
	if (version < 0x0900) {
		GetIndString (message, kXPFStringsResource, kRequiresOS9);
		reportFatalError (message);
		return NULL;	
	}
	
	if (version >= 0x01000) {
		GetIndString (message, kXPFStringsResource, kNoMacOSXYet);
		reportFatalError (message);
		return NULL;	
	}
	
	if (!NVRAMVariables::GetVariables()->getCanWritePatches()) {
		GetIndString (message, kXPFStringsResource, kNoNVRAMPatches);
		message += NVRAMVariables::GetVariables()->getCompatible();
		reportFatalError (message);
		return NULL;
	}
								
	fPrefs->Initialize ();
	
	fMainWindow = (XPFWindow *) TViewServer::fgViewServer->NewTemplateWindow(1004, NULL);		
	fMainWindow->Open();
			
	return NULL;

}

void 
XPFApplication::HandleDiskEvent(TToolboxEvent* event)
{
	TApplication::HandleDiskEvent (event);
	fPrefs->Initialize ();
}

void 
XPFApplication::RegainControl(bool checkClipboard)
{
	TApplication::RegainControl (checkClipboard);
	fPrefs->Initialize ();
}

TDocument* 
XPFApplication::OpenOld(CommandNumber itsOpenCommand, CList_AC* aFileList)
{
	#pragma unused (itsOpenCommand, aFileList)
	return OpenNew (itsOpenCommand);
}

void 
XPFApplication::InstallHelpMenuItems()
{
	CStr255_AC theMenuName;
	GetIndString (theMenuName, kXPFStringsResource, kXPostFactoHelpMenu);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (theMenuName, cShowHelpFile);
	GetIndString (theMenuName, kXPFStringsResource, kXPostFactoOnlineHelpMenu);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (theMenuName, cShowOnlineHelpFile);
	GetIndString (theMenuName, kXPFStringsResource, kXPostFactoSourceCodeMenu);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (theMenuName, cShowSourceCode);
}

void 
XPFApplication::DoMenuCommand(CommandNumber aCommandNumber) // Override 
{
	switch (aCommandNumber) 
	{			
		case cQuit:
			TApplication::DoMenuCommand (aCommandNumber);
			break;
			
		case cShowHelpFile:
			DoShowHelpFile ();
			break;
			
		case cShowOnlineHelpFile:
			launchURL ("\phttp://eshop.macsales.com/OSXCenter/XPostFacto/framework.cfm?page=XPostFacto.html");
			break;
			
		case cShowSourceCode:
			launchURL ("\phttp://www.opendarwin.org/cgi-bin/cvsweb.cgi/proj/XPostFacto/");
			break;

		case cToggleVerboseMode:
			fPrefs->setBootInVerboseMode (!fPrefs->getBootInVerboseMode ());
			break;
						
		case cToggleSingleUserMode:
			fPrefs->setBootInSingleUserMode (!fPrefs->getBootInSingleUserMode ());
			break;
			
		case cReinstallBootX:
			fPrefs->setReinstallBootX (!fPrefs->getReinstallBootX ());
			break;
			
		case cReinstallExtensions:
			fPrefs->setReinstallExtensions (!fPrefs->getReinstallExtensions ());
			break;
			
		case cToggleAutoBoot:
			fPrefs->setAutoBoot (!fPrefs->getAutoBoot ());
			break;
			
		case cSetupL2Cache:
			fPrefs->setSetupL2Cache (!fPrefs->getSetupL2Cache ());
			break;
			
		case cSetOutputDeviceNone:
			fPrefs->setOutputDeviceIndex (0);
			break;
	
		case cSetInputDeviceNone:
			fPrefs->setInputDeviceIndex (0);
			break;
			
		case cSetThrottleNone:
			fPrefs->setThrottle (0);
			break;
			
		case cToggleDebugBreakpoint:
			fPrefs->setDebugBreakpoint (!fPrefs->getDebugBreakpoint ());
			break;
			
		case cToggleDebugPrint:
			fPrefs->setDebugPrintf (!fPrefs->getDebugPrintf ());
			break;
			
		case cToggleDebugNMI:
			fPrefs->setDebugNMI (!fPrefs->getDebugNMI ());
			break;
			
		case cToggleDebugkprintf:
			fPrefs->setDebugKprintf (!fPrefs->getDebugKprintf ());
			break;
			
		case cToggleDebugUseDDB:
			fPrefs->setDebugDDB (!fPrefs->getDebugDDB ());
			break;
			
		case cToggleDebugSystemLog:
			fPrefs->setDebugSyslog (!fPrefs->getDebugSyslog ());
			break;
			
		case cToggleDebugARP:
			fPrefs->setDebugARP (!fPrefs->getDebugARP ());
			break;
			
		case cToggleDebugOldGDB:
			fPrefs->setDebugOldGDB (!fPrefs->getDebugOldGDB ());
			break;
	
		default:
			if ((aCommandNumber >= cFirstInputDevice) && (aCommandNumber < fPrefs->getNextInputDevice ())) {
				fPrefs->setInputDeviceIndex (aCommandNumber - cFirstInputDevice + 1);
			} else if ((aCommandNumber >= cFirstOutputDevice) && (aCommandNumber < fPrefs->getNextOutputDevice ())) {
				fPrefs->setOutputDeviceIndex (aCommandNumber - cFirstOutputDevice + 1);
			} else if ((aCommandNumber > cThrottleBase) && (aCommandNumber <= cThrottleBase + kNumThrottleOptions)) {
				fPrefs->setThrottle (aCommandNumber - cThrottleBase);
			} else {
				TApplication::DoMenuCommand(aCommandNumber);
			}
			break;
	}
}

void 
XPFApplication::DoSetupMenus() 
{
	TApplication::DoSetupMenus();
	
	if (!fPrefs) return; // too early--call back later!
	
	bool notCopying = !getCopyInProgress ();
	
	EnableCheck (cToggleVerboseMode, notCopying, fPrefs->getBootInVerboseMode ());
	EnableCheck (cToggleSingleUserMode, notCopying, fPrefs->getBootInSingleUserMode ());
	EnableCheck (cReinstallBootX, notCopying, fPrefs->getReinstallBootX ());
	EnableCheck (cReinstallExtensions, notCopying, fPrefs->getReinstallExtensions ());
	EnableCheck (cToggleAutoBoot, notCopying, fPrefs->getAutoBoot ());
	EnableCheck (cSetupL2Cache, notCopying && fHasL2Cache, fPrefs->getSetupL2Cache ());
	Enable (cSelectInputDevice, notCopying);
	Enable (cSelectOutputDevice, notCopying);
	
	EnableCheck (cSetInputDeviceNone, notCopying, fPrefs->getInputDeviceIndex () == 0);
	EnableCheck (cSetOutputDeviceNone, notCopying, fPrefs->getOutputDeviceIndex () == 0);
	
	for (unsigned x = cFirstInputDevice; x < fPrefs->getNextInputDevice (); x++) {
		int index = x - cFirstInputDevice + 1;
		EnableCheck (x, notCopying, fPrefs->getInputDeviceIndex () == index);
	}
	for (unsigned x = cFirstOutputDevice; x < fPrefs->getNextOutputDevice (); x++) {
		int index = x - cFirstOutputDevice + 1;
		EnableCheck (x, notCopying, fPrefs->getOutputDeviceIndex () == index);
	}
	
	EnableCheck (cSetThrottleNone, notCopying, fPrefs->getThrottle () == 0);
	for (unsigned x = 1; x <= kNumThrottleOptions; x++) {
		int index = cThrottleBase + x;
		EnableCheck (index, notCopying, fPrefs->getThrottle () == x);
	}
	
	EnableCheck (cToggleDebugBreakpoint, notCopying, fPrefs->getDebugBreakpoint ());
	EnableCheck (cToggleDebugPrint, notCopying, fPrefs->getDebugPrintf ());
	EnableCheck (cToggleDebugNMI, notCopying, fPrefs->getDebugNMI ());
	EnableCheck (cToggleDebugkprintf, notCopying, fPrefs->getDebugKprintf ());
	EnableCheck (cToggleDebugUseDDB, notCopying, fPrefs->getDebugDDB ());
	EnableCheck (cToggleDebugSystemLog, notCopying, fPrefs->getDebugSyslog ());
	EnableCheck (cToggleDebugARP, notCopying, fPrefs->getDebugARP ());
	EnableCheck (cToggleDebugOldGDB, notCopying, fPrefs->getDebugOldGDB ());
	
	Enable (cShowHelpFile, true);
	Enable (cShowOnlineHelpFile, true);
	Enable (cShowSourceCode, true);
}

Boolean 
XPFApplication::copyFilter (const FSRef *src)
{
	FSSpec spec;
	FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
   	setCopyingFile (spec.name);
	return true;
}

void
XPFApplication::setCopyInProgress (bool val)
{
	if (fCopyInProgress != val) {
		fCopyInProgress = val;
		Changed (cSetCopyInProgress, NULL);
	}
}

void
XPFApplication::setCopyingFile (CStr255_AC copyMessage)
{
	fCopyingFile.CopyFrom (copyMessage);
	Changed (cSetCopyMessage, NULL);
}

void 
XPFApplication::install ()
{	
	try {
		setCopyInProgress (true);
		
		setCopyingFile ("BootX");
		
		// See if we have to install BootX
		// I was forcing the install, but it shouldn't be necessary anymore with my
		// versioning system in place now.
		fPrefs->getInstallDisk ()->installBootXIfNecessary (fPrefs->getReinstallBootX ());
								
		// First, create the System:Library:Extensions directories, if not already in existence
		
		UniChar extensionsCacheName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's', '.', 'm', 'k', 'e', 'x', 't'};
				
		FSRef systemFolder, systemLibraryFolder, systemLibraryExtensionsFolder;
		
		UniChar systemName[] = {'S', 'y', 's', 't', 'e', 'm'};
		UniChar libraryName[] = {'L', 'i', 'b', 'r', 'a', 'r', 'y'};
		UniChar extensionsName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's'};
		
		FSCatalogInfo catInfo, tmpCatInfo;
		
		catInfo.permissions[0] = 0;
		catInfo.permissions[1] = 0;
		catInfo.permissions[2] = 0x41ED;
		catInfo.permissions[3] = 0;

		tmpCatInfo.permissions[0] = 0;
		tmpCatInfo.permissions[1] = 0;
		tmpCatInfo.permissions[2] = 0x43FF;
		tmpCatInfo.permissions[3] = 0;

		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (fPrefs->getInstallDisk()->getRootDirectory(), 
			sizeof (systemName) / sizeof (UniChar), systemName, kFSCatInfoPermissions, &catInfo, 
			&systemFolder, NULL, NULL));
		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&systemFolder, 
			sizeof (libraryName) / sizeof (UniChar), libraryName, kFSCatInfoPermissions, &catInfo, 
			&systemLibraryFolder, NULL, NULL));
		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&systemLibraryFolder,
			sizeof (extensionsName) / sizeof (UniChar), extensionsName, kFSCatInfoPermissions, &catInfo,
			&systemLibraryExtensionsFolder, NULL, NULL));	
						
		// Now we copy over any of the hfs archives

		copyHFSArchivesTo (&systemLibraryExtensionsFolder);
	
		// If there is an existing extensions cache, we'd best delete it

		FSRef existingCache;
		OSErr err = FSMakeFSRefUnicode (&systemLibraryFolder, 
			sizeof (extensionsCacheName) / sizeof (UniChar), extensionsCacheName, kTextEncodingUnknown,
			&existingCache);
		if (err == noErr) FSDeleteObject (&existingCache);
			
		// Now, get or create the /private/tmp directories, for the copies from the CD
		
		UniChar privateName[] = {'p', 'r', 'i', 'v', 'a', 't', 'e'};
		UniChar tmpName[] = {'t', 'm', 'p'};
		UniChar kernelName[] = {'m', 'a', 'c', 'h', '_', 'k', 'e', 'r', 'n', 'e', 'l'};
		
		FSRef privateFolder, privateTmpFolder;
		
		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (fPrefs->getInstallDisk()->getRootDirectory (),
			sizeof (privateName) / sizeof (UniChar), privateName, kFSCatInfoPermissions, &catInfo,
			&privateFolder, NULL, NULL));
		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&privateFolder, 
			sizeof (tmpName) / sizeof (UniChar), tmpName, kFSCatInfoPermissions, &tmpCatInfo,
			&privateTmpFolder, NULL, NULL));

		// Now, copy the kernel from the CD
		
		setCopyingFile ("mach_kernel");
		
		FSRef kernelOnCD;
		ThrowIfOSErr_AC (FSMakeFSRefUnicode (fPrefs->getBootDisk()->getRootDirectory (),
			sizeof (kernelName) / sizeof (UniChar), kernelName, kTextEncodingUnknown, &kernelOnCD));
			
		err = FSRefFileCopy (&kernelOnCD, &privateTmpFolder, NULL, NULL, 0, false);
		if (err == dupFNErr) {
			FSRef kernelOnTarget;
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (&privateTmpFolder,
				sizeof (kernelName) / sizeof (UniChar), kernelName, kTextEncodingUnknown, &kernelOnTarget));
			ThrowIfOSErr_AC (FSDeleteObject (&kernelOnTarget));
			ThrowIfOSErr_AC (FSRefFileCopy (&kernelOnCD, &privateTmpFolder, NULL, NULL, 0, false));
		} else {
			ThrowIfOSErr_AC (err);
		}
		
		// Done! Now, we need to create the /System/Library/Extensions structure in /private/tmp. First, 
		// we'll delete anything we find there (it is the /tmp folder, after all!) and then we'll add our 
		// stuff.
		
		FSRef tmpSystemFolder, tmpSystemLibraryFolder;
		err = FSMakeFSRefUnicode (&privateTmpFolder,
			sizeof (systemName) / sizeof (UniChar), systemName, kTextEncodingUnknown, &tmpSystemFolder);
		if (err == noErr) ThrowIfOSErr_AC (FSRefDeleteDirectory (&tmpSystemFolder));
		ThrowIfOSErr_AC (FSCreateDirectoryUnicode (&privateTmpFolder, 
			sizeof (systemName) / sizeof (UniChar), systemName, kFSCatInfoNone, NULL,
			&tmpSystemFolder, NULL, NULL));
		ThrowIfOSErr_AC (FSCreateDirectoryUnicode (&tmpSystemFolder,
			sizeof (libraryName) / sizeof (UniChar), libraryName, kFSCatInfoNone, NULL,
			&tmpSystemLibraryFolder, NULL, NULL));
			
		// Now, if there is an Extensions.mkext, then we'll just copy it. If not, we'll copy the entire
		// Extensions folder from the CD.

		FSRef cdSystemFolder, cdSystemLibraryFolder, cdSystemLibraryExtensionsFolder, cdExtensionsCache;
		
		ThrowIfOSErr_AC (FSMakeFSRefUnicode (fPrefs->getBootDisk()->getRootDirectory (),
			sizeof (systemName) / sizeof (UniChar), systemName, kTextEncodingUnknown, &cdSystemFolder));
		ThrowIfOSErr_AC (FSMakeFSRefUnicode (&cdSystemFolder,
			sizeof (libraryName) / sizeof (UniChar), libraryName, kTextEncodingUnknown, &cdSystemLibraryFolder));
		err = FSMakeFSRefUnicode (&cdSystemLibraryFolder, 
			sizeof (extensionsCacheName) / sizeof (UniChar), extensionsCacheName, kTextEncodingUnknown, &cdExtensionsCache);
		if (err == noErr) {
			// we have a cache, so we'll just copy it.
			setCopyingFile ("Extensions.mkext");
			ThrowIfOSErr_AC (FSRefFileCopy (&cdExtensionsCache, &tmpSystemLibraryFolder, NULL, NULL, 0, false));
		} else {
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (&cdSystemLibraryFolder,
				sizeof (extensionsName) / sizeof (UniChar), extensionsName, kTextEncodingUnknown, &cdSystemLibraryExtensionsFolder));
			ThrowIfOSErr_AC (FSRefFilteredDirectoryCopy (&cdSystemLibraryExtensionsFolder, &tmpSystemLibraryFolder, NULL, NULL, 0, false, 
									NULL, copyFilterGlue));
		}
		
		// Now, we will copy BootX from the CD, to make the Installer think that Mac OS X is already
		// installed, so that it won't move the extensions we pre-installed
		
		FSRef systemLibraryCoreServicesFolder, cdSystemLibraryCoreServicesFolder, cdBootX;
		UniChar coreServicesName[] = {'C', 'o', 'r', 'e', 'S', 'e', 'r', 'v', 'i', 'c', 'e', 's'};
		UniChar bootXName[] = {'B', 'o', 'o', 't', 'X'};
		
		ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&systemLibraryFolder,
			sizeof (coreServicesName) / sizeof (UniChar), coreServicesName, kFSCatInfoPermissions, &catInfo,
			&systemLibraryCoreServicesFolder, NULL, NULL));	
		ThrowIfOSErr_AC (FSMakeFSRefUnicode (&cdSystemLibraryFolder,
				sizeof (coreServicesName) / sizeof (UniChar), coreServicesName, kTextEncodingUnknown, &cdSystemLibraryCoreServicesFolder));
		ThrowIfOSErr_AC (FSMakeFSRefUnicode (&cdSystemLibraryCoreServicesFolder, 
			sizeof (bootXName) / sizeof (UniChar), bootXName, kTextEncodingUnknown, &cdBootX));

		setCopyingFile ("BootX");
		err = FSRefFileCopy (&cdBootX, &systemLibraryCoreServicesFolder, NULL, NULL, 0, false);
		if (err != dupFNErr) ThrowIfOSErr_AC (err);
		
		// Now, we need to setup the /Library/Extensions folder in tmp with our extra stuff

		FSRef tmpLibraryFolder, tmpLibraryExtensionsFolder;
		err = FSMakeFSRefUnicode (&privateTmpFolder,
			sizeof (libraryName) / sizeof (UniChar), libraryName, kTextEncodingUnknown, &tmpLibraryFolder);
		if (err == noErr) ThrowIfOSErr_AC (FSRefDeleteDirectory (&tmpLibraryFolder));
		ThrowIfOSErr_AC (FSCreateDirectoryUnicode (&privateTmpFolder, 
			sizeof (libraryName) / sizeof (UniChar), libraryName, kFSCatInfoNone, NULL,
			&tmpLibraryFolder, NULL, NULL));
		ThrowIfOSErr_AC (FSCreateDirectoryUnicode (&tmpLibraryFolder,
			sizeof (extensionsName) / sizeof (UniChar), extensionsName, kFSCatInfoNone, NULL,
			&tmpLibraryExtensionsFolder, NULL, NULL));		
		copyHFSArchivesTo (&tmpLibraryExtensionsFolder);

		// Now, set up & write the NVRAM
		NVRAMVariables *nvram = NVRAMVariables::GetVariables ();
		
		CChar255_AC bootDevice (fPrefs->getBootDeviceForInstall ());
		CChar255_AC bootCommand (fPrefs->getBootCommandForInstall ());
		CChar255_AC bootFile (fPrefs->getBootFileForInstall ());
		nvram->setBootDevice (bootDevice);
		nvram->setBootCommand (bootCommand);
		nvram->setBootFile (bootFile);
		nvram->setAutoBoot (fPrefs->getAutoBoot ());
		CChar255_AC inputDevice (fPrefs->getInputDevice ());
		nvram->setInputDevice (inputDevice);
		CChar255_AC outputDevice (fPrefs->getOutputDevice ());
		nvram->setOutputDevice (outputDevice);
		
		adjustThrottle (nvram);
		
		#if qLogging
			gLogFile << "Restarting ..." << endl_AC;
			gLogFile << "Boot-device: " << bootDevice << endl_AC;
			gLogFile << "Boot-file: " << bootFile << endl_AC;
			gLogFile << "Boot-command: " << bootCommand << endl_AC;
			gLogFile << "input-device: " << inputDevice << endl_AC;
			gLogFile << "output-device: " << outputDevice << endl_AC;
		#endif
		
		#if qDebug
			cout << "boot-device: " << bootDevice << endl;
			cout << "boot-command: " << bootCommand << endl;
			cout << "Restarting ..." << endl;
		#else
			if (nvram->write () == noErr) {
				tellFinderToRestart ();
			} else {
				ThrowException_AC (kErrorWritingNVRAM, 0);
			}
		#endif
	}
	catch (...) {	
		setCopyInProgress (false);
		throw;
	}
	
	setCopyInProgress (false);
}

void
XPFApplication::copyHFSArchivesTo (FSRef *directory)
{
	SInt16 archiveCount = CountResources ('hfsA');
	for (SInt16 x = 1; x <= archiveCount; x++) {
		SetResLoad (false);
		Handle hfsA = GetIndResource ('hfsA', x);
		SetResLoad (true);
		SInt16 resourceID;
		GetResInfo (hfsA, &resourceID, NULL, NULL);
		ReleaseResource (hfsA);

		CResourceStream_AC stream ('hfsA', resourceID);
		HFSPlusArchive archive (&stream, copyFilterGlue);
		ThrowIfOSErr_AC (archive.extractArchiveTo (directory));
	}
}

void
XPFApplication::tellFinderToRestart ()
{
	AEDesc finderAddr;
	AppleEvent myRestart, nilReply;
	OSType fndrSig = 'MACS';
    ThrowIfOSErr_AC (AECreateDesc (typeApplSignature, &fndrSig, sizeof(fndrSig), &finderAddr));
   	ThrowIfOSErr_AC (AECreateAppleEvent (kAEFinderEvents, kAERestart, &finderAddr, kAutoGenerateReturnID,
                              kAnyTransactionID, &myRestart));
    ThrowIfOSErr_AC (AESend (&myRestart, &nilReply, kAENoReply + kAECanSwitchLayer + kAEAlwaysInteract,
                  kAENormalPriority, kAEDefaultTimeout, NULL, NULL));
	AEDisposeDesc (&myRestart);
	AEDisposeDesc (&finderAddr);
}

void
XPFApplication::prepare ()
{
	// First, we copy the extra extensions into place

	FSRef systemFolder, systemLibraryFolder, systemLibraryExtensionsFolder;

	UniChar systemName[] = {'S', 'y', 's', 't', 'e', 'm'};
	UniChar libraryName[] = {'L', 'i', 'b', 'r', 'a', 'r', 'y'};
	UniChar extensionsName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's'};

	ThrowIfOSErr_AC (FSMakeFSRefUnicode (fPrefs->getBootDisk()->getRootDirectory(), 
		sizeof (systemName) / sizeof (UniChar), systemName, kTextEncodingUnknown, &systemFolder));
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&systemFolder, 
		sizeof (libraryName) / sizeof (UniChar), libraryName, kTextEncodingUnknown, &systemLibraryFolder));
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&systemLibraryFolder,
		sizeof (extensionsName) / sizeof (UniChar), extensionsName, kTextEncodingUnknown, &systemLibraryExtensionsFolder));

	copyHFSArchivesTo (&systemLibraryExtensionsFolder);
	
	// If there is an existing extensions cache, we'd best delete it

	FSRef existingCache;
	UniChar extensionsCacheName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's', '.', 'm', 'k', 'e', 'x', 't'};
	OSErr err = FSMakeFSRefUnicode (&systemLibraryFolder, 
		sizeof (extensionsCacheName) / sizeof (UniChar), extensionsCacheName, kTextEncodingUnknown,
		&existingCache);
	if (err == noErr) FSDeleteObject (&existingCache);
	
	// Now, we need to copy the OldWorldSupport package to the packages folder
	
	FSRef installationFolder, packagesFolder;
	UniChar installationName[] = {'I', 'n', 's', 't', 'a', 'l', 'l', 'a', 't', 'i', 'o', 'n'};
	UniChar packagesName[] = {'P', 'a', 'c', 'k', 'a', 'g', 'e', 's'};
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&systemFolder, 
		sizeof (installationName) / sizeof (UniChar), installationName, kTextEncodingUnknown, &installationFolder));
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&installationFolder, 
		sizeof (packagesName) / sizeof (UniChar), packagesName, kTextEncodingUnknown, &packagesFolder));
			
	CResourceStream_AC stream ('pkgA', 128);
	HFSPlusArchive archive (&stream, copyFilterGlue);
	ThrowIfOSErr_AC (archive.extractArchiveTo (&packagesFolder));

	// Now, we need to modify the /private/etc/rc.cdrom script to run kextd properly
	{
		CFSSpec_AC cdromScriptSpec;
		ThrowIfOSErr_AC (FSMakeFSSpec (fPrefs->getBootDisk()->getIOVDrvInfo (), fsRtDirID, "\p:private:etc:rc.cdrom", &cdromScriptSpec));
		CFile_AC cdromScriptFile;
		cdromScriptFile.Specify (cdromScriptSpec);
		cdromScriptFile.OpenDataFork (fsRdWrPerm);
		long length;
		cdromScriptFile.GetDataLength (length);
		Ptr data = NewPtr (length + 1);
		ThrowIfNULL_AC (data);
		data[length] = 0;
		cdromScriptFile.ReadData (data, length);
		char *find = strstr (data, "kextd -j");
		if (find) {
			cdromScriptFile.SetPosition (find - data);
			length = 8;
			ThrowIfOSErr_AC (cdromScriptFile.WriteData ("kextd   ", length));
		}
		cdromScriptFile.CloseDataFork ();
	}
	
	// Now, we need to modify the installation file
	{
		CFSSpec_AC packageSpec;
		ThrowIfOSErr_AC (FSMakeFSSpec (fPrefs->getBootDisk()->getIOVDrvInfo (), fsRtDirID, 
			"\p:System:Installation:Packages:OSInstall.mpkg:Contents:Resources:OSInstall.list", 
			&packageSpec));
		CFile_AC packageFile;
		packageFile.Specify (packageSpec);
		packageFile.OpenDataFork (fsRdWrPerm);
		long length;
		packageFile.GetDataLength (length);
		Ptr data = NewPtr (length + 1);
		ThrowIfNULL_AC (data);
		data[length] = 0;
		packageFile.ReadData (data, length);
		char *find = strstr (data, "OldWorldSupport.pkg");
		if (!find) {
			packageFile.SetPosition (length);
			char *line = "OldWorldSupport.pkg\n";
			length = strlen (line);
			ThrowIfOSErr_AC (packageFile.WriteData (line, length));
		}
		packageFile.CloseDataFork ();
	}
	
	fPrefs->getBootDisk()->setHasBeenPrepared (true);
//	if (fMainWindow) fMainWindow->UpdateUI ();
}

void
XPFApplication::adjustThrottle (NVRAMVariables *nvram) {
	char nvramrc [2048];
	strcpy (nvramrc, nvram->getNVRAMRC ());
	char *ictc = strstr (nvramrc, "11 $I");
	if (ictc) {
		char throttleValue [12];
		sprintf (throttleValue, "%X", fPrefs->getThrottle () ? (fPrefs->getThrottle () << 1) | 1 : 0);
		ictc [0] = ' ';
		ictc [1] = ' ';
		BlockMoveData (throttleValue, ictc, strlen (throttleValue));
	} 
	nvram->setNVRAMRC (nvramrc);
}

void 
XPFApplication::restart ()
{
	MountedVolume *bootDisk = fPrefs->getBootDisk ();

	if (!bootDisk->getIsOnBootableDevice ()) {
//		prepare ();
		return;
	}
	
	if (bootDisk->getIsWriteable ()) {
	
		setCopyInProgress (true);
			
		setCopyingFile ("BootX");

		// Check to see whether we need to install BootX
		bootDisk->installBootXIfNecessary (fPrefs->getReinstallBootX ());

		// Check whether we need to install Old World Support
		if (fPrefs->getReinstallExtensions () || !bootDisk->getHasOldWorldSupport ()) {
			try {
				FSRef systemFolder, systemLibraryFolder, systemLibraryExtensionsFolder;
			
				UniChar systemName[] = {'S', 'y', 's', 't', 'e', 'm'};
				UniChar libraryName[] = {'L', 'i', 'b', 'r', 'a', 'r', 'y'};
				UniChar extensionsName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's'};
			
				ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (bootDisk->getRootDirectory(), 
					sizeof (systemName) / sizeof (UniChar), systemName, kFSCatInfoNone, NULL, 
					&systemFolder, NULL, NULL));
				ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&systemFolder, 
					sizeof (libraryName) / sizeof (UniChar), libraryName, kFSCatInfoNone, NULL, 
					&systemLibraryFolder, NULL, NULL));
				ThrowIfOSErr_AC (FSGetOrCreateDirectoryUnicode (&systemLibraryFolder,
					sizeof (extensionsName) / sizeof (UniChar), extensionsName, kFSCatInfoNone, NULL,
					&systemLibraryExtensionsFolder, NULL, NULL));

				copyHFSArchivesTo (&systemLibraryExtensionsFolder);

				// If there is an existing extensions cache, we'd best delete it
		
				FSRef existingCache;
				UniChar extensionsCacheName[] = {'E', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', 's', '.', 'm', 'k', 'e', 'x', 't'};
				OSErr err = FSMakeFSRefUnicode (&systemLibraryFolder, 
					sizeof (extensionsCacheName) / sizeof (UniChar), extensionsCacheName, kTextEncodingUnknown,
					&existingCache);
				if (err == noErr) FSDeleteObject (&existingCache);
			}
			catch (...) {
				setCopyInProgress (false);
				throw;
			}
		}

		setCopyInProgress (false);
	}

	NVRAMVariables *nvram = NVRAMVariables::GetVariables ();
	CChar255_AC bootDevice (fPrefs->getBootDevice ());
	nvram->setBootDevice (bootDevice);
	CChar255_AC bootCommand (fPrefs->getBootCommand ());
	nvram->setBootCommand (bootCommand);
	nvram->setAutoBoot (fPrefs->getAutoBoot ());	
	CChar255_AC inputDevice (fPrefs->getInputDevice ());
	nvram->setInputDevice (inputDevice);
	CChar255_AC outputDevice (fPrefs->getOutputDevice ());
	nvram->setOutputDevice (outputDevice);
	
	adjustThrottle (nvram);
		
	#if qLogging
		gLogFile << "Restarting ..." << endl_AC;
		gLogFile << "Boot-device: " << bootDevice << endl_AC;
		gLogFile << "Boot-command: " << bootCommand << endl_AC;
		gLogFile << "input-device: " << inputDevice << endl_AC;
		gLogFile << "output-device: " << outputDevice << endl_AC;
	#endif

	#if qDebug
		cout << "Restarting ..." << endl;
	#else
		if (nvram->write () == noErr) {	
			tellFinderToRestart ();
		} else {
			ThrowException_AC (kErrorWritingNVRAM, 0);
		}
	#endif
}

