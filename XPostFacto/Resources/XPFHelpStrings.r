/*

Copyright (c) 2001 - 2004
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

#include "MacAppTypes.r"
#include "Balloons.r"
#include "XPFHelpStrings.h"

resource 'STR#' (kHelpStringsResource, purgeable) {
	{
/* 1 */  "";

		 "Single user mode stops the boot process at the shell. You generally want it off.";

		 "Verbose mode displays text messages during the boot process. It can be useful to help diagnose problems.";

		 "These settings control debug features in the Mac OS X kernel. "
		 "\"Show Panic Text\" can be useful in diagnosing problems. "
		 "The other settings should generally be off, especially \"Early Breakpoint\".";

/* 5 */	 "Turn off auto-boot if you want to stop at the Open Firmware shell. "
		 "Generally, you will want auto-boot to be on. "
		 "You must select an input-device and output-device before turning auto-boot off.";

		 "Set the input-device if you turn auto-boot off. Otherwise, you can leave it at \"None\".";

		 "Set the output-device if you turn auto-boot off. Otherwise, you can leave it at \"None\".";

		 "Speeds up the boot process by enabling the L2 or L3 cache early. "
		 "Turn it off if it seems to be causing problems.";
		 
		 "Helps boot from some SCSI drives by slowing down the CPU during the boot process. "
		 "Higher settings mean a slower CPU. "
		 "Use 0 unless you have trouble booting (in which case, a setting of 8 is sensible).";
		 
/* 10 */ "Opens a web page that explains the debug settings.";

		 "Select the volume you want to use for Mac OS X. "
		 "You can then restart from this volume, or install Mac OS X to this volume.";
		 
		 "Shows additional information about the selected volume. Not implemented yet.";
		 
		 "Summarizes the NVRAM settings which will be applied for restart or install.";
		 
		 "Change NVRAM settings affecting debug mode, boot mode (verbose or single user), "
		 "CPU settings (cache and throttle), auto-boot, as well as input-device and output-device.";
		 
/* 15 */ "Open a web browser to OWC's web site.";

		 "Choose a 'helper' disk to assist in the boot process. Only select a helper disk if your "
		 "target volume will not boot without one. This is mainly meant for booting from Firewire drives "
		 "or booting from CDs, but in can be useful in some other cases as well.";
		 
		 "Restart Mac OS X from the selected target volume, using the helper disk if you have selected one.";
		 
		 "Install Mac OS X to the selected target volume, using the Install CD you have selected";
		 
		 "Select an Install CD to use for installing Mac OS X.";
		 
/* 20 */ "Opens a web page which explains Open Firmware settings.";

		 "Where you have already installed Mac OS X on the target volume, select items to update. "
		 "Not implemented yet.";
		 
		 "Update the items you have selected. Not implemented yet.";
		 
		 "Choose whether to reboot in Mac OS 9 or Mac OS X. Note that you cannot choose a specific "
		 "Mac OS 9 volume -- this feature is not yet implemented.";
		 
		 "Force Mac OS X 10.3 (Panther) to use video drivers it would otherwise reject as too old. "
		 "This setting has no effect prior to Mac OS X 10.3";
		 
/* 25 */ "Use the PatchedRagoPro.kext to help with video issues on the Wallstreet Powerbooks. "
		 "Turn it off if it seems to be causing problems.";
		 
		 "Click on the warning icon for additional information.";
		 
		 "Select a volume to inspect.";
		 
		 "Specify the bus which this volume is attached to. The bus which XPF would select is underlined. "
		 "If XPF is not choosing the correct bus, this is a bug in XPF which should be fixed.";
	}
};

resource 'hdlg' (kHelpStringsResource, purgeable) {
	HelpMgrVersion, 1, hmDefaultOptions, 0, 5,
	HMSkipItem {},
	{
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 1, kHelpStringsResource, 1, kHelpStringsResource, 1, kHelpStringsResource, 1},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 2, kHelpStringsResource, 2, kHelpStringsResource, 2, kHelpStringsResource, 2},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 3, kHelpStringsResource, 3, kHelpStringsResource, 3, kHelpStringsResource, 3},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 4, kHelpStringsResource, 4, kHelpStringsResource, 4, kHelpStringsResource, 4},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 5, kHelpStringsResource, 5, kHelpStringsResource, 5, kHelpStringsResource, 5},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 6, kHelpStringsResource, 6, kHelpStringsResource, 6, kHelpStringsResource, 6},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 7, kHelpStringsResource, 7, kHelpStringsResource, 7, kHelpStringsResource, 7},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 8, kHelpStringsResource, 8, kHelpStringsResource, 8, kHelpStringsResource, 8},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 9, kHelpStringsResource, 9, kHelpStringsResource, 9, kHelpStringsResource, 9},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 10, kHelpStringsResource, 10, kHelpStringsResource, 10, kHelpStringsResource, 10},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 11, kHelpStringsResource, 11, kHelpStringsResource, 11, kHelpStringsResource, 11},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 12, kHelpStringsResource, 12, kHelpStringsResource, 12, kHelpStringsResource, 12},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 13, kHelpStringsResource, 13, kHelpStringsResource, 13, kHelpStringsResource, 13},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 14, kHelpStringsResource, 14, kHelpStringsResource, 14, kHelpStringsResource, 14},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 15, kHelpStringsResource, 15, kHelpStringsResource, 15, kHelpStringsResource, 15},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 16, kHelpStringsResource, 16, kHelpStringsResource, 16, kHelpStringsResource, 16},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 17, kHelpStringsResource, 17, kHelpStringsResource, 17, kHelpStringsResource, 17},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 18, kHelpStringsResource, 18, kHelpStringsResource, 18, kHelpStringsResource, 18},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 19, kHelpStringsResource, 19, kHelpStringsResource, 19, kHelpStringsResource, 19},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 20, kHelpStringsResource, 20, kHelpStringsResource, 20, kHelpStringsResource, 20},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 21, kHelpStringsResource, 21, kHelpStringsResource, 21, kHelpStringsResource, 21},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 22, kHelpStringsResource, 22, kHelpStringsResource, 22, kHelpStringsResource, 22},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 23, kHelpStringsResource, 23, kHelpStringsResource, 23, kHelpStringsResource, 23},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 24, kHelpStringsResource, 24, kHelpStringsResource, 24, kHelpStringsResource, 24},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 25, kHelpStringsResource, 25, kHelpStringsResource, 25, kHelpStringsResource, 25},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 26, kHelpStringsResource, 26, kHelpStringsResource, 26, kHelpStringsResource, 26},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 27, kHelpStringsResource, 27, kHelpStringsResource, 27, kHelpStringsResource, 27},
		HMStringResItem {{1,1}, {0,0,0,0}, kHelpStringsResource, 28, kHelpStringsResource, 28, kHelpStringsResource, 28, kHelpStringsResource, 28},
	}
};
