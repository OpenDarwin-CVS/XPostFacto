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


// Command Numbers
// ---------------

#define cSetVerboseMode					1402
#define cSetSingleUserMode				1403
#define cSetAutoBoot					1404
#define cSetInputDevice					1405
#define cSetOutputDevice				1406

#define cSetDebugBreakpoint				1407
#define cSetDebugPrintf					1408
#define cSetDebugNMI					1409
#define cSetDebugKprintf				1410
#define cSetDebugDDB					1411
#define cSetDebugSyslog					1412
#define cSetDebugARP					1413
#define cSetDebugOldGDB					1414
#define cSetDebugPanicText				1415

#define cShowHelpFile					1416
#define cShowOnlineHelpFile				1417
#define cShowSourceCode					1418

#define cInstallExtensions				1419
#define cInstallBootX					1420
#define cInstallStartupItem				1421

#define cSetUseShortStrings				1422
#define cSetUseShortStringsForInstall	1423
#define cSetEnableCacheEarly			1424
#define cSetThrottle					1425

#define cSetInstallCD					1426
#define cSetTargetDisk					1427
#define cSetHelperDisk					1428

#define cSetCopyMessage					1429
#define cSetCopyInProgress				1430

#define cNewMountedVolume				1432
#define cDeleteMountedVolume			1433
#define cSetVolumeName					1434

#define cShowLogWindow					1435
#define cShowOptionsWindow				1436

#define cSetHelperDiskForTarget			1437

// Views and Windows
// -----------------

#define kTabbedWindow				1007
#define kGridWindow					kDefaultWindowID	// 1001
#define kSplashWindow 				1009
#define kOptionsWindow				1010
#define kProgressWindow				1020
#define kLogWindow					1100

// Menus
// -----

#define mInstall 70
	
// Finder Information
// ------------------

#define kFileType		'pref'
#define kSignature		'usuX'			// Application signature 

// Text Resources
// --------------

#define kCopyrightID	1005
#define kHelpID			1006

#endif