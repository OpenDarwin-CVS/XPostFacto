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

#ifndef __XPFPREFS_H__
#define __XPFPREFS_H__

#include "OFAliases.h"

class MountedVolume;
class XPFIODevice;

class XPFPrefs : public TFileBasedDocument {

	public:
	
		XPFPrefs (TFile* itsFile = NULL);
		~XPFPrefs ();

		// TFileBasedDocument methods

		void DoInitialState ();
		void DoRead (TFile* aFile, bool forPrinting);
		void DoWrite (TFile* aFile, bool makingCopy);
		void DoMakeViews (bool forPrinting);
		void RegainControl ();
		void UpdateWindowIcon (TWindow* /* aWindow */) {}
		
		// Commands
		
		virtual void DoSetupMenus (); // Override		
		virtual void DoMenuCommand (CommandNumber aCommandNumber); // Override

		// Accessors

		void setTargetDisk (MountedVolume *theDisk);
		MountedVolume* getTargetDisk () {return fTargetDisk;}
		
		void setInstallCD (MountedVolume *theDisk);
		MountedVolume* getInstallCD () {return fInstallCD;}
								
		bool getBootInVerboseMode () {return fBootInVerboseMode;}
		bool getBootInSingleUserMode () {return fBootInSingleUserMode;}
		
		void setBootInVerboseMode (bool val);
   		void setBootInSingleUserMode (bool val);
		
		bool getAutoBoot () {return fAutoBoot;}
		void setAutoBoot (bool val);
		
		bool getUseShortStrings () {return fUseShortStrings;}
		bool getUseShortStringsForInstall () {return fUseShortStringsForInstall;}

		bool getEnableCacheEarly () {return fEnableCacheEarly;}
		void setEnableCacheEarly (bool newVal);

		CStr255_AC getBootDevice ();
		CStr255_AC getBootFile ();
		CStr255_AC getBootCommand ();
		
		CStr255_AC getBootDeviceForInstall ();
		CStr255_AC getBootCommandForInstall ();
		CStr255_AC getBootFileForInstall ();
		
		CStr255_AC getInputDevice ();
		CStr255_AC getOutputDevice ();
		CStr255_AC getInputDeviceForInstall ();
		CStr255_AC getOutputDeviceForInstall ();
		
		unsigned getInputDeviceIndex ();
		unsigned getOutputDeviceIndex ();
		
		void setInputDevice (char *label, bool callChanged = true);
		void setInputDevice (XPFIODevice *device);
		
		void setOutputDevice (char *label, bool callChanged = true);
		void setOutputDevice (XPFIODevice *device);
						
		void setThrottle (unsigned throttle);
		unsigned getThrottle () {return fThrottle;}
		
		void setDebugBreakpoint (bool val);
		void setDebugPrintf (bool val);
		void setDebugNMI (bool val);
		void setDebugKprintf (bool val);
		void setDebugDDB (bool val);
		void setDebugSyslog (bool val);
		void setDebugARP (bool val);
		void setDebugOldGDB (bool val);
		void setDebugPanicText (bool val);
		
		bool getDebugBreakpoint ();
		bool getDebugPrintf ();
		bool getDebugNMI ();
		bool getDebugKprintf ();
		bool getDebugDDB ();
		bool getDebugSyslog ();
		bool getDebugARP ();
		bool getDebugOldGDB ();
		bool getDebugPanicText ();
		
	private:
			
#ifdef __MACH__
		void getPrefsFromNVRAM ();
#endif

		void Changed(ChangeID_AC theChange, void* changeData);
		void checkStringLength ();
				
		CStr255_AC getBootCommandBase ();
		
		void setUseShortStrings (bool newVal);
		void setUseShortStringsForInstall (bool newVal);

		bool fUseShortStrings;
		bool fUseShortStringsForInstall;

		MountedVolume *fTargetDisk;
		MountedVolume *fInstallCD;
				
		XPFIODevice *fInputDevice;
		XPFIODevice *fOutputDevice;

		// Values for prefs file. Don't change type, as that would
		// break the prefs file format.

		UInt32	fThrottle;
		UInt8	fBootInVerboseMode;
		UInt8	fBootInSingleUserMode;
		UInt8	fAutoBoot;
		UInt8	fEnableCacheEarly;
		UInt32	fDebug;
};

#endif
