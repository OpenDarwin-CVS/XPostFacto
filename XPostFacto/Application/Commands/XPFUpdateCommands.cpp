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

#include "XPFUpdateCommands.h"

#include "MountedVolume.h"
#include "XPFProgressWindow.h"
#include "XPFStrings.h"
#include "XPFUpdate.h"
#include "XPFFSRef.h"
#include "XPFAuthorization.h"
#include "FSRefCopying.h"
#include "XPFLog.h"

#include <stdio.h>

#define Inherited XPFThreadedCommand

void
XPFInstallBootXCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingBootX));
	setCopyingFile ("\pBootX", true);
	fProgressMax = 1000;
	fBootDisk->installBootX ();	
	fProgressWindow->setFinished ();
}

void
XPFInstallExtensionsCommand::DoItInProgressWindow ()
{
	fProgressMax = 1000;
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingExtensions));
	if (!fRootDisk->getHasInstaller()) installExtensionsWithRootDirectory (fRootDisk->getRootDirectory (), !fUpdate->getInstallCD ());
	fProgressWindow->setFinished ();
}

void
XPFInstallStartupCommand::DoItInProgressWindow ()
{
	fProgressMax = 1000;
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingStartupItem));
	if (!fRootDisk->getHasInstaller()) installStartupItemWithRootDirectory (fRootDisk->getRootDirectory ());
	fProgressWindow->setFinished ();
}

void
XPFRecopyHelperFilesCommand::DoItInProgressWindow ()
{
	fProgressMax = 1000;
	setDescription (CStr255_AC (kXPFStringsResource, kRecopyingHelperFiles));
	synchronizeWithHelper (true);
	fProgressWindow->setFinished ();
}

void
XPFUpdateCommand::DoItInProgressWindow ()
{
	bool updateExtensionsCache = false;

	setDescription (CStr255_AC (kXPFStringsResource, kUpdating));
	
	XPFUpdateItemList *list = fUpdate->getItemList ();
	
	fProgressWindow->setProgressMax (list->GetSize ());

	for (int x = 1; x <= list->GetSize (); x++) {
		fProgressWindow->setProgressValue (x, true);
		XPFUpdateItem *item = list->At (x);
		if (item->getAction () != kActionNone) {
			CStr255_AC status (kXPFStringsResource, kUpdating);
			status += " ";
			status += item->getResourceName ();
			setStatusMessage (status, true);
			
			item->doUpdate ();
			
			SInt16 resourceID = item->getResourceID ();
			if (resourceID && (resourceID < 200)) updateExtensionsCache = true;
		}	
	}
	
	if (updateExtensionsCache) updateExtensionsCacheForRootDirectory (fUpdate->getTarget()->getRootDirectory ());

	fProgressWindow->setFinished ();
}

void
XPFBlessMacOS9SystemFolderCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC (kXPFStringsResource, kBlessingSystemFolder));
	fUpdate->getTarget()->blessMacOS9SystemFolder ();	
	fProgressWindow->setFinished ();
}

void
XPFUninstallCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC (kXPFStringsResource, kUninstalling));
	
	XPFUpdateItemList *list = fUpdate->getItemList ();
	
	fProgressWindow->setProgressMax (list->GetSize ());

	for (int x = 1; x <= list->GetSize (); x++) {
		fProgressWindow->setProgressValue (x, true);
		XPFUpdateItem *item = list->At (x);

		CStr255_AC status (kXPFStringsResource, kUninstalling);
		status += " ";
		status += item->getResourceName ();
		setStatusMessage (status, true);
			
		item->uninstall ();
	}
	
	updateExtensionsCacheForRootDirectory (fUpdate->getTarget()->getRootDirectory ());
	
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kDeletingXPFCache), true);

	XPFSetUID myUID (0);
	FSRef xpfDirectory;	
	OSErr err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getTarget()->getRootDirectory(), &xpfDirectory, false);
	if (err == noErr) FSRefDeleteDirectoryContents (&xpfDirectory, true);
	if (fUpdate->getHelper ()) {
		err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getHelper()->getRootDirectory(), &xpfDirectory, false);
		if (err == noErr) FSRefDeleteDirectoryContents (&xpfDirectory, true);	
	}
	
	fProgressWindow->setFinished ();
}

void
XPFEmptyCacheCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC ((ResNumber) kXPFStringsResource, kDeletingXPFCache));
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kDeletingXPFCache), true);
	fProgressWindow->setProgressMax (2);

	XPFSetUID myUID (0);
	FSRef xpfDirectory;	
	OSErr err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getTarget()->getRootDirectory(), &xpfDirectory, false);
	if (err == noErr) ThrowIfOSErr_AC (FSRefDeleteDirectoryContents (&xpfDirectory, true));
	fProgressWindow->setProgressValue (1, true);
	if (fUpdate->getHelper ()) {
		err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getHelper()->getRootDirectory(), &xpfDirectory, false);
		if (err == noErr) ThrowIfOSErr_AC (FSRefDeleteDirectoryContents (&xpfDirectory, true));	
	}
	
	fProgressWindow->setFinished ();
}

void
XPFCheckPermissionsCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC ((ResNumber) kXPFStringsResource, kCheckingPermissions));
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kCheckingPermissions), true);
	fProgressWindow->setProgressMax (2);

	XPFSetUID myUID (0);
	FSRef xpfDirectory;	
	OSErr err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getTarget()->getRootDirectory(), &xpfDirectory, false);
	if (err == noErr) checkPermissions (&xpfDirectory);
	fProgressWindow->setProgressValue (1, true);
	if (fUpdate->getHelper ()) {
		err = XPFFSRef::getOrCreateXPFDirectory (fUpdate->getHelper()->getRootDirectory(), &xpfDirectory, false);
		if (err == noErr) checkPermissions (&xpfDirectory);
	}
	
	fProgressWindow->setFinished ();
}

#define ITERATE_OVER	16

OSErr
XPFCheckPermissionsCommand::checkPermissions (FSRef *directory)
{
	OSErr error;
	FSCatalogInfo catInfo;
	error = FSGetCatalogInfo (directory, kFSCatInfoNodeFlags, &catInfo, NULL, NULL, NULL);
	if (error != noErr) return error;
	if (!(catInfo.nodeFlags & kFSNodeIsDirectoryMask)) return errFSNotAFolder;

	bool done = false;
	FSIterator iterator;
	error = FSOpenIterator (directory, kFSIterateFlat, &iterator);
	if (error != noErr) return error;

	while (!done) {
		ItemCount actualObjects;
		FSRef items [ITERATE_OVER];
		FSSpec specs [ITERATE_OVER];
		FSCatalogInfo catInfos [ITERATE_OVER];

		error = FSGetCatalogInfoBulk (iterator, ITERATE_OVER, &actualObjects, NULL, 
					kFSCatInfoNodeFlags | kFSCatInfoPermissions, catInfos, items, specs, NULL);
											
		if (error == errFSNoMoreItems) {
			error = noErr;
			done = true;
		} else if (error != noErr) {
			break;
		}
		
		for (int x = 0; x < actualObjects; x++) {
			if (catInfos[x].nodeFlags & kFSNodeIsDirectoryMask) {
				error = checkPermissions (&items[x]);
			} else {
				if ((catInfos[x].permissions[0] != 0) || (catInfos[x].permissions[1] != 0) ||
							(catInfos[x].permissions[2] & 0022)) {
					char temp[64];
					snprintf (temp, 63, "0%o", catInfos[x].permissions[2] & 0777);
					gLogFile << "--> " << (CChar63_AC) specs[x].name << " was "
						<< catInfos[x].permissions[0] << "-" 
						<< catInfos[x].permissions[1] << "-" << temp << endl_AC;
					catInfos[x].permissions[0] = 0;
					catInfos[x].permissions[1] = 0;
					catInfos[x].permissions[2] &= ~0022;	// turn off group and world write
					XPFSetUID myUID (0);
					FSSetCatalogInfo (&items[x], kFSCatInfoPermissions, &catInfos[x]);
					#ifdef __MACH__
						char path[1024];
						error = FSRefMakePath (&items[x], (UInt8 *) path, 1023);
						if (error == noErr) chown (path, 0, 0);
					#endif
				}
			}
		}
		
		if (error != noErr) return error;
	}

	FSCloseIterator (iterator);	
	return error;
}

void
XPFFixSymlinksCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC ((ResNumber) kXPFStringsResource, kFixingSymbolicLinks));
	setStatusMessage (CStr255_AC ((ResNumber) kXPFStringsResource, kFixingSymbolicLinks), true);
	fProgressWindow->setProgressMax (3);
	
	MountedVolume *target = fUpdate->getTarget();
	
	fProgressWindow->setProgressValue (1, true);
	target->fixSymlinkAtPath ("etc");

	fProgressWindow->setProgressValue (2, true);
	target->fixSymlinkAtPath ("tmp");

	fProgressWindow->setProgressValue (3, true);
	target->fixSymlinkAtPath ("var");	
	
	target->checkSymlinks ();
	fProgressWindow->setFinished ();
}