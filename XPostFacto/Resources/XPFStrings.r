/*

Copyright (c) 2001, 2002
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
/* 1 */		" is not a SCSI device.";
			" is not formatted as an HFS Plus disk.";
			" is not bootable (Mac OS X is not installed).";
			" is not bootable (unable to find Open Firmware name).";
/* 5 */		"' has no OldWorld support installed.";
			"' is not bootable (BootX has not been installed).";
			" (May fail because partition extends past 8 GB)";
			" is not writeable.";
			"' is not an installation disk.";
/* 10 */	"XPostFacto requires Mac OS 9 or later.";
			"XPostFacto does not know how to patch NVRAM for ";
			"XPostFacto does not work in Mac OS X (yet).";
	}
};
