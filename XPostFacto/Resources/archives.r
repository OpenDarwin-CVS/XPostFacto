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

/*

	Sets up the resources necessary to copy the kernel extensions
	to the target volume.

*/

#include "XPostFacto.h"

// Load archives into resource fork

Read 'hfsA' (130, "AppleMaceEthernet") ":Derived:Extensions:AppleMaceEthernet.hfs";
Read 'plst' (130, "AppleMaceEthernet") ":Derived:Extensions:plist:AppleMaceEthernet.plist";

Read 'hfsA' (140, "OpenPMU") ":Derived:Extensions:OpenPMU.hfs";
Read 'plst' (140, "OpenPMU") ":Derived:Extensions:plist:OpenPMU.plist";

Read 'hfsA' (141, "PowerStarCPU") ":Derived:Extensions:PowerStarCPU.hfs";
Read 'plst' (141, "PowerStarCPU") ":Derived:Extensions:plist:PowerStarCPU.plist";

Read 'hfsA' (128, "AppleGrandCentral") ":Derived:Extensions:AppleGrandCentral.hfs";
Read 'plst' (128, "AppleGrandCentral") ":Derived:Extensions:plist:AppleGrandCentral.plist";

Read 'hfsA' (129, "ApplePowerSurgePE") ":Derived:Extensions:ApplePowerSurgePE.hfs";
Read 'plst' (129, "ApplePowerSurgePE") ":Derived:Extensions:plist:ApplePowerSurgePE.plist";

Read 'hfsA' (131, "AppleCurio") ":Derived:Extensions:AppleCurio.hfs";
Read 'plst' (131, "AppleCurio") ":Derived:Extensions:plist:AppleCurio.plist";

Read 'hfsA' (132, "PatchedIOSCSICDDrive") ":Derived:Extensions:PatchedIOSCSICDDrive.hfs";
Read 'plst' (132, "PatchedIOSCSICDDrive") ":Derived:Extensions:plist:PatchedIOSCSICDDrive.plist";

Read 'hfsA' (133, "PatchedAppleNVRAM") ":Derived:Extensions:PatchedAppleNVRAM.hfs";
Read 'plst' (133, "PatchedAppleNVRAM") ":Derived:Extensions:plist:PatchedAppleNVRAM.plist";

Read 'hfsA' (134, "PatchedSCSIDeviceType05") ":Derived:Extensions:PatchedSCSIDeviceType05.hfs";
Read 'plst' (134, "PatchedSCSIDeviceType05") ":Derived:Extensions:plist:PatchedSCSIDeviceType05.plist";

Read 'hfsA' (135, "PowerSurgeCPU") ":Derived:Extensions:PowerSurgeCPU.hfs";
Read 'plst' (135, "PowerSurgeCPU") ":Derived:Extensions:plist:PowerSurgeCPU.plist";

Read 'hfsA' (136, "AppleOHare") ":Derived:Extensions:AppleOHare.hfs";
Read 'plst' (136, "AppleOHare") ":Derived:Extensions:plist:AppleOHare.plist";

Read 'hfsA' (137, "OHareATA") ":Derived:Extensions:OHareATA.hfs";
Read 'plst' (137, "OHareATA") ":Derived:Extensions:plist:OHareATA.plist";

Read 'hfsA' (138, "ApplePowerStarPE") ":Derived:Extensions:ApplePowerStarPE.hfs";
Read 'plst' (138, "ApplePowerStarPE") ":Derived:Extensions:plist:ApplePowerStarPE.plist";

Read 'hfsA' (139, "ApplePowerExpressPE") ":Derived:Extensions:ApplePowerExpressPE.hfs";
Read 'plst' (139, "ApplePowerExpressPE") ":Derived:Extensions:plist:ApplePowerExpressPE.plist";

Read 'hfsA' (142, "PatchedNDRVSupport") ":Derived:Extensions:PatchedNDRVSupport.hfs";
Read 'plst' (142, "PatchedNDRVSupport") ":Derived:Extensions:plist:PatchedNDRVSupport.plist";

// Read 'hfsA' (143, "OWCCacheConfig") ":Derived:Extensions:OWCCacheConfig.hfs";
// Read 'plst' (143, "OWCCacheConfig") ":Derived:Extensions:plist:OWCCacheConfig.plist";

// Startup Item

Read 'hfsS' (200, "XPFStartupItem") ":Derived:StartupItems:XPFStartupItem.hfs";
Read 'plst' (200, "XPFStartupItem") ":Derived:StartupItems:plist:XPFStartupItem.plist";

// Set up the BootX file for extraction

Read 'BooX' (128, "bootX.xcoff") ":Derived:bootx.xcoff";

// Other shared stuff

Read 'TEXT' (kCopyrightID, "Copyright") "copyright.h";
Read 'TEXT' (kHelpID, "Help") "::owc:XPostFacto3.html";

