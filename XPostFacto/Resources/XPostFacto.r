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

//--------------------------------------------------------------------------------------------------
// INCLUDE FILES 
//--------------------------------------------------------------------------------------------------

// Standard Includes 

// MacAppTypes.r must be first
#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif

#ifndef __Balloons__
#include "Balloons.r"
#endif

//#ifndef __MACAPPVERSION__
//#include "MacAppVersion.h"
//#endif

#ifndef __ViewTypes__
#include "ViewTypes.r"
#endif

#ifndef __FILETYPESANDCREATORS_R__
#include "FileTypesAndCreators.r"
#endif

#include "XPostFacto.h"

//--------------------------------------------------------------------------------------------------
// CONSTANTS 
//--------------------------------------------------------------------------------------------------

// Bundle Constants
// ------------------

#define kBundleID						128
#define kApplicationID					128

// STR# resources
// --------------

#define kXPFWindowTitles			1001

// Text styles
// --------------

#define kSystemTextStyle				1000

// 'View' resources
// --------------

#define kXPFWindowID				kDefaultWindowID

Read 'TEXT' (kCopyrightID, "Copyright") "copyright.h";
Read 'TEXT' (kHelpID, "Help") "::XPostFacto.html";

// Memory Management Resources
// ---------------------------

#define kXPFSeg	1000
#define kXPFMem	1000
#define kXPFRes	1000

// Default includes
// ---------------------------

#if !defined(THINK_Rez) && !defined(MW_Rez)
	#if qDebug | qPerform
		include kDebugRsrcName not 'ckid';
	#endif

	include kDialogRsrcName not 'ckid';
	include kMacAppRsrcName not 'ckid';

	// Includes for Building Blocks

	include kPrintingRsrcName not 'ckid';

	// Include Code Resources

	#if !qNoCode
		include $$Shell("ObjApp")$$Shell("XAppName") 'CODE';
		#if qModelCFM
			include $$Shell("ObjApp")$$Shell("XAppName") 'cfrg';
			include $$Shell("ObjApp")$$Shell("XAppName") 'rseg';
		#endif
	#endif

	#define kApplicationName	TARGET_APPLICATION_NAME
#else
	include kDefaultRsrcName  'CMNU' (mApple);			// Grab the default Apple menus
//	include kDefaultRsrcName  'CMNU' (mFile);			// Grab the default File menus
//	include kDefaultRsrcName  'CMNU' (mEdit);			// Grab the default Edit menu
//	include kDefaultRsrcName  'ALRT' (phAboutApp);		// Grab the default about box
	include kDefaultRsrcName  'aete' (0);				// Grab the default AppleEvent terminology
//	include kDefaultRsrcName  'STR#' (kDefaultCredits);	// Grab the default credits
//	include kDefaultRsrcName  'vers' (2);				// Overall package

//	include kDefaultRsrcName  'ICN#' (kApplicationID);	// MacApp Family large black & white	
//	include kDefaultRsrcName  'ics#' (kApplicationID);	// MacApp Family small black & white	
//	include kDefaultRsrcName  'ics4' (kApplicationID);	// MacApp Family small 4 bit
//	include kDefaultRsrcName  'ics8' (kApplicationID);	// MacApp Family small 4 bit
//	include kDefaultRsrcName  'icl4' (kApplicationID);	// MacApp Family large 4 bit
//	include kDefaultRsrcName  'icl8' (kApplicationID);	// MacApp Family large 8 bit


	#if qDebug | qPerform
		include kDebugRsrcName not 'ckid';
	#endif

	include kDialogRsrcName not 'ckid';
	include kGeneralRsrcName not 'ckid';

	#define kApplicationName	$$Shell("XAppName")
#endif //!THINK_Rez

//--------------------------------------------------------------------------------------------------
// RESOURCES FOR MENUS 
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Menu Bars 
//--------------------------------------------------------------------------------------------------

resource 'MBAR' (kMBarDisplayed,
#if qNames
"XPostFacto MBAR",
#endif
	purgeable) {
	{ 
		mApple; 
		mFile; 
		mAdvanced; 
		mOpenFirmware; 
		mThrottle;
		mDebug;
	}
};

resource 'MBAR' (kMBarHierarchical,
#if qNames
"kMBarHierarchical",
#endif
nonpurgeable) {
	{
		mInputDevice;
		mOutputDevice;
	}
};



//--------------------------------------------------------------------------------------------------
// Menus 
//--------------------------------------------------------------------------------------------------

resource 'CMNU' (mFile,
#if qNames
"mFile",
#endif
purgeable) {
	mFile,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"File",
	{
//	"New",				noIcon, "N",	noMark, plain, cNew;
//	"Open…",			noIcon, "O",	noMark, plain, cOpen;
//	"-",				noIcon, noKey,	noMark, plain, nocommand;
//	"Close",			noIcon, "W",	noMark, plain, cClose;
//	"Save",				noIcon, "S",	noMark, plain, cSave;
//	"Save As…",			noIcon, noKey,	noMark, plain, cSaveAs;
//	"Revert",			noIcon, noKey,	noMark, plain, cRevert;
//	"-",				noIcon, noKey,	noMark, plain, nocommand;
//	"Page Setup…",		noIcon, noKey,	noMark, plain, cPageSetup;
//	"Print…",			noIcon, "P",	noMark, plain, cPrint;
//	"Print One Copy",	noIcon, noKey,	noMark, plain, cPrintOne;
//	"Create Archive…", noIcon, noKey, noMark, plain, cCreateArchive;
//	"Extract Archive…", noIcon, noKey, noMark, plain, cExtractArchive;
//	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Quit",				noIcon, "Q",	noMark, plain, cQuit
	}
};

resource 'CMNU' (mAdvanced,
#if qNames
"mAdvanced",
#endif
purgeable) {
	mAdvanced,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Advanced",
	{
		"Verbose Mode", noIcon, noKey, noMark, plain, cToggleVerboseMode;
		"Single User Mode", noIcon, noKey, noMark, plain, cToggleSingleUserMode;
		"-",				noIcon, noKey,	noMark, plain, nocommand;
		"Reinstall BootX", noIcon, noKey, noMark, plain, cReinstallBootX;
		"Reinstall Extensions", noIcon, noKey, noMark, plain, cReinstallExtensions;
#if 0	// this doesn't work yet
		"-", 				noIcon, noKey,	noMark, plain, nocommand;
		"Setup L2 Cache", noIcon, noKey, noMark, plain, cSetupL2Cache;
#endif
	}
};

resource 'CMNU' (mDebug,
#if qNames
"mDebug",
#endif
purgeable) {
	mDebug,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Debug",
	{
		"Early Breakpoint", noIcon, noKey, noMark, plain, cToggleDebugBreakpoint;
		"Debug Print", noIcon, noKey, noMark, plain, cToggleDebugPrint;
		"Enable NMI", noIcon, noKey, noMark, plain, cToggleDebugNMI;
		"Enable kprintf", noIcon, noKey, noMark, plain, cToggleDebugkprintf;
		"Use DDB", noIcon, noKey, noMark, plain, cToggleDebugUseDDB;
		"Extra Diagnostics", noIcon, noKey, noMark, plain, cToggleDebugSystemLog;
		"Debugger ARP", noIcon, noKey, noMark, plain, cToggleDebugARP;
		"Support old GDB", noIcon, noKey, noMark, plain, cToggleDebugOldGDB;
	}
};

resource 'CMNU' (mOpenFirmware,
#if qNames
"mOpenFirmware",
#endif
purgeable) {
	mOpenFirmware,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Open Firmware",
	{
		"auto-boot?", noIcon, noKey, noMark, plain, cToggleAutoBoot;
		"input-device", noIcon, kHierarchicalMenu, hmInputDevice, plain, cSelectInputDevice;
		"output-device", noIcon, kHierarchicalMenu, hmOutputDevice, plain, cSelectOutputDevice;
	}
};

resource 'CMNU' (mInputDevice,
#if qNames
"mInputDevice",
#endif
purgeable) {
	mInputDevice,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Input Device",
	{
		"None", noIcon, noKey, noMark, plain, cSetInputDeviceNone;
		"-", noIcon, noKey, noMark, plain, noCommand;
	}
};

resource 'CMNU' (mOutputDevice,
#if qNames
"mOutputDevice",
#endif
purgeable) {
	mOutputDevice,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Output Device",
	{
		"None", noIcon, noKey, noMark, plain, cSetOutputDeviceNone;
		"-", noIcon, noKey, noMark, plain, noCommand;
	}
};

resource 'CMNU' (mThrottle,
#if qNames
"mThrottle",
#endif
purgeable) {
	mThrottle,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Throttle",
	{
		"None", noIcon, noKey, noMark, plain, cSetThrottleNone;
		"-", noIcon, noKey, noMark, plain, noCommand;
	}
};

/*
resource 'CMNU' (mEdit,
#if qNames
"mEdit",
#endif
purgeable) {
	mEdit,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Edit",
	{
	"Reset Startup Disk", noIcon, noKey, noMark, plain, cResetStartupDisk;
	"Reset Install Disk", noIcon, noKey, noMark, plain, cResetInstallDisk;
	}
};
*/

//--------------------------------------------------------------------------------------------------
// RESOURCES FOR VIEWS, DIALOGS, ALERTS etc. 
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Icons, Bundles and FRefs… Oh my! (don't forget the Signature)
//--------------------------------------------------------------------------------------------------

type kSignature as 'STR ';
resource kSignature (0,
#if qNames
"Signature",
#endif
	purgeable) {
	"XPostFacto"
};

resource 'FREF' (kApplicationID,
#if qNames
"XPostFacto Application",
#endif
	purgeable) {
	'APPC',
	0,
	""
};

resource 'BNDL' (kBundleID,
#if qNames
"XPostFacto",
#endif
	purgeable) {
	kSignature,
	0,
	{
		'ICN#',
		{
			0, kApplicationID,
		},
		'FREF',
		{
			0, kApplicationID,
		}
	}
};

/*
resource 'open' (kBundleID)
{
	kSignature, { kFileType, kStationery }
};
*/

resource 'kind' (kBundleID)
{
	kSignature,
	verUS,
	{
		ftApplicationName, 	"XPostFacto",
	}
};

