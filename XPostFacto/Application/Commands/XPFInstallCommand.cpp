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

#include "XPFInstallCommand.h"
#include "MountedVolume.h"
#include "XPFPrefs.h"
#include "NVRAM.h"
#include "FSRefCopying.h"
#include "XPFLog.h"
#include "XPFAuthorization.h"
#include "XPFStrings.h"
#include "XPFApplication.h"
#include "XPFFSRef.h"
#include "XPFPrefs.h"
#include "XPFUpdateCommands.h"
#include "XPFUpdate.h"

#undef Inherited
#define Inherited XPFRestartCommand

XPFInstallCommand::XPFInstallCommand (XPFPrefs *prefs)
	: XPFRestartCommand (prefs, true)
{
}

void
XPFInstallCommand::DoIt ()
{						
	// We copy BootX from the CD, to make the Installer think that Mac OS X is already
	// installed, so that it won't move the extensions we pre-install
	
	MountedVolume *targetDisk = fPrefs->getTargetDisk ();
	MountedVolume *installCD = fPrefs->getInstallCD ();
	
	targetDisk->turnOffIgnorePermissions ();
	installCD->turnOffIgnorePermissions ();
	if (targetDisk->getHelperDisk ()) targetDisk->getHelperDisk()->turnOffIgnorePermissions ();
						
	if (!(((XPFApplication *) gApplication)->getDebugOptions () & kDisableCoreServices)) {
		FSRef coreServicesFolder, cdBootX, targetBootX;
			
		OSErr err = XPFFSRef::getBootXFSRef (targetDisk->getRootDirectory (), &targetBootX, false);
		if (err == fnfErr) {
			ThrowIfOSErr_AC (XPFFSRef::getOrCreateCoreServicesDirectory (targetDisk->getRootDirectory (), &coreServicesFolder));
			err = XPFFSRef::getBootXFSRef (installCD->getRootDirectory (), &cdBootX);
			if (err != fnfErr) {
				if (err) gLogFile << "Error finding BootX on the Install CD" << endl_AC;
				ThrowIfOSErr_AC (err);
				XPFSetUID myUID (0);
				err = FSRefFileCopy (&cdBootX, &coreServicesFolder, NULL, NULL, 0, false);
				if (err != dupFNErr) ThrowIfOSErr_AC (err);
			}
		} else {
			if (err) gLogFile << "Error finding /System/Library/CoreServers/BootX on install target" << endl_AC;
			ThrowIfOSErr_AC (err);
		}
	}
	
	// And we install the extensions and startup item and BootX to the installTarget
	
	XPFUpdate update (targetDisk, targetDisk->getHelperDisk (), installCD);
	
	PerformCommand (TH_new XPFInstallBootXCommand (&update));
	PerformCommand (TH_new XPFInstallExtensionsCommand (&update));
	PerformCommand (TH_new XPFInstallStartupCommand (&update));
		
	// And then do the restart
	
	XPFRestartCommand::DoIt ();
}
