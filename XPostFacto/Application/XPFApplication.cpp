/*

Copyright (c) 2001 - 2003
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

#include "XPFApplication.h"
#include "XPFVersion.h"
#include "XPFAboutBox.h"
#include "XPFFatalErrorWindow.h"
#include "XPFSettingsWindow.h"
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
#include "XPFPrefs.h"
#include "XPFStrings.h"
#include "FSRefCopying.h"
#include "XPostFacto.h"
#include "OFAliases.h"
#include "XPFGridWindow.h"
#include "XPFVolumeList.h"
#include "XPFVolumeDisplay.h"
#include "XPFAuthorization.h"
#include "UTEViewStream.h"
#include "XPFHelpBehavior.h"
#include "XPFOpenURLBehavior.h"
#include "XPFProgressWindow.h"
#include "XPFVolumePopups.h"
#include "XPFButtons.h"
#include "XPFSettingsText.h"
#include "XPFWarnings.h"
#include "XPFUpdateWindow.h"

#include <InternetConfig.h>
#include <UnicodeConverter.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __MACH__
	#include <sys/types.h>
	#include <unistd.h>
#endif

enum {
  gestaltCPU7400 = 0x010C				
};

// -----------
// Constants
// -----------

// Whether we're running in OS X

UInt32 gSystemVersion = 0;

// Direct messages to log

class CLogReporter : public CReporter_AC
{
public:
	CLogReporter(TTEViewStream& stream)
		: fLog(stream)
	{}
	virtual ~CLogReporter()
	{}
	
	virtual void OutputMessage(const char* inStr, bool /*continueIfPossible*/)
	{
		fLog << inStr << endl_AC;
		fLog.Flush ();
	}
	
	TTEViewStream& fLog;
};


//========================================================================================
// CLASS XPFApplication
//========================================================================================

#undef Inherited
#define Inherited TApplication

//----------------------------------------------------------------------------------------
// XPFApplication constructor
//----------------------------------------------------------------------------------------

XPFApplication::XPFApplication() :
	TApplication (kFileType, kSignature)
{
	fShowDebugOptions = false;
	fDebugOptions = 0;
	fAboutBox = NULL;
	fLogWindow = NULL;
	fWantsNavigationServices = true;
							
	Gestalt (gestaltSystemVersion, (SInt32 *) &gSystemVersion);

	REGISTER_CLASS_AC (XPFAboutBox);
	REGISTER_CLASS_AC (XPFFatalErrorWindow);
	REGISTER_CLASS_AC (XPFSettingsWindow);
	REGISTER_CLASS_AC (XPFGridWindow);
	REGISTER_CLASS_AC (XPFTargetVolumeList);
	REGISTER_CLASS_AC (XPFInstallCDList);
	REGISTER_CLASS_AC (XPFVolumeDisplay);
	REGISTER_CLASS_AC (XPFHelpBehavior);
	REGISTER_CLASS_AC (XPFOpenURLBehavior);
	REGISTER_CLASS_AC (XPFProgressWindow);
	REGISTER_CLASS_AC (XPFHelperPopup);
	REGISTER_CLASS_AC (XPFInstallCDPopup);
	REGISTER_CLASS_AC (XPFInstallButton);
	REGISTER_CLASS_AC (XPFRestartButton);
	REGISTER_CLASS_AC (XPFSettingsText);
	REGISTER_CLASS_AC (XPFMacOS9Button);
	REGISTER_CLASS_AC (XPFMacOSXButton);
	REGISTER_CLASS_AC (XPFInstallWarning);
	REGISTER_CLASS_AC (XPFRestartWarning);
	REGISTER_CLASS_AC (XPFUpdateWindow);
	
	ProcessInfoRec info;
	FSSpec appSpec, resourceSpec;
	ProcessSerialNumber psn = {0, kCurrentProcess};

	info.processInfoLength = sizeof (info);
	info.processName = NULL;
	info.processAppSpec = &appSpec;
	
	GetProcessInformation (&psn, &info);
	FSMakeFSSpec (appSpec.vRefNum, appSpec.parID, "\p::Resources:SharedResources.rsrc", &resourceSpec);
	FSpOpenResFile (&resourceSpec, fsRdPerm);
}

//----------------------------------------------------------------------------------------
// XPFApplication destructor
//----------------------------------------------------------------------------------------

XPFApplication::~XPFApplication()
{
	gLogFile.setViewStream (NULL);
	gLogFile << endl_AC << "Quitting" << endl_AC;

	if (fPlatform) delete fPlatform;
	if (fAboutBox) delete fAboutBox;
	if (fLogWindow) delete fLogWindow;
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
XPFApplication::DoAEClose(TAppleEvent* message, TAppleEvent* reply)
{
	CTempAllocation tempAllocation(true);
	CTempObjectAllocation tempObjectAllocation(true);
	TQuitCommand * quitCommand = TH_new TQuitCommand (message, reply);
	quitCommand->Process();
}

void
XPFApplication::launchURL (CStr255_AC theURL)
{
	if ((Ptr) ICStart == (Ptr) kUnresolvedCFragSymbolAddress) return;
	long start = 0, end = theURL[0];
	ICInstance inst;
	ThrowIfOSErr_AC (ICStart (&inst, 'usuX'));
#ifndef __MACH__
	ThrowIfOSErr_AC (ICFindConfigFile (inst, 0, NULL));
#endif
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
		
#if __MACH__	
	
		// Now, we open the file with a browser
		FSRef helpFileRef;
		ThrowIfOSErr_AC (FSpMakeFSRef (&helpSpec, &helpFileRef));
		CFURLRef urlRef = CFURLCreateFromFSRef (NULL, &helpFileRef);
		CFStringRef urlStringRef = CFURLGetString (urlRef);
		Str255 urlString;
		CFStringGetPascalString (urlStringRef, urlString, 255, CFStringGetSystemEncoding());
		CFRelease (urlRef);
		CFRelease (urlStringRef);

		launchURL (urlString);	
		
#else

		AEDesc finderAddr, fileDesc;
		AEDescList docs;
		AppleEvent myOpenDoc, nilReply;
		AliasHandle docAlias;
		
		OSType fndrSig = 'MACS';
  		ThrowIfOSErr_AC (AECreateDesc (typeApplSignature, &fndrSig, sizeof (fndrSig), &finderAddr));
   		ThrowIfOSErr_AC (AECreateAppleEvent (kCoreEventClass, kAEOpenDocuments, &finderAddr, 
   				kAutoGenerateReturnID, kAnyTransactionID, &myOpenDoc));
   				
   		ThrowIfOSErr_AC (AECreateList (NULL, 0, false, &docs));
   		ThrowIfOSErr_AC (NewAlias (NULL, &helpSpec, &docAlias));
   		
   		HLock ((Handle) docAlias);
		ThrowIfOSErr_AC (AECreateDesc (typeAlias, (Ptr) (*docAlias), 
				GetHandleSize ((Handle) docAlias), &fileDesc));
		HUnlock ((Handle) docAlias); 
		
		ThrowIfOSErr_AC (AEPutDesc (&docs, 0, &fileDesc));
		ThrowIfOSErr_AC (AEPutParamDesc (&myOpenDoc, keyDirectObject, &docs));
   		
   		ThrowIfOSErr_AC (AESend (&myOpenDoc, &nilReply, kAENoReply + kAECanSwitchLayer + kAEAlwaysInteract,
                  kAENormalPriority, kAEDefaultTimeout, NULL, NULL));
                  
		DisposeHandle ((Handle) docAlias);
		AEDisposeDesc (&fileDesc);
		AEDisposeDesc (&docs);
		AEDisposeDesc (&myOpenDoc);
		AEDisposeDesc (&finderAddr);

#endif	
	}
}

void
XPFApplication::DoInitialState ()
{
	Inherited::DoInitialState ();

	fSplash = TViewServer::fgViewServer->NewTemplateWindow (kSplashWindow, NULL);
	fSplash->Open ();
	fSplash->Show (true, true);
	UpdateAllWindows ();

	fLogWindow = TViewServer::fgViewServer->NewTemplateWindow (kLogWindow, NULL);	
	fViewStream = new TTEViewStream (dynamic_cast_or_throw_AC (TTEView*, fLogWindow->FindSubView ('LogV')));
	fReporter = new CLogReporter (*fViewStream);
	gLogFile.setViewStream (fViewStream);

	fPlatform = new XPFPlatform;
}

void 
XPFApplication::reportFatalError (CStr255_AC error)
{
	XPFFatalErrorWindow *window = (XPFFatalErrorWindow *) TViewServer::fgViewServer->NewTemplateWindow(1006, NULL);
	window->Open ();
	window->SetText (error);
}

void
XPFApplication::CloseSplashWindow ()
{
	if (fSplash) {
		fSplash->Close ();
		fSplash = NULL;
	}
}

TDocument* 
XPFApplication::DoMakeDocument (CommandNumber /* itsCommandNumber */, TFile* itsFile)
{
	fPrefs = new XPFPrefs (itsFile);
	return fPrefs;
}

TFile*
XPFApplication::FindOrCreatePrefsFile (CommandNumber itsCommandNumber)
{
	// What I need to do here is either find or create my preferences file, and 
	// then generate a command to open it as an "old" file.

	TFile *prefsFile = DoMakeFile (itsCommandNumber);

	// We're going to put the prefs file in an invisible .XPostFacto folder in the root directory.
	// If we can find a prefs file on any disk, we'll use it.
	// If we can't find a prefs file, we'll put it on the system disk.
	// The idea is to retain preferences between Mac OS 9 and Mac OS X
	
	// First, we'll look on the system disk
	
	OSErr err;
	short vRefNum;
	long dirID;
	FSSpec fileSpec;

	err = FindFolder (kOnSystemDisk, kVolumeRootFolderType, true, &vRefNum, &dirID);
	if (err == noErr) {
		err = FSMakeFSSpec (vRefNum, fsRtDirID, "\p:.XPostFacto:XPostFacto Prefs", &fileSpec);
		if (err == noErr) {
			prefsFile->Specify (fileSpec);
			return prefsFile;
		}
	}

	// If we don't find one, then we'll look in each available volume

	ItemCount item = 0;
	FSRef rootDirectory;	
	while (true) {	
		item++;
		err = FSGetVolumeInfo (kFSInvalidVolumeRefNum, item, NULL, kFSCatInfoNone, NULL, NULL, &rootDirectory);
		if (err != noErr) break;
		err = FSGetCatalogInfo (&rootDirectory, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL);
		if (err == noErr) {
			err = FSMakeFSSpec (fileSpec.vRefNum, fsRtDirID, "\p:.XPostFacto:XPostFacto Prefs", &fileSpec);
			if (err == noErr) {
				prefsFile->Specify (fileSpec);
				return prefsFile;
			}
		}
	}

	// If we still don't have one, then we'll have to create it
	
	ThrowIfOSErr_AC (FindFolder (kOnSystemDisk, kVolumeRootFolderType, true, &vRefNum, &dirID));
	err = FSMakeFSSpec (vRefNum, fsRtDirID, "\p:.XPostFacto", &fileSpec);
	if (err != noErr) {
		ThrowIfOSErr_AC (FSpDirCreate (&fileSpec, smSystemScript, &dirID));
		ThrowIfOSErr_AC (FSMakeFSSpec (vRefNum, dirID, "\p", &fileSpec));
		FSpSetIsInvisible (&fileSpec);
	}
	err = FSMakeFSSpec (vRefNum, fsRtDirID, "\p:.XPostFacto:XPostFacto Prefs", &fileSpec);
	prefsFile->Specify (fileSpec);
	ThrowIfOSErr_AC (prefsFile->CreateCFile ());
	
	return prefsFile;
}

TDocument* 
XPFApplication::OpenNew (CommandNumber itsCommandNumber)
{	
	// Check for a few things that could stop me in my tracks
	
#ifdef __MACH__
	if (geteuid () != 0) {
		OSStatus status = XPFAuthorization::Authenticate ();
		if (status == errAuthorizationSuccess) {		
			CFURLRef myURL = CFBundleCopyExecutableURL (CFBundleGetMainBundle ());
			if (myURL) {
				FSRef myFSRef;
				if (CFURLGetFSRef (myURL, &myFSRef)) {
					char myPath[256];
					OSStatus err = FSRefMakePath (&myFSRef, (UInt8 *) myPath, 255);
					if (err == noErr) {
						XPFAuthorization::ExecuteWithPrivileges (myPath, NULL, NULL);				
					}
				}
				CFRelease (myURL);
			}	
		}
		PostCommand (TH_new TQuitCommand);
		return NULL;
	} else {
		seteuid (getuid ()); // we'll set it back when we need to
		ProcessSerialNumber currentProcess = {0, kCurrentProcess};
		SetFrontProcess (&currentProcess);
	}
#endif

	#if qLogging
		gLogFile << "XPostFacto Version " << kXPFVersion << endl_AC;
	#endif
	
	CStr255_AC message;
	UpdateAllWindows ();
			
	if (gSystemVersion < 0x0900) {
		if (fSplash) fSplash->Close ();
		GetIndString (message, kXPFStringsResource, kRequiresOS9);
		reportFatalError (message);
		return NULL;	
	}
	
/*
	if (!fPlatform->getCanPatchNVRAM()) {
		if (fSplash) fSplash->Close ();
		GetIndString (message, kXPFStringsResource, kNoNVRAMPatches);
		message += fPlatform->getCompatible();
		reportFatalError (message);
		return NULL;
	}
*/
	
	// Then tell it to open my prefs file, either old or new
	
	CList_AC *fileList = NewList_AC ();
	fileList->InsertLast (FindOrCreatePrefsFile (itsCommandNumber));
	PostCommand (TH_new TODocCommand (itsCommandNumber, fileList));
													
	return NULL;
}

void 
XPFApplication::HandleDiskEvent(TToolboxEvent* event)
{
	TApplication::HandleDiskEvent (event);
	MountedVolume::Initialize ();
	if (!fPrefs->getInstallCD ()) fPrefs->setInstallCD (MountedVolume::GetDefaultInstallerDisk ());
}

void
XPFApplication::addDebugOptionsToHelpMenu ()
{
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem ("-", cNoCommand);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableRestart), cDisableRestart);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableNVRAM), cDisableNVRAMWriting);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableBootX), cDisableBootX);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableStartupItem), cDisableStartupItem);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableExtensions), cDisableExtensions);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableExtensionsCache), cDisableExtensionsCache);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableCoreServices), cDisableCoreServices);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuDisableHelperCopy), cDisableCopyToHelper);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (CString_AC ((ResNumber) kXPFStringsResource, kMenuVisibleHelper), cVisibleHelperFiles);
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
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem ("-", cNoCommand);
	GetIndString (theMenuName, kXPFStringsResource, kShowLogWindowMenu);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (theMenuName, cShowLogWindow);
	GetIndString (theMenuName, kXPFStringsResource, kShowDebugOptions);
	TMenuBarManager::fgMenuBarManager->AddHelpMenuItem (theMenuName, cShowDebugOptions);
	
	if (fShowDebugOptions) addDebugOptionsToHelpMenu ();
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
			
		case cShowLogWindow:
			fLogWindow->Show (true, false);
			fLogWindow->Select ();
			break;
			
		case cShowDebugOptions:
			if (!fShowDebugOptions) addDebugOptionsToHelpMenu ();
			fShowDebugOptions = true;
			break;
			
		case cDisableRestart: 			toggleDebugOption (kDisableRestart); break;
		case cVisibleHelperFiles: 		toggleDebugOption (kVisibleHelperFiles); break;
		case cDisableNVRAMWriting: 		toggleDebugOption (kDisableNVRAMWriting); break;
		case cDisableBootX: 			toggleDebugOption (kDisableBootX); break;
		case cDisableCopyToHelper: 		toggleDebugOption (kDisableCopyToHelper); break;
		case cDisableExtensions: 		toggleDebugOption (kDisableExtensions); break;
		case cDisableExtensionsCache:	toggleDebugOption (kDisableExtensionsCache); break;
		case cDisableStartupItem: 		toggleDebugOption (kDisableStartupItem); break;
		case cDisableCoreServices: 		toggleDebugOption (kDisableCoreServices); break;
				
		default:
			TApplication::DoMenuCommand(aCommandNumber);
			break;
	}
}

void 
XPFApplication::DoSetupMenus() 
{
	TApplication::DoSetupMenus();
	
	bool enable = !InModalState ();
	
	Enable (cShowHelpFile, enable);
	Enable (cShowOnlineHelpFile, enable);
	Enable (cShowSourceCode, enable);
	Enable (cShowLogWindow, enable);
	Enable (cShowDebugOptions, enable);
	
	EnableCheck (cDisableRestart, enable, fDebugOptions & kDisableRestart);
	EnableCheck (cVisibleHelperFiles, enable, fDebugOptions & kVisibleHelperFiles);
	EnableCheck (cDisableNVRAMWriting, enable, fDebugOptions & kDisableNVRAMWriting);
	EnableCheck (cDisableCopyToHelper, enable, fDebugOptions & kDisableCopyToHelper);
	EnableCheck (cDisableBootX, enable, fDebugOptions & kDisableBootX);
	EnableCheck (cDisableExtensions, enable, fDebugOptions & kDisableExtensions);
	EnableCheck (cDisableExtensionsCache, enable, fDebugOptions & kDisableExtensionsCache);
	EnableCheck (cDisableStartupItem, enable, fDebugOptions & kDisableStartupItem);
	EnableCheck (cDisableCoreServices, enable, fDebugOptions & kDisableCoreServices);	
}
