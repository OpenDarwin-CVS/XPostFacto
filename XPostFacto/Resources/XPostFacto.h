/*

Copyright (c) 2002
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

#ifndef __XPOSTFACTO_H__
#define __XPOSTFACTO_H__

// Constants for Menu IDs
// ----------------------

#define mAdvanced		4
#define mOpenFirmware 	5
#define mInputDevice 	6
#define mOutputDevice	7
#define mThrottle		8
#define mDebug			9

#define hmInputDevice  "\0x06"
#define hmOutputDevice "\0x07"


// Command Numbers For Menu Items
// ------------------------------

//#define cCreateArchive			1000
//#define cExtractArchive			1001
#define cToggleVerboseMode		1002
#define cToggleSingleUserMode 	1003
#define cReinstallBootX			1004
#define cReinstallExtensions 	1005
#define cToggleAutoBoot			1006
#define cSetupL2Cache			1007
#define cSelectInputDevice 		1008
#define cSelectOutputDevice		1009
#define cSetInputDeviceNone		1010
#define cSetOutputDeviceNone	1011
#define cSetThrottleNone		1012

#define cToggleDebugBreakpoint	1013
#define cToggleDebugPrint		1014
#define cToggleDebugNMI			1015
#define cToggleDebugkprintf		1016
#define cToggleDebugUseDDB		1017
#define cToggleDebugSystemLog	1018
#define cToggleDebugARP			1019
#define cToggleDebugOldGDB		1020

#define cFirstInputDevice		1100
#define cFirstOutputDevice		1200

#define cThrottleBase			1300
#define kNumThrottleOptions		24


// Command Numbers for identifying changes
// (I suppose these should be real commands someday)
// ---------------------------------

#define cSetInstallDisk			1400
#define cSetBootDisk			1401
#define cSetCopyMessage			1402
#define cSetCopyInProgress		1403
#define cVolumeListChange		1404


// Finder Information
// ------------------

#define kSignature			'usuX'			// Application signature 


#endif