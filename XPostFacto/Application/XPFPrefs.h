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

#include "MDependable_AC.h"

class MountedVolume;

struct XPFDebug {
	bool	breakpoint;
	bool	printf;
	bool	nmi;
	bool	kprintf;
	bool	ddb;
	bool	syslog;
	bool	arp;
	bool	oldgdb;
	bool	panicText;
};

class XPFPrefs : public MDependable_AC {

	public:
	
		XPFPrefs ();
		~XPFPrefs ();

		void Initialize ();

		void setBootDisk (MountedVolume *theDisk);
		MountedVolume* getBootDisk () {return fBootDisk;}
		
		void setInstallDisk (MountedVolume *theDisk);
		MountedVolume* getInstallDisk () {return fInstallDisk;}
		
		void setHelperDisk (MountedVolume *theDisk);
		MountedVolume* getHelperDisk () {return fHelperDisk;}
		
		bool getBootInVerboseMode () {return fBootInVerboseMode;}
		bool getBootInSingleUserMode () {return fBootInSingleUserMode;}
		
		void setBootInVerboseMode (bool val);
		void setBootInDebugMode (bool val);
		void setBootInSingleUserMode (bool val);
		
		bool getReinstallBootX () {return fReinstallBootX;}
		void setReinstallBootX (bool val);
		bool getReinstallExtensions () {return fReinstallExtensions;}
		void setReinstallExtensions (bool val);
		bool getAutoBoot () {return fAutoBoot;}
		void setAutoBoot (bool val);
		bool getSetupL2Cache () {return fSetupL2Cache;}
		void setSetupL2Cache (bool val);
		unsigned int getL2CRValue () {return fL2CRValue;}
		bool getUseShortStrings () {return fUseShortStrings;}

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
				
		unsigned int getNextInputDevice () {return fNextInputDevice;}
		unsigned int getNextOutputDevice () {return fNextOutputDevice;}
		
		unsigned int getInputDeviceIndex () {return fInputDeviceIndex;}
		unsigned int getOutputDeviceIndex () {return fOutputDeviceIndex;}
		
		unsigned int getNextHelperVolume () {return fNextHelperVolume;}
		unsigned int getHelperVolumeIndex ();

		void setHelperVolumeIndex (unsigned index);
		void setOutputDeviceIndex (unsigned index);
		void setInputDeviceIndex (unsigned index);
		
		void setThrottle (unsigned throttle);
		unsigned getThrottle () {return fThrottle;}
		
		void Changed(ChangeID_AC theChange, void* changeData);
		void checkStringLength ();
		
		void setDebugBreakpoint (bool val);
		void setDebugPrintf (bool val);
		void setDebugNMI (bool val);
		void setDebugKprintf (bool val);
		void setDebugDDB (bool val);
		void setDebugSyslog (bool val);
		void setDebugARP (bool val);
		void setDebugOldGDB (bool val);
		void setDebugPanicText (bool val);
		
		bool getDebugBreakpoint () {return fDebug.breakpoint;}
		bool getDebugPrintf () {return fDebug.printf;}
		bool getDebugNMI () {return fDebug.nmi;}
		bool getDebugKprintf () {return fDebug.kprintf;}
		bool getDebugDDB () {return fDebug.ddb;}
		bool getDebugSyslog () {return fDebug.syslog;}
		bool getDebugARP () {return fDebug.arp;}
		bool getDebugOldGDB () {return fDebug.oldgdb;}
		bool getDebugPanicText () {return fDebug.panicText;}
		
	private:
	
		void getPrefsFromFile ();
		void writePrefsToFile ();
		
		void initializeInputAndOutputDevices ();
		void initializeHelperMenu ();
		void addInputOutputDevice (RegEntryID *entry, TemplateList_AC <char> *list);
		
		CStr255_AC getBootCommandBase ();

		CFile_AC *fPrefs;

		unsigned int fCachedCreationDate;
		unsigned int fHelperCreationDate;
		unsigned int fL2CRValue;
		unsigned int fThrottle;
		bool fBootInVerboseMode;
		bool fBootInSingleUserMode;
		bool fReinstallBootX;
		bool fReinstallExtensions;
		bool fAutoBoot;
		bool fSetupL2Cache;

		bool fDirty;
		bool fUseShortStrings;
		bool fUseShortStringsForInstall;
		bool fForceShortStrings;

		MountedVolume *fBootDisk;
		MountedVolume *fInstallDisk;
		MountedVolume *fHelperDisk;

		TemplateList_AC <char> fInputDevices;
		TemplateList_AC <char> fOutputDevices;
		TemplateList_AC <char> fShortInputDevices;
		TemplateList_AC <char> fShortOutputDevices;
		unsigned int fInputDeviceIndex;
		unsigned int fOutputDeviceIndex;
		
		unsigned int fNextInputDevice;
		unsigned int fNextOutputDevice;		
		unsigned int fNextHelperVolume;
		
		XPFDebug fDebug;

};

#endif