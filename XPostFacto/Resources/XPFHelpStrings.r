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

#include "MacAppTypes.r"
#include "XPFHelpStrings.h"

type 'hlps' {
	wide array HelpStrings {
		literal longint;	// the identifier
		integer;			// Index in STR# of string
		fill word;
	};
};

resource 'hlps' (kHelpStringsResource, purgeable) {
	{
		'null', 1;
		'sinm', 2;
		'verm', 3;
		'debg', 4;
		'auto', 5;
		'inpd', 6;
		'outd', 7;
		'cach', 8;
		'thro', 9;
		'exdb', 10;
		'selt', 11;
		'info', 12;
		'nvrm', 13;
		'opti', 14;
		'owcp', 15;
		'hlpd', 16;
		'rest', 17;
		'inst', 18;
		'incd', 19;
		'exof', 20;
		'selu', 21;
		'selb', 22;
	}
};

resource 'STR#' (kHelpStringsResource, purgeable) {
	{
/* 1 */  "";

		 "Single user mode stops the boot process at the shell. You generally want it off.";

		 "Verbose mode displays text messages during the boot process. It can be useful to help diagnose problems.";

		 "These settings control debug features in the Mac OS X kernel. "
		 "\"Show Panic Text\" can be useful in diagnosing problems. "
		 "The other settings should generally be off, especially \"Early Breakpoint\".";

/* 5 */	 "Turn off auto-boot if you want to stop at the Open Firmware shell. "
		 "Generally, you will want auto-boot to be on.";

		 "Set the input-device if you turn auto-boot off. Otherwise, you can leave it at \"None\".";

		 "Set the output-device if you turn auto-boot off. Otherwise, you can leave it at \"None\".";

		 "Speeds up the boot process by enabling the L2 or L3 cache early. "
		 "Turn it off if it seems to be causing problems."
		 "Not implemented yet.";
		 
		 "Helps boot from some SCSI drives by slowing down the CPU during the boot process. "
		 "Higher settings mean a slower CPU. "
		 "Use 0 unless you have trouble booting (in which case, a setting of 8 is sensible).";
		 
/* 10 */ "Opens a web page that explains the debug settings.";

		 "Select the volume you want to use for Mac OS X. "
		 "Use the menus to the right to restart from this volume, or install Mac OS X to this volume.";
		 
		 "Shows additional information about the selected volume. Not implemented yet.";
		 
		 "Summarizes the NVRAM settings which will be applied for restart or install.";
		 
		 "Change NVRAM settings affecting debug mode, boot mode (verbose or single user), "
		 "CPU settings (cache and throttle), auto-boot, as well as input-device and output-device.";
		 
/* 15 */ "Open a web browser to OWC's web site.";

		 "Choose a 'helper' disk to assist in the boot process. Only select a helper disk if your "
		 "target volume will not boot without one. This is mainly meant for booting from Firewire drives "
		 "or booting from CDs, but in can be useful in some other cases as well.";
		 
		 "Restart Mac OS X from the selected target volume, using the helper disk if you have selected one. "
		 "To restart from an Install CD, select the destination as the target volume, and then use the "
		 "'Install' button below.";
		 
		 "Install Mac OS X to the selected target volume, using the Install CD selected in the 'Use CD' menu.";
		 
		 "Select an Install CD to use for installing Mac OS X.";
		 
/* 20 */ "Opens a web page which explains Open Firmware settings.";

		 "Where you have already installed Mac OS X on the target volume, select items to update."
		 "Not implemented yet.";
		 
		 "Update the items you have selected. Not implemented yet.";
	}
};
