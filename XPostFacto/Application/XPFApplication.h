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


#ifndef __XPFAPPLICATION_H__
#define __XPFAPPLICATION_H__

#include "UApplication.h"
#include "CCooperativeThread_AC.h"
#include "CThreadRunner_AC.h"
#include "XPFPlatform.h"

extern UInt32 gSystemVersion;

//----------------------------------------------------------------------------------------
// XPFApplication
//----------------------------------------------------------------------------------------

class XPFWindow;
class XPFAboutBox;
class MountedVolume;
class TTEViewStream;
class CLogReporter;

class XPFNVRAMSettings;
class XPFPrefs;

enum {
	kDisableRestart 			= 1 << 0,
	kVisibleHelperFiles 		= 1 << 1,
	kDisableNVRAMWriting		= 1 << 2,
	kDisableCopyToHelper		= 1 << 3,
	kDisableBootX				= 1 << 4,
	kDisableExtensionsCache 	= 1 << 5,
	kDisableExtensions			= 1 << 6,
	kDisableStartupItem			= 1 << 7,
	kDisableCoreServices		= 1 << 8
};

class XPFApplication : public TApplication {

	public:
		XPFApplication();
			// Constructor
		
		virtual ~XPFApplication();
			// Destructor
					
		// TApplication glue
		
		void DoInitialState ();
		
		TDocument* DoMakeDocument (CommandNumber /* itsCommandNumber */, TFile* itsFile);
		TFile* FindOrCreatePrefsFile (CommandNumber itsCommand);
		virtual TDocument* OpenNew (CommandNumber itsCommandNumber);
			
		virtual void HandleDiskEvent (TToolboxEvent* event);
							
		virtual void InstallHelpMenuItems();
		
#ifndef __MACH__
		virtual void GetHelpParameters(	ResNumber			helpResource,
										short				helpIndex,
										short				helpState,
										HMMessageRecord&	helpMessage,
										CPoint_AC&			localQDTip,
										CRect_AC&			localQDRect,
										short&				balloonVariant);
#endif
			
		virtual void AboutToLoseControl (bool saveClipboard);
		virtual void RegainControl (bool checkClipboard);
			
		// Accessors
		
		XPFPlatform* getPlatform () {return fPlatform;}
		UInt32 getDebugOptions () {return fDebugOptions;}
		XPFPrefs *getPrefs () {return fPrefs;}
		
		// Commands
		void CloseSplashWindow ();
		void launchURL (CStr255_AC theURL);
		void reportFatalError (CStr255_AC error);
		
		virtual void DoSetupMenus(); // Override		
		virtual void DoMenuCommand(CommandNumber aCommandNumber); // Override
		virtual void DoAboutBox();
		void DoShowHelpFile ();
			
		virtual void DoAEClose(TAppleEvent* message, TAppleEvent* reply);
		
	private:		
		
		void toggleDebugOption (UInt32 option) {if (fDebugOptions & option) fDebugOptions &= ~option; else fDebugOptions |= option;}
		void addDebugOptionsToHelpMenu ();

		void installMenuHelpTags (ResNumber menuResID);

		void saveLogToFile ();

	private:
				
		XPFAboutBox *fAboutBox;
		TWindow *fSplash;
		TWindow *fLogWindow;
		
		CAutoPtr_AC <TTEViewStream> fViewStream;
		CAutoPtr_AC <CLogReporter> fReporter;
			
		XPFPrefs *fPrefs;
		XPFPlatform *fPlatform;
		
		bool fShowDebugOptions;
		UInt32 fDebugOptions;
		
		bool fSystemShowHelpTags;
				
};	

#endif

