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

#define Inherited XPFThreadedCommand

void
XPFInstallBootXCommand::DoItInProgressWindow ()
{
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingBootX));
	setCopyingFile ("\pBootX", true);
	fProgressMax = 1000;
	fBootDisk->installBootXIfNecessary (true);	
	fProgressWindow->setFinished ();
}

void
XPFInstallExtensionsCommand::DoItInProgressWindow ()
{
	fProgressMax = 1000;
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingExtensions));
	installExtensionsWithRootDirectory (fRootDisk->getRootDirectory ());
	fProgressWindow->setFinished ();
}

void
XPFInstallStartupCommand::DoItInProgressWindow ()
{
	fProgressMax = 1000;
	setDescription (CStr255_AC (kXPFStringsResource, kInstallingStartupItem));
	installStartupItemWithRootDirectory (fRootDisk->getRootDirectory ());
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
	
	fProgressWindow->setFinished ();
}
