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

//----------------------------------------------------------------------------------------
// XPFApplication
//----------------------------------------------------------------------------------------

class XPFWindow;
class XPFAboutBox;
class MountedVolume;

class NVRAMVariables;
class XPFPrefs;

class XPFApplication : public TApplication {

	public:
		XPFApplication();
			// Constructor
		
		virtual ~XPFApplication();
			// Destructor
			
		virtual void DoAboutBox();
		
		void reportFatalError (CStr255_AC error);
			
		virtual void HandleDiskEvent(TToolboxEvent* event);
		virtual void RegainControl(bool checkClipboard);
			
		virtual TDocument* OpenNew(CommandNumber itsCommandNumber);
		virtual TDocument* OpenOld(CommandNumber itsOpenCommand, CList_AC* aFileList);
		
		bool getHasHFSPlusAPIs () {return fHasHFSPlusAPIs;}
		bool getRunningInBlueBox () {return fRunningInBlueBox;}
						
		void copyHFSArchivesTo (FSRef *directory);
				
		void setCopyInProgress (bool val);
		bool getCopyInProgress () {return fCopyInProgress;}
		const CPascalStr_AC& getCopyingFile () {return fCopyingFile;}
		void setCopyingFile (CStr255_AC copyMessage);
		
		XPFPrefs* getPrefs () {return fPrefs;}
		
		// Commands
		virtual void DoSetupMenus(); // Override		
		virtual void DoMenuCommand(CommandNumber aCommandNumber); // Override
		
		void prepare ();
		void restart ();
		void install ();
		
		unsigned canBoot ();
		
		Boolean copyFilter (const FSRef *src);	
		
	private:
		void tellFinderToRestart ();
		
		void adjustThrottle (NVRAMVariables *nvram);
				
		void initializeNVRAM ();
		void initializeThrottleMenu ();
		
		XPFWindow *fMainWindow;
		XPFAboutBox *fAboutBox;
			
		XPFPrefs *fPrefs;
			
		bool fHasHFSPlusAPIs;
		bool fRunningInBlueBox;
		
		bool fHasL2Cache;
	
		bool fCopyInProgress;
		CStr255_AC fCopyingFile;
		
};	

#endif

