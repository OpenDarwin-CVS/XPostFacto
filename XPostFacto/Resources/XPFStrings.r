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
			"XPostFacto does not know how to patch NVRAM for ";
			
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
	}
};
