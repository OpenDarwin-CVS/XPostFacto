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

#undef Inherited
#define Inherited XPFRestartCommand

XPFInstallCommand::XPFInstallCommand (XPFPrefs *prefs)
	: XPFRestartCommand (prefs)
{
	fInstallCD = prefs->getInstallCD ();
	
	// Substitute the "install" versions of the settings
	
	fBootDevice = prefs->getBootDeviceForInstall ();
	fBootCommand = prefs->getBootCommandForInstall ();
	fBootFile = prefs->getBootFileForInstall ();
	fInputDevice = prefs->getInputDeviceForInstall ();
	fOutputDevice = prefs->getOutputDeviceForInstall ();
	fUseShortStrings = prefs->getUseShortStringsForInstall ();
}

void
XPFInstallCommand::DoItThreaded ()
{	
	// For the most part, we treat this as a special-case of a "helper" restart.
	// It's as if the Install CD were the target, and the target the helper.
	// But we do a little bit of "pre-install" first.
						
	// We copy BootX from the CD, to make the Installer think that Mac OS X is already
	// installed, so that it won't move the extensions we pre-install
		
	FSRef coreServicesFolder, cdBootX;
		
	ThrowIfOSErr_AC (getOrCreateCoreServicesDirectory (fTargetDisk->getRootDirectory (), &coreServicesFolder));
		
	OSErr err = getBootXFSRef (fInstallCD->getRootDirectory (), &cdBootX);
	if (err != fnfErr) {
		ThrowIfOSErr_AC (err);
		setCopyingFile ("\pBootX");
		XPFSetUID myUID (0);
		err = FSRefFileCopy (&cdBootX, &coreServicesFolder, NULL, NULL, 0, false);
		if (err != dupFNErr) ThrowIfOSErr_AC (err);
	}
	
	// And we install the extensions and startup item to the installTarget
	
	installExtensionsWithRootDirectory (fTargetDisk->getRootDirectory ());
	installStartupItemWithRootDirectory (fTargetDisk->getRootDirectory ());
	
	// Now, set up all the Restart stuff
	// The idea is that we'll actually restart from the Install CD, and use
	// the target disk (or it's helper, if it needs one) as the helper
	// That way, I don't need separate logic for installs and helpers.
	// So we switch the settings to make XPFRestartCommand do the right thing
	
	fHelperDisk = fTargetDisk->getHelperDisk ();
	if (fHelperDisk == NULL) fHelperDisk = fTargetDisk;
	fTargetDisk = fInstallCD;
	
	XPFRestartCommand::DoItThreaded ();
}
