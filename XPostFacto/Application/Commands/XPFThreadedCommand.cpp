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
#include "XPFApplication.h"
#include "XPFErrors.h"
#include "XPFFSRef.h"
#include "XPFUpdate.h"

#ifdef __MACH__
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/stat.h>
#endif

// ------------------
// XPFThreadedCommand
// ------------------

#define Inherited TCommand

XPFThreadedCommand::XPFThreadedCommand (XPFUpdate *update)
	: fItemsToCopy (0), fItemsCopied (0), fProgressMin (0), fProgressMax (0)
{	
	fUpdate = update;
	
	fRootDisk = update->getTarget ();
	fRootDisk->turnOffIgnorePermissions ();
	
	fBootDisk = update->getHelper ();
	if (fBootDisk == NULL) fBootDisk = fRootDisk;
	if (fBootDisk != fRootDisk) fBootDisk->turnOffIgnorePermissions ();

	fDebugOptions = ((XPFApplication *) gApplication)->getDebugOptions ();		
	fCopyWord.CopyFrom (kXPFStringsResource, kTheWordCopy, 63);
	
	fPrefs = ((XPFApplication *) gApplication)->getPrefs ();
}	

// Methods to run things in a progress window

void
XPFThreadedCommand::DoIt ()
{
	fProgressWindow = (XPFProgressWindow *) TViewServer::fgViewServer->NewTemplateWindow (kProgressWindow, NULL);
	fProgressWindow->setProgressMax (1000);
	fProgressWindow->Open ();
	fProgressWindow->Show (true, true);
	try {
		DoItInProgressWindow ();
	}
	catch (...) {
		fProgressWindow->Close ();
		throw;
	}
	fProgressWindow->Close ();
}

void
XPFThreadedCommand::setStatusMessage (unsigned char* message, bool forceRedraw) {
	fProgressWindow->setStatus (message, forceRedraw);
}

void 
XPFThreadedCommand::setCopyingFile (unsigned char *fileName, bool forceRedraw)
{
	CStr255_AC message = fCopyWord + " " + fileName;
	setStatusMessage (message, forceRedraw);
}

void 
XPFThreadedCommand::setDescription (unsigned char* description)
{
	fProgressWindow->setDescription (description);
}

Boolean 
XPFThreadedCommand::copyFilter (const FSRef *src, Boolean preflight)
{
	if (preflight) {
		fItemsToCopy++;
	} else {
		fItemsCopied++;
		float scale = (float) (fProgressMax - fProgressMin) / (float) fItemsToCopy;
		fProgressWindow->setProgressValue (fProgressMin + scale * fItemsCopied, false);
		FSSpec spec;
		FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
	 	setCopyingFile (spec.name, false);
	}
	return true;
}

Boolean 
XPFThreadedCommand::archiveFilter (const FSRef *src, Boolean preflight)
{
	#pragma unused (preflight)
	FSSpec spec;
	FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
 	setCopyingFile (spec.name, false);
	return true;
}

pascal Boolean 
XPFThreadedCommand::CopyFilterGlue (void *refCon, const FSRef *src, Boolean preflight)
{
	return ((XPFThreadedCommand *) refCon)->copyFilter (src, preflight);
}

pascal Boolean 
XPFThreadedCommand::ArchiveFilterGlue (void *refCon, const FSRef *src, Boolean preflight)
{
	return ((XPFThreadedCommand *) refCon)->archiveFilter (src, preflight);
}

// Install methods

void
XPFThreadedCommand::updateExtensionsCacheForRootDirectory (FSRef *rootDirectory)
{
	if (fDebugOptions & kDisableExtensionsCache) return;
#ifdef __MACH__
	fProgressWindow->setProgressMax (0);
	fProgressWindow->setProgressMin (0);
	fProgressWindow->setProgressValue (0, true);
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kUpdatingExtensionsCache), true);

	char rootPath [1024];
	ThrowIfOSErr_AC (FSRefMakePath (rootDirectory, (UInt8 *) rootPath, 1023));
	XPFSetUID myUID (0);
	if (!chdir (rootPath)) {
		pid_t pid = fork ();
		if (pid) {
			if (pid != -1) {
				int status;
				while (!wait4 (pid, &status, WNOHANG, 0)) {
					usleep (1000000 / 60); // about 1 tick 
					fProgressWindow->animate ();
				}
			}
		} else {
			execl ("/usr/sbin/kextcache", "kextcache", "-l", "-m", "System/Library/Extensions.mkext", "System/Library/Extensions", NULL);
			ThrowException_AC (kInternalError, 0);	// the execl shouldn't return
		}
	}
#else
	FSRef extensionsCache;
	OSErr err = XPFFSRef::getExtensionsCacheFSRef (rootDirectory, &extensionsCache, false);
	if (err == noErr) FSDeleteObject (&extensionsCache);
#endif
}

void
XPFThreadedCommand::installExtensionsWithRootDirectory (FSRef *rootDirectory)
{
	if (fDebugOptions & kDisableExtensions) return;
	FSRef systemLibraryExtensionsFolder;
	
	ThrowIfOSErr_AC (XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (rootDirectory, &systemLibraryExtensionsFolder));
	copyHFSArchivesTo ('hfsA', &systemLibraryExtensionsFolder);
		
	updateExtensionsCacheForRootDirectory (rootDirectory);
}

void
XPFThreadedCommand::installSecondaryExtensionsWithRootDirectory (FSRef *rootDirectory)
{
	if (fDebugOptions & kDisableExtensions) return;
	FSRef libraryExtensionsFolder;
	
	ThrowIfOSErr_AC (XPFFSRef::getOrCreateLibraryExtensionsDirectory (rootDirectory, &libraryExtensionsFolder));
	copyHFSArchivesTo ('hfsA', &libraryExtensionsFolder);
}

void
XPFThreadedCommand::installStartupItemWithRootDirectory (FSRef *rootDirectory)
{
	if (fDebugOptions & kDisableStartupItem) return;
	FSRef libraryStartupItemsFolder;
	
#ifdef __MACH__
	// On Mac OS X, if we're installing in the current root directory, then we need to move
	// the old startup item (if any) out of the way first, because otherwise we'll try to delete
	// a running binary, which doesn't work.
	char rootPath [1024];
	ThrowIfOSErr_AC (FSRefMakePath (rootDirectory, (UInt8 *) rootPath, 1023));
	if (!strcmp (rootPath, "/")) {
		struct stat sb;
		char dstPath [256];
		for (int x = 0; x < 100; x++) {
			sprintf (dstPath, "/tmp/XPFStartupItem.%d", x);
			if (stat (dstPath, &sb)) break; // we break on an error--i.e. no file!
		}
		XPFSetUID myUID (0);
		rename ("/Library/StartupItems/XPFStartupItem", dstPath);
		// we don't care about rename errors--for instance, if the src doesn't exist, that's fine
	}
#endif 
		
	ThrowIfOSErr_AC (XPFFSRef::getOrCreateStartupDirectory (rootDirectory, &libraryStartupItemsFolder));	
	copyHFSArchivesTo ('hfsS', &libraryStartupItemsFolder);
	
	// We'll restart it later in Mac OS X, in XPFPrefs::Close
}

void
XPFThreadedCommand::copyHFSArchivesTo (ResType type, FSRef *directory)
{
	SInt16 archiveCount = CountResources (type);
	float scale = (float) (fProgressMax - fProgressMin) / (float) archiveCount;
	for (SInt16 x = 1; x <= archiveCount; x++) {
		SetResLoad (false);
		Handle hfsA = GetIndResource (type, x);
		SetResLoad (true);
		SInt16 resourceID;
		GetResInfo (hfsA, &resourceID, NULL, NULL);
		ReleaseResource (hfsA);

		XPFUpdateItem *updateItem = fUpdate->getItemWithResourceID (resourceID);
		if (!updateItem) {
			gLogFile << "Could not find update item for resource ID: " << resourceID << endl_AC;
			continue;
		}
		
		if (updateItem->getIsQualified ()) {
			CResourceStream_AC stream (type, resourceID);
			HFSPlusArchive archive (&stream, &ArchiveFilterGlue, this);
			XPFSetUID myUID (0);
			ThrowIfOSErr_AC (archive.extractArchiveTo (directory));
		} else {
			FSRef existingDir;
			OSErr err = XPFFSRef::getFSRef (directory, (CChar255_AC) updateItem->getResourceName (), &existingDir);
			if (err == noErr) {
				XPFSetUID myUID (0);
				FSRefDeleteDirectory (&existingDir);
			}
		}

		fProgressWindow->setProgressValue (fProgressMin + scale * (float) x, true);
	}
}

void
XPFThreadedCommand::synchronizeWithHelper (bool deleteFirst)
{
	float scale = (float) (fProgressMax - fProgressMin) / (float) fProgressMax;
	unsigned progbase = fProgressMin;

	if ((fRootDisk != fBootDisk) && !(fDebugOptions & kDisableCopyToHelper)) {	
		
		FSRef helperDir;
		ThrowIfOSErr_AC (XPFFSRef::getOrCreateHelperDirectory (
			fBootDisk->getRootDirectory (), 
			(CChar255_AC) fRootDisk->getOpenFirmwareName (true),
			&helperDir,
			true)
		);
			
		if (deleteFirst) {
			XPFSetUID myUID (0);
			FSRefDeleteDirectoryContents (&helperDir);
		}
			
		// Now, copy the mach_kernel file
		// Note the FSRefFileCopy will skip the copy if the files are the same
		FSRef kernelOnRoot;
		ThrowIfOSErr_AC (XPFFSRef::getKernelFSRef (fRootDisk->getRootDirectory (), &kernelOnRoot));
		setCopyingFile ("\pmach_kernel", true);

		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (FSRefFileCopy (&kernelOnRoot, &helperDir, NULL, NULL, 0, false));

		fProgressWindow->setProgressValue (progbase + scale * 200, true);		
		
		// Copy the Extensions and Extensions.mkext
		// Note that the FSRefCopy* routines skip copies that aren't necessary
		FSRef helperSystemLibraryRef;
		FSRef rootSystemLibraryExtensionsRef, rootSystemLibraryExtensionsCacheRef;

		ThrowIfOSErr_AC (XPFFSRef::getOrCreateSystemLibraryDirectory (&helperDir, &helperSystemLibraryRef));
	
		// Check for the Extensions.mkext.
		OSErr rootErr = XPFFSRef::getExtensionsCacheFSRef (fRootDisk->getRootDirectory (), &rootSystemLibraryExtensionsCacheRef);

		if (rootErr == fnfErr) {
			// It doesn't exist on the root. So make sure it doesn't exist on the helper.
			FSRef helperExtensionsCacheRef;
			OSErr helperErr = XPFFSRef::getExtensionsCacheFSRef (&helperDir, &helperExtensionsCacheRef);
			if (helperErr == noErr) ThrowIfOSErr_AC (FSDeleteObject (&helperExtensionsCacheRef));
		} else {
			ThrowIfOSErr_AC (rootErr);
			setCopyingFile ("\pExtensions.mkext", true);
			ThrowIfOSErr_AC (FSRefFileCopy (&rootSystemLibraryExtensionsCacheRef, &helperSystemLibraryRef, NULL, NULL, 0, false));			
		}	
		
		fProgressMin = progbase + scale * 250;
		fProgressMax = progbase + scale * 900;		

		// Check for the extensions directory. It should exist :-)
		ThrowIfOSErr_AC (XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (fRootDisk->getRootDirectory (), &rootSystemLibraryExtensionsRef));
		ThrowIfOSErr_AC (FSRefFilteredDirectoryCopy (&rootSystemLibraryExtensionsRef, &helperSystemLibraryRef, NULL, NULL, 0, true, 
								NULL, CopyFilterGlue, this));

		fProgressMin = fProgressMax;
		fProgressMax = progbase + scale * 950;
		
		// If the root disk was not writeable, then we need to install the extensions in the secondary location
		// on the helper, so that BootX will pick them up
		if (!fRootDisk->getIsWriteable ()) {
			installSecondaryExtensionsWithRootDirectory (&helperDir);
		}
	}		
}