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

/*

	XPFBootableDevice
	=================

	The purpose of this class is to model a device that XPostFacto can use to boot Mac OS X.
	So, basically, it is the superclass for SCSIDevice and ATADevice. It keeps a list of
	the available devices.

*/

#ifndef __XPFBOOTABLEDEVICE_H__
#define __XPFBOOTABLEDEVICE_H__

#include "XPFPartition.h"

class MountedVolume;
class HFSPlusVolumeHeader;
class XPFBus;

class XPFBootableDevice : public MDependable_AC
{
	public:
		
		static void Initialize ();
		
#ifdef __MACH__	
		static XPFBootableDevice* DeviceForRegEntry (io_registry_entry_t entry);
#else
		static XPFBootableDevice* DeviceWithInfo (FSVolumeInfo *info);		
		static XPFBootableDevice* DeviceWithDriverRefNum (SInt16 driverRefNum);	
#endif	

		static XPFBootableDevice* DeviceWithOpenFirmwareName (char *ofName);
		static XPFBootableDevice* DeviceWithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name);
						
		static void DeleteInvalidDevices ();

		void invalidate () {fInvalid = true;}

		void installBootXToPartition (XPFPartition *part);
		UInt32 getActiveBootXVersion ();

#if qDebug
		static void Dump ();
		void dump ();
#endif

		virtual ~XPFBootableDevice ();
		
		XPFPartition* partitionWithCreationDate (unsigned int date);
		XPFPartition* partitionWithInfo (FSVolumeInfo *info);
		XPFPartition* partitionWithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name);
		
		XPFPartition* getFirstHFSPartition ();
		
#ifdef __MACH__
		OSErr readBlocks (unsigned int start, unsigned int count, UInt8 **buffer);
		OSErr writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer);
		void readCapacity ();
		void openDeviceFile ();
		void closeDeviceFile ();
#else
		virtual OSErr readBlocks (unsigned int start, unsigned int count, UInt8 **buffer) = 0;
		virtual OSErr writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer) = 0;
		virtual	void readCapacity () = 0;
#endif

		char* getOpenFirmwareName (bool useShortName) {return useShortName ? fShortOpenFirmwareName : fOpenFirmwareName;}
		
		virtual bool getNeedsHelper () {return fNeedsHelper;}
		
		virtual XPFBus* getBus () {return fBus;}
		virtual XPFBus* getDefaultBus () {return fDefaultBus;}
		virtual void setBus (XPFBus *bus);
		virtual CVoidList_AC* getBusList () {return NULL;}
		
		virtual bool isFirewireDevice ();
		virtual bool isReallyATADevice ();

		static void DisableCDDriver ();
		static void EnableCDDriver ();
		
	protected:	

#ifdef __MACH__
		XPFBootableDevice (io_registry_entry_t entry);
#else
		XPFBootableDevice (SInt16 driverRefNum);
#endif

		virtual void checkOpenFirmwareName () {}	// meant to be overridden
	
		static TemplateAutoList_AC <XPFBootableDevice> gDeviceList;
		static bool fInitialized;	
			
		virtual void extractPartitionInfo ();
		
#if qDebug
		static void PrintPartitionMapEntry (Partition *part);
#endif

		XPFBus *fBus;
		XPFBus *fDefaultBus;

		char fOpenFirmwareName[256];
		char fShortOpenFirmwareName[256];

		unsigned int fBlockCount;
		unsigned int fBlockSize;
		bool fInvalid;
		bool fNeedsHelper;
		
#ifdef __MACH__
		char fBSDName[32];
		FILE *fDeviceFile;
		bool fIsFireWireDevice;
		bool fIsATADevice;
#else
		SInt16 fDriverRefNum;
#endif
		
		PartitionList *fPartitionList;	

};

typedef TemplateAutoList_AC <XPFBootableDevice>::Iterator DeviceIterator;

#endif