/*

Copyright (c) 2002, 2003
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
		void Close ();
		
		void DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace);
								
		short PoseSaveDialog ();
		short PoseConfirmDialog (bool forInstall, bool quitting);
		
		void SaveFile (	CommandNumber		itsCommandNumber,
						TFile*				theSaveFile,
						CSaveOptions*		itsOptions);
	
		// Startup item
		
		void suspendStartupItem ();
		void restartStartupItem ();
		
		// Commands
		
		virtual void DoSetupMenus (); // Override		
		virtual void DoMenuCommand (CommandNumber aCommandNumber); // Override
		virtual void DoEvent	(EventNumber eventNumber,
							TEventHandler* source,
							TEvent* event);
							
		// NVRAM
		
		void writePrefsToNVRAM (bool forInstall);
						
		// Accessors

		bool getUseShortStrings (bool forInstall) {return forInstall ? fUseShortStringsForInstall : fUseShortStrings;}
		bool getTooBigForNVRAM (bool forInstall) {return forInstall ? fTooBigForNVRAMForInstall : fTooBigForNVRAM;}

		CStr255_AC getBootDevice (bool forInstall);
		CStr255_AC getBootFile (bool forInstall);
		CStr255_AC getBootCommand (bool forInstall);
				
		CStr255_AC getInputDevice (bool forInstall);
		CStr255_AC getOutputDevice (bool forInstall);
		
		unsigned getInputDeviceIndex ();
		unsigned getOutputDeviceIndex ();

		void setInputDevice (char *label, bool callChanged = true);		
		void setOutputDevice (char *label, bool callChanged = true);
		
		bool getRestartOnClose () {return fRestartOnClose;}
		void setRestartOnClose (bool val) {fRestartOnClose = val;}
				
#define DECLARE_ACCESSORS(type,method) 						\
	void set##method (type val, bool callChanged = true); 	\
	type get##method () {return f##method;}
	
#define DECLARE_DEBUG_ACCESSORS(method) 					\
	void set##method (bool val, bool callChanged = true); 	\
	bool get##method ();
	
DECLARE_ACCESSORS (MountedVolume*, TargetDisk)
DECLARE_ACCESSORS (MountedVolume*, InstallCD)
DECLARE_ACCESSORS (XPFIODevice*, InputDevice)
DECLARE_ACCESSORS (XPFIODevice*, OutputDevice)
DECLARE_ACCESSORS (bool, BootInVerboseMode)
DECLARE_ACCESSORS (bool, BootInSingleUserMode)
DECLARE_ACCESSORS (bool, AutoBoot)		
DECLARE_ACCESSORS (bool, RebootInMacOS9)
DECLARE_ACCESSORS (bool, EnableCacheEarly)
DECLARE_ACCESSORS (bool, UseShortStrings)
DECLARE_ACCESSORS (bool, UseShortStringsForInstall)
DECLARE_ACCESSORS (unsigned, Throttle)
DECLARE_ACCESSORS (bool, UseROMNDRV)
		
DECLARE_DEBUG_ACCESSORS (DebugBreakpoint)
DECLARE_DEBUG_ACCESSORS (DebugPrintf)
DECLARE_DEBUG_ACCESSORS (DebugNMI)
DECLARE_DEBUG_ACCESSORS (DebugKprintf)
DECLARE_DEBUG_ACCESSORS (DebugDDB)
DECLARE_DEBUG_ACCESSORS (DebugSyslog)
DECLARE_DEBUG_ACCESSORS (DebugARP)
DECLARE_DEBUG_ACCESSORS (DebugOldGDB)
DECLARE_DEBUG_ACCESSORS (DebugPanicText)
						
	private:
			
		void getPrefsFromNVRAM ();
		void checkForUpdates (bool forInstall);

		void tellFinderToRestart ();

		void Changed(ChangeID_AC theChange, void* changeData);
		void checkStringLength ();
				
		CStr255_AC getBootCommandBase ();
		
		void installXPFPartitionInfo ();

		// Variables

		bool fUseShortStrings;
		bool fUseShortStringsForInstall;
		bool fRebootInMacOS9;
		bool fRestartOnClose;
		bool fTooBigForNVRAM;
		bool fTooBigForNVRAMForInstall;
		bool fForceAskSave;
				
		MountedVolume *fTargetDisk;
		MountedVolume *fInstallCD;
				
		XPFIODevice *fInputDevice;
		XPFIODevice *fOutputDevice;
		
		TWindow *fOptionsWindow;

		// Values for prefs file. Don't change type, as that would
		// break the prefs file format.

		UInt32	fThrottle;
		UInt8	fBootInVerboseMode;
		UInt8	fBootInSingleUserMode;
		UInt8	fAutoBoot;
		UInt8	fEnableCacheEarly;
		UInt32	fDebug;
		UInt8	fUseROMNDRV;
};

#endif
