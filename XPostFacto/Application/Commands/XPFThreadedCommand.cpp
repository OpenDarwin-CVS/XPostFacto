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

#include "XPFThreadedCommand.h"
#include "XPFPrefs.h"
#include "MountedVolume.h"
#include "XPFStrings.h"
#include "XPFAuthorization.h"
#include "XPFLog.h"
#include "FSRefCopying.h"
#include "HFSPlusArchive.h"
#include "XPostFacto.h"
#include "XPFProgressWindow.h"
#include "UThreads.h"
#include "MoreFilesExtras.h"

// String constants

char systemName[] =  "System";
char libraryName[] = "Library";
char extensionsName[] = "Extensions"; 
char extensionsCacheName[] = "Extensions.mkext";
char privateName[] = "private";
char tmpName[] = "tmp";
char kernelName[] = "mach_kernel";
char coreServicesName[] = "CoreServices";
char bootXName[] = "BootX";
char installName[] = "XPFInstall";
char XPFName[] = ".XPostFacto";
char startupItemsName[] = "StartupItems";

// Constructor for getting the right permissions and ownership

class XPFCatalogInfo : public FSCatalogInfo {

	public:

		XPFCatalogInfo (UInt32 mode, bool isDirectory = true, UInt32 uid = 0, UInt32 gid = 0) {
			permissions[0] = uid;
			permissions[1] = gid;
			permissions[2] = mode |= (isDirectory ? 0x40000 : 0x100000);
			permissions[3] = 0;
		}
};

// ------------------
// XPFCommandThread
// ------------------

XPFCommandThread::XPFCommandThread (XPFThreadedCommand *theCommand)
	: fCommand (theCommand)
{
}

void
XPFCommandThread::Run ()
{
	try {
		fCommand->DoItThreaded ();
	}
	catch (CException_AC& ex) {
		fCommand->getProgressWindow ()->displayException (ex);
	}
}

// ------------------
// XPFThreadedCommand
// ------------------

#define Inherited TCommand

XPFThreadedCommand::XPFThreadedCommand (XPFPrefs *prefs)
	: fYielder (1)
{
	fTargetDisk = prefs->getTargetDisk ();
	fHelperDisk = fTargetDisk->getHelperDisk ();

	ThrowIfOSErr_AC (CreateTextToUnicodeInfoByEncoding (
		CreateTextEncoding (kTextEncodingMacRoman, kTextEncodingDefaultVariant, kUnicode16BitFormat),
		&fConverter));
		
	fCopyWord.CopyFrom (kXPFStringsResource, kTheWordCopy, 63);
}	

XPFThreadedCommand::~XPFThreadedCommand ()
{
	DisposeTextToUnicodeInfo (&fConverter);
}

// Methods to run things threaded

void
XPFThreadedCommand::DoIt ()
{
	fProgressWindow = (XPFProgressWindow *) TViewServer::fgViewServer->NewTemplateWindow (kProgressWindow, NULL);
	XPFCommandThread *thread = TH_new XPFCommandThread (this);
	fRunner = thread;
	fProgressWindow->setThread (thread);
	fProgressWindow->PoseModally ();
	if (thread->IsAlive ()) thread->Join ();
	fProgressWindow->Close ();
}

void
XPFThreadedCommand::setStatusMessage (unsigned char* message) {
	fProgressWindow->setStatus (message);
}

void 
XPFThreadedCommand::setCopyingFile (unsigned char *fileName)
{
	CStr255_AC message = fCopyWord + " " + fileName;
	setStatusMessage (message);
}

void 
XPFThreadedCommand::setDescription (unsigned char* description)
{
	fProgressWindow->setDescription (description);
}

Boolean 
XPFThreadedCommand::copyFilter (const FSRef *src)
{
	fYielder.YieldIfTime ();
	FSSpec spec;
	FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
 	setCopyingFile (spec.name);
	return !fRunner->ShutdownRequested ();
}

pascal Boolean 
XPFThreadedCommand::CopyFilterGlue (void *refCon, const FSRef *src)
{
	return ((XPFThreadedCommand *) refCon)->copyFilter (src);
}

// Methods to get or create directories or other FSRefs (with proper permissions and ownership)

OSErr
XPFThreadedCommand::getFSRef (FSRef *rootDirectory, char *path, FSRef *result)
{
	fYielder.YieldIfTime ();
	OSErr err;
	ByteCount uniLength, converted;
	UniChar uniChars[256];
	err = ConvertFromTextToUnicode (fConverter, strlen (path), path, 0, 0, NULL, 0, NULL, 
			256 * sizeof (UniChar), &converted, &uniLength, uniChars);
	if (err) return err;

	return FSMakeFSRefUnicode (rootDirectory, uniLength / sizeof (UniChar), uniChars, NULL, result);
}

OSErr
XPFThreadedCommand::getOrCreateDirectory (FSRef *rootDirectory, char *path, UInt32 mode, FSRef *result)
{
	fYielder.YieldIfTime ();
	OSErr err;
	ByteCount uniLength, converted;
	UniChar uniChars[256];
	err = ConvertFromTextToUnicode (fConverter, strlen (path), path, 0, 0, NULL, 0, NULL, 
			256 * sizeof (UniChar), &converted, &uniLength, uniChars);
	if (err) return err;

	XPFCatalogInfo catInfo (mode);
	XPFSetUID myUID (0);
	return FSGetOrCreateDirectoryUnicode (rootDirectory, 
			uniLength / sizeof (UniChar), uniChars, kFSCatInfoPermissions, &catInfo, 
			result, NULL, NULL);
}

OSErr
XPFThreadedCommand::getOrCreateXPFDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err = getOrCreateDirectory (rootDirectory, XPFName, 0755, result);
	if (err == noErr) {
		FSSpec spec;
		FSCatalogInfo catInfo;
		err = FSGetCatalogInfo (result, kFSCatInfoFinderInfo, &catInfo, NULL, &spec, NULL);
		if (err == noErr) {
			FolderInfo *info = (FolderInfo *) catInfo.finderInfo;
			if (!(info->finderFlags & kIsInvisible)) FSpSetIsInvisible (&spec);
		}
	}
	return err;
}

OSErr
XPFThreadedCommand::getOrCreateSystemLibraryDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err;
	FSRef systemRef;
	
	err = getOrCreateDirectory (rootDirectory, systemName, 0755, &systemRef);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemRef, libraryName, 0755, result);
}

OSErr
XPFThreadedCommand::getOrCreateSystemLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err;
	FSRef systemLibraryRef;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &systemLibraryRef);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemLibraryRef, extensionsName, 0755, result);
}

OSErr
XPFThreadedCommand::getOrCreateCoreServicesDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err;
	FSRef systemLibraryRef;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &systemLibraryRef);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemLibraryRef, coreServicesName, 0755, result);
}

OSErr
XPFThreadedCommand::getOrCreateLibraryDirectory (FSRef *rootDirectory, FSRef *result)
{
	return getOrCreateDirectory (rootDirectory, libraryName, 0755, result);
}

OSErr
XPFThreadedCommand::getOrCreateStartupDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err;
	FSRef libraryRef;
	
	err = getOrCreateLibraryDirectory (rootDirectory, &libraryRef);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&libraryRef, startupItemsName, 0755, result); 
}

OSErr
XPFThreadedCommand::getOrCreateLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result)
{
	OSErr err;
	FSRef libraryRef;
	
	err = getOrCreateLibraryDirectory (rootDirectory, &libraryRef);
	if (err != noErr) return err;
				
	return getOrCreateDirectory (&libraryRef, extensionsName, 0755, result);
}

OSErr
XPFThreadedCommand::getKernelFSRef (FSRef *rootDirectory, FSRef *result)
{
	return getFSRef (rootDirectory, kernelName, result);
}

OSErr
XPFThreadedCommand::getExtensionsCacheFSRef (FSRef *rootDirectory, FSRef *result)
{
	FSRef libraryRef;
	OSErr err;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &libraryRef);
	if (err != noErr) return err;
	
	return getFSRef (&libraryRef, extensionsCacheName, result);
}

OSErr
XPFThreadedCommand::getBootXFSRef (FSRef *rootDirectory, FSRef *result)
{
	FSRef coreServicesRef;
	OSErr err;
	
	err = getOrCreateCoreServicesDirectory (rootDirectory, &coreServicesRef);
	if (err != noErr) return err;
	
	return getFSRef (&coreServicesRef, bootXName, result);
}

// Install methods

void
XPFThreadedCommand::installExtensionsWithRootDirectory (FSRef *rootDirectory)
{
	FSRef systemLibraryExtensionsFolder;
	
	ThrowIfOSErr_AC (getOrCreateSystemLibraryExtensionsDirectory (rootDirectory, &systemLibraryExtensionsFolder));
	copyHFSArchivesTo ('hfsA', &systemLibraryExtensionsFolder);
}

void
XPFThreadedCommand::installSecondaryExtensionsWithRootDirectory (FSRef *rootDirectory)
{
	FSRef libraryExtensionsFolder;
	
	ThrowIfOSErr_AC (getOrCreateLibraryExtensionsDirectory (rootDirectory, &libraryExtensionsFolder));
	copyHFSArchivesTo ('hfsA', &libraryExtensionsFolder);
}

void
XPFThreadedCommand::installStartupItemWithRootDirectory (FSRef *rootDirectory)
{
	FSRef libraryStartupItemsFolder;
		
	ThrowIfOSErr_AC (getOrCreateStartupDirectory (rootDirectory, &libraryStartupItemsFolder));	
	copyHFSArchivesTo ('hfsS', &libraryStartupItemsFolder);
}


void
XPFThreadedCommand::copyHFSArchivesTo (ResType type, FSRef *directory)
{
	SInt16 archiveCount = CountResources (type);
	for (SInt16 x = 1; x <= archiveCount; x++) {
		fYielder.YieldIfTime ();
		if (fRunner->ShutdownRequested ()) return;
	
		SetResLoad (false);
		Handle hfsA = GetIndResource (type, x);
		SetResLoad (true);
		SInt16 resourceID;
		GetResInfo (hfsA, &resourceID, NULL, NULL);
		ReleaseResource (hfsA);

		CResourceStream_AC stream (type, resourceID);
		HFSPlusArchive archive (&stream, CopyFilterGlue, this);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (directory));
	}
}
