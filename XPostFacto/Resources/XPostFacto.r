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

// Text styles
// --------------

#define kSystemTextStyle				1000

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
//	include kDefaultRsrcName  'CMNU' (mApple);			// Grab the default Apple menus
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
		mInstall;
	}
};

#if qCarbon
resource 'MBAR' (kMBarAqua, 
#if qNames
"kMBarAqua", 
#endif
purgeable)
{
	{
		mApple;
		mInstall;
	}
};
#endif

//--------------------------------------------------------------------------------------------------
// Menus 
//--------------------------------------------------------------------------------------------------

resource 'CMNU' (mApple, 
#if qNames
"mApple", 
#endif
purgeable) {
	mApple,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{
	"About XPostFacto…",	noIcon, noKey, noMark, plain, cAboutApp;
	"-",					noIcon, noKey, noMark, plain, nocommand;
	}
};

#ifndef qCarbonMach0
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
	"Quit",				noIcon, "Q",	noMark, plain, cQuit;
	}
};
#endif

resource 'CMNU' (mInstall,
#if qNames
"mInstall",
#endif
purgeable) {
	mInstall,
	textMenuProc,
	EnablingManagedByMacApp,
	enabled,
	"Reinstall",
	{
	"BootX",		noIcon, noKey, noMark, plain, cInstallBootX;
	"Extensions",	noIcon, noKey, noMark, plain, cInstallExtensions;
	"Startup Item", noIcon, noKey, noMark, plain, cInstallStartupItem;
	}
};

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
		'apnm', 	"XPostFacto",
	}
};

resource 'tab#' (3000, "Tab Labels", purgeable)
{
	versionZero
	{{
		0, "Boot",
		0, "Install",
		0, "Debug",
		0, "Open Firmware",
		0, "Log",
	}}
};


