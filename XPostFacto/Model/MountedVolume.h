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

/* 

MountedVolume
=============

The purpose of this class is to maintain a list of the mounted volumes, and
to gather the information required to boot Mac OS X from them. We include
all mounted volumes in the list, but note cases in which a volume is not
bootable. That way, the user interface lists all the volumes that a user
could potentially wish to select, but indicate why some are unavailable.

The application needs to call the static method Initialize () before using
the class, and whenever a volume may have been mounted or unmounted.

The MountedVolume class makes internal use of the SCSIDevice and SCSIBus
classes, but the application need not be aware of that. It can focus on
MountedVolume.

*/

#ifndef __MOUNTED_VOLUME_H__
#define __MOUNTED_VOLUME_H__

#include <SCSI.h>
#include "MDependable_AC.h"
#include "SCSIDevice.h"
#include <Devices.h>
#include <iostream.h>

class MountedVolume;

typedef TemplateAutoList_AC <MountedVolume> MountedVolumeList;
typedef TemplateAutoList_AC <MountedVolume>::Iterator MountedVolumeIterator;

typedef pascal OSErr (*PBControlImmedProcPtr) (ParmBlkPtr paramBlock);

class MountedVolume : public MDependable_AC
{
	public:
	
		static void Initialize ();

#if qDebug
		static void Print (ostream& stream);		
		friend ostream& operator << (ostream& os, MountedVolume& volume);
#endif
		
		const CStr255_AC& getOpenFirmwareName () {if (fPartInfo) return fPartInfo->getOpenFirmwareName (); else return "";}
		const CStr255_AC& getShortOpenFirmwareName () {if (fPartInfo) return fPartInfo->getShortOpenFirmwareName (); else return "";}
		const CStr255_AC& getVolumeName () {return fVolumeName;}
		unsigned int getCreationDate () {return fCreationDate;}
		bool getIsHFSPlus () {return fIsHFSPlus;}
		bool getHasBootX () {if (fPartInfo) return fPartInfo->getHasBootX (); else return false;}
		bool getExtendsPastEightGB () {if (fPartInfo) return fPartInfo->getExtendsPastEightGB (); else return false;}
		bool getIsOnBootableDevice () {return fIsOnBootableDevice;}
		bool getHasMachKernel () {return fHasMachKernel;}
		bool getHasInstaller () {return fHasInstaller;}
		bool getIsWriteable () {return fIsWriteable;}
		bool getValidOpenFirmwareName () {return fValidOpenFirmwareName;}
		bool getHasOldWorldSupport () {return fHasOldWorldSupport;}
		bool getHasFinder () {return fHasFinder;}
		FSRef* getRootDirectory () {return &fRootDirectory;}
		
		bool getHasBeenPrepared () {return fHasBeenPrepared;}
		void setHasBeenPrepared (bool val) {fHasBeenPrepared = val;}
		
		void installBootXIfNecessary (bool forceInstall = false);
		
		short getIOVDrvInfo () {return fInfo.driveNumber;}
		short getIOVDRefNum () {return fInfo.driverRefNum;}
		
		unsigned getBootStatus ();
		unsigned getInstallerStatus ();
		unsigned getInstallTargetStatus ();
		
		static const MountedVolumeList* GetVolumeList () {return &gVolumeList;}
		
		static MountedVolume* WithCreationDate (unsigned int date);
		static MountedVolume* WithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name);
		static MountedVolume* WithInfo (FSVolumeInfo *info);
		
	private:
	
		MountedVolume (FSVolumeInfo *info, HFSUniStr255 *name, FSRef *rootDirectory);
	
		FSRef fRootDirectory;
		CStr255_AC fVolumeName;
		HFSUniStr255 fHFSName;
		FSVolumeInfo fInfo;

		UInt32 fAllocationBlockSize;

		bool fIsOnBootableDevice;
		bool fIsWriteable;
		bool fHasMachKernel;
		bool fHasOldWorldSupport;
		bool fIsHFSPlus;
		bool fHasInstaller;
		bool fValidOpenFirmwareName;
		bool fStillThere;
		bool fHasBeenPrepared;
		bool fHasFinder;
		
		XPFBootableDevice *fBootableDevice;
		XPFPartition *fPartInfo;

		unsigned int fCreationDate;
				
		static MountedVolumeList gVolumeList;
		
		static PBControlImmedProcPtr gPBControlImmed; 
		static OSErr callPBControlImmed (ParmBlkPtr paramBlock);

};

#endif