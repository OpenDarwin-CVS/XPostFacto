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

#ifdef __MACH__
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/stat.h>
#endif

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
	: fYielder (1), fItemsToCopy (0), fItemsCopied (0), fProgressMin (0), fProgressMax (0)
{
	fTargetDisk = prefs->getTargetDisk ();
	fHelperDisk = fTargetDisk->getHelperDisk ();
	
	turnOffIgnorePermissionsForVolume (fTargetDisk);
	turnOffIgnorePermissionsForVolume (fHelperDisk);

	fDebugOptions = ((XPFApplication *) gApplication)->getDebugOptions ();
		
	fCopyWord.CopyFrom (kXPFStringsResource, kTheWordCopy, 63);
}	

// Methods to run things threaded

void
XPFThreadedCommand::DoIt ()
{
	fProgressWindow = (XPFProgressWindow *) TViewServer::fgViewServer->NewTemplateWindow (kProgressWindow, NULL);
	XPFCommandThread *thread = TH_new XPFCommandThread (this);
	fRunner = thread;
	fProgressWindow->setThread (thread);
	fProgressWindow->setProgressMax (1000);
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
XPFThreadedCommand::copyFilter (const FSRef *src, Boolean preflight)
{
	fYielder.YieldIfTime ();
	if (preflight) {
		fItemsToCopy++;
	} else {
		fItemsCopied++;
		float scale = (float) (fProgressMax - fProgressMin) / (float) fItemsToCopy;
		fProgressWindow->setProgressValue (fProgressMin + scale * fItemsCopied);
		FSSpec spec;
		FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
	 	setCopyingFile (spec.name);
	}
	return !fRunner->ShutdownRequested ();
}

Boolean 
XPFThreadedCommand::archiveFilter (const FSRef *src, Boolean preflight)
{
	fYielder.YieldIfTime ();
	FSSpec spec;
	FSGetCatalogInfo (src, kFSCatInfoNone, NULL, NULL, &spec, NULL);
 	setCopyingFile (spec.name);
	return !fRunner->ShutdownRequested ();
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

void
XPFThreadedCommand::turnOffIgnorePermissionsForVolume (MountedVolume *volume)
{
#ifdef __MACH__
	if (!volume) return;
	char path[256];
	ThrowIfOSErr_AC (FSRefMakePath (volume->getRootDirectory (), (UInt8 *) path, 255));
	XPFSetUID myUID (0);
	pid_t pid = fork ();
	if (pid) {
		if (pid != -1) {
			int status;
			waitpid (pid, &status, 0);
		}
	} else {
		setuid (0);	// because vsdbutil checks the uid, rather than the euid
		execl ("/usr/sbin/vsdbutil", "vsdbutil", "-a", path, NULL);
		ThrowException_AC (kInternalError, 0);	// the execl shouldn't return
	}
#endif
}

// Install methods

void
XPFThreadedCommand::updateExtensionsCacheForRootDirectory (FSRef *rootDirectory)
{
	if (fDebugOptions & kDisableExtensionsCache) return;
#ifdef __MACH__
	fProgressWindow->setProgressMax (0);
	fProgressWindow->setProgressMin (0);
	fProgressWindow->setProgressValue (0);
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kUpdatingExtensionsCache));

	char rootPath [256];
	ThrowIfOSErr_AC (FSRefMakePath (rootDirectory, (UInt8 *) rootPath, 256));
	XPFSetUID myUID (0);
	if (!chdir (rootPath)) {
		pid_t pid = fork ();
		if (pid) {
			if (pid != -1) {
				int status;
				while (!wait4 (pid, &status, WNOHANG, 0)) fYielder.YieldNow ();
			}
		} else {
			execl ("/usr/sbin/kextcache", "kextcache", "-l", "-m", "System/Library/Extensions.mkext", "System/Library/Extensions", NULL);
			ThrowException_AC (kInternalError, 0);	// the execl shouldn't return
		}
	}
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
	char rootPath [256];
	bool moveAndRestart = false;
	ThrowIfOSErr_AC (FSRefMakePath (rootDirectory, (UInt8 *) rootPath, 255));
	if (!strcmp (rootPath, "/")) moveAndRestart = true;
	
	if (moveAndRestart) {
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

#ifdef __MACH__
	if (moveAndRestart) {
		XPFSetUID myUID (0);
		system ("/Library/StartupItems/XPFStartupItem/XPFStartupItem restart");
	}
#endif
}

void
XPFThreadedCommand::copyHFSArchivesTo (ResType type, FSRef *directory)
{
	SInt16 archiveCount = CountResources (type);
	float scale = (float) (fProgressMax - fProgressMin) / (float) archiveCount;
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
		HFSPlusArchive archive (&stream, &ArchiveFilterGlue, this);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (directory));

		fProgressWindow->setProgressValue (fProgressMin + scale * (float) x);
	}
}
