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

#include "XPFStrings.h"

#include "MacAppTypes.r"

resource 'STR#' (kXPFStringsResource, purgeable) {
	{
/* 1 */		"Not a bootable device.";
			"Not formatted as an HFS Plus disk.";
			"Mac OS X is not installed.";
			"Unable to find Open Firmware name.";
/* 5 */		"No OldWorld support installed.";
			"BootX has not been installed.";
			"(May fail because partition extends past 8 GB)";
			"Not writeable.";
			"Not an installation disk.";
/* 10 */	"XPostFacto requires Mac OS 9 or later.";
			"XPostFacto does not work with this model: ";
			
			"XPostFacto does not work in Mac OS X (yet). "
			"The one useful thing it can do is install the "
			"current version of the extensions. For this, use the "
			"'Install Extensions' command in the 'File' menu.";

			"^0 Help";
			"Online Help";
/* 15 */	"Online Source Code";
			"can only be used as an installer.";
			"requires a helper disk.";
			"Show Log Window";
			"Copying";
/* 20 */	"Preparing to Restart";
			"Preparing to Install";
			"Installing BootX";
			"Installing Extensions";
			"Installing Startup Item";
/* 25 */	"Updating Extensions Cache";
			"Show Debug Options";
			"Hide Debug Menu";
			
			"Disable Restart";
			"Disable NVRAM Writing";
/* 30 */	"Disable BootX Install";
			"Disable StartupItem Install";
			"Disable Extensions Install";
			"Disable Extensions.mkext Update";
			"Disable CoreServices Copy";
/* 35 */	"Disable Helper Copy";
			"Keep Helper Visible";
			"Recopying Helper Files";
			"Updating";
			
			"There is not enough NVRAM to store these settings.";
/* 40 */	"There is not enough NVRAM to store these installation settings.";
			
			"Install";
			"Delete";
			
			"Synchronizing";
			"Uninstalling";
/* 45 */	"Blessing Mac OS 9 System Folder";
			"Deleting XPostFacto Cache Files";
			"Checking Permissions";
			
			"Register XPostFacto";
			
			"G3/G4 CPU upgrade required";
			
/* 50 */	"Not installed";

			"The partition map has fewer patch partitions than expected. You may need to reformat "
			"the drive in Mac OS 9.x., using Apple's Drive Setup or Intech's Hard Disk SpeedTools. "
			"Alternatively, you may be able to use a 'Helper Drive' to boot from this volume.";
			
			"The symbolic links /etc, /tmp, or /var are either missing or corrupted. "
			"You can attempt to fix this with the 'Fix Symbolic Links' command in the 'Install' menu.";

			"The symbolic links /etc, /tmp, or /var are either missing or corrupted. ";
				
			"Fixing symbolic links";
			
/* 55 */	"This HFS+ volume has no HFS wrapper, so the boot process may complain that "
			"there is no bootable HFS volume. Erasing or reformatting the volume in Mac OS 9.x "
			"can help. Alternatively, you may be able to use a 'Helper Drive' to boot from this volume.";
			
			"The Mac OS X Installer will complain that this volume extends past 8 GB, and refuse to install. "
			"One workaround is to copy an installation from another drive, using Carbon Copy Cloner.";
			
			"This volume cannot be used to boot Mac OS X, because it extends past the 8 GB mark on the disk."; 
			
			"Requires installation";
			
			"Could not determine partition number";
			
			"You have specified a non-default bus for this volume. This is only necessary in cases where "
			"XPostFacto would otherwise assign the volume to the wrong bus.";
			
			"XPostFacto";
			"Save Current Log";
			"XPostFacto Log";
			
			"Check log for error";
			
			"One of the extension cache files appears to be invalid (see log for details). You may be "
			"able to fix this problem with the 'Install Extensions' command.";
	}
};
