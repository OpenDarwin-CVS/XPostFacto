/*

Copyright (c) 2000, 2001
Ryan Rempel
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

#include "MountedVolume.h"
#include "MoreFilesExtras.h"
#include "SCSIBus.h"
#include "FastUnicodeCompare.h"

#include <Devices.h>
#include <Files.h>
#include <HFSVolumes.h>
#include <UnicodeConverter.h>
#include "XPFLog.h"
#include "HFSPlusArchive.h"
#include "XPFErrors.h"
#include "Sound.h"

#include <iostream.h>
#include "XPostFacto.h"
#include "XPFStrings.h"

MountedVolumeList MountedVolume::gVolumeList;


PBControlImmedProcPtr MountedVolume::gPBControlImmed = NULL;

OSErr 
MountedVolume::callPBControlImmed (ParmBlkPtr paramBlock)
{
	if (gPBControlImmed == 0) return paramErr;
	return (*gPBControlImmed) (paramBlock);
}


void
MountedVolume::Initialize ()
{
	#if qLogging
		gLogFile << "Initializing Mounted Volumes" << endl_AC;
	#endif

	// First, we set up the calls to ataManager
	if (!gPBControlImmed) {
		OSErr err;
		CFragConnectionID connID = kInvalidID;

		err = GetSharedLibrary( "\pInterfaceLib", kCompiledCFragArch,
			kReferenceCFrag, &connID, NULL, NULL );

		if ( err == noErr ) {
			err = FindSymbol (connID, "\pPBControlImmed", (Ptr *) &gPBControlImmed, NULL);
		} 

		if ( err != noErr ) gPBControlImmed = NULL;
	}

	// Initialize the devices
	XPFBootableDevice::Initialize ();

	// We mark all the current volumes as gone, so that we can see which are
	// still around.
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current (); iter.Next ()) {
		iter->fStillThere = false;
	}
		
	OSErr err = noErr;
	ItemCount item = 0;
	FSVolumeInfo info;
	HFSUniStr255 volName;
	
	do {	
		item++;
		err = FSGetVolumeInfo (kFSInvalidVolumeRefNum, item, NULL, 
				kFSVolInfoCreateDate | kFSVolInfoBlocks, &info, &volName, NULL);
		if (err == noErr) {
			MountedVolume *volume = MountedVolume::WithInfoAndName (&info, &volName);
			if (volume) volume->fStillThere = true;
		}	
	} while (err == noErr);


	// Now we see whether anything has gone away. If so, we need to delete it from the
	// list, and invalidate its device
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current (); iter.Next ()) {
		MountedVolume *current = iter.Current ();
		if (!current->fStillThere) {
			if (current->fBootableDevice) current->fBootableDevice->invalidate ();
			gVolumeList.Delete (current);	
			delete current;
			gApplication->Changed (cVolumeListChange, NULL);
		}
	}
	
	XPFBootableDevice::DeleteInvalidDevices ();
	
	// Now we cycle through again to get the new ones. This could be simplified, but
	// it makes other things easier if we delete the old ones first and then do the
	// new ones.
	
	item = 0;
	err = noErr;
	
	do {	
		item++;
		FSRef rootDirectory;
		err = FSGetVolumeInfo (kFSInvalidVolumeRefNum, item, NULL, 
				kFSVolInfoCreateDate | kFSVolInfoBlocks | kFSVolInfoFlags | 
				kFSVolInfoFSInfo | kFSVolInfoDriveInfo, &info, &volName, &rootDirectory);
		if (err == noErr) {
			MountedVolume *volume = MountedVolume::WithInfoAndName (&info, &volName);
			if (!volume) {
				try {
					volume = new MountedVolume (&info, &volName, &rootDirectory);
					gVolumeList.InsertLast (volume);
					gApplication->Changed (cVolumeListChange, NULL);
					volume->fStillThere = true;
				}
				catch (...) {
					#if qLogging
						gLogFile << "Error initializing MountedVolume" << endl_AC;
					#endif
				}
			}
		}	
	} while (err == noErr);

			
	#if qLogging
		gLogFile << "Finished initializing Mounted Volumes" << endl_AC;
	#endif
}

/************/

#if qDebug

void 
MountedVolume::Print (ostream& stream)
{
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current(); iter.Next()) {
		stream << *(iter.Current ());		
	}
}	

ostream& operator << (ostream& os, MountedVolume& volume)
{
	os << "\"";
	os.write ((char *) &volume.fVolumeName[1], volume.fVolumeName[0]);
	os << "\" ";
	if (volume.getValidOpenFirmwareName ()) {
		CStr255_AC openFirmwareName = volume.getOpenFirmwareName ();
		os.write ((char *) openFirmwareName[1], openFirmwareName[0]);
	} else {
		os << " -- not SCSI" << endl;
	}
	return os;
}

#endif // qDebug

/***************/

MountedVolume*
MountedVolume::WithCreationDate (unsigned int date)
{
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current(); iter.Next()) {
		if (iter->fCreationDate == date) {
			return iter.Current ();
		}		
	}
	return NULL;
}

MountedVolume*
MountedVolume::WithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name)
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current(); iter.Next()) {
		FSVolumeInfo *compareInfo = &iter->fInfo;
		if (	(compareInfo->createDate.lowSeconds == info->createDate.lowSeconds) && 
				(compareInfo->blockSize == info->blockSize) &&
				(compareInfo->totalBlocks == info->totalBlocks) &&
				(FastUnicodeCompare (name->unicode, name->length, iter->fHFSName.unicode, iter->fHFSName.length) == 0)
		) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

MountedVolume*
MountedVolume::WithInfo (FSVolumeInfo *info)
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current(); iter.Next()) {
		FSVolumeInfo *compareInfo = &iter->fInfo;
		if (	(compareInfo->createDate.lowSeconds == info->createDate.lowSeconds) && 
				(compareInfo->blockSize == info->blockSize) &&
				(compareInfo->totalBlocks == info->totalBlocks)
		) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

void
MountedVolume::installBootXIfNecessary (bool forceInstall)
{
	if (!getIsWriteable()) {
		if (getHasBootX ()) {
			// we'll just assume that it's really installed. I really should check
			// and throw an exception if it isn't
			return;
		} else {
			ThrowException_AC (kVolumeNotWriteable, 0);
		}
	}
	if (fPartInfo) {
		fPartInfo->installBootXIfNecessary (forceInstall);
	} else {
		#if gLogging
			gLogFile << "No partiton info for: " << getVolumeName () << endl_AC;
		#endif
	}
}


MountedVolume::MountedVolume (FSVolumeInfo *info, HFSUniStr255 *name, FSRef *rootDirectory)
{
	fValidOpenFirmwareName = false;
	fIsOnBootableDevice = false;
	fPartInfo = NULL;
	fBootableDevice = NULL;
	fCreationDate = 0;
		
	// Copy the info
	{
		BlockMoveData (info, &fInfo, sizeof (FSVolumeInfo));
		BlockMoveData (name, &fHFSName, sizeof (HFSUniStr255));
	
		OSStatus status;
		UnicodeToTextInfo converter;		
		status = CreateUnicodeToTextInfoByEncoding (
			CreateTextEncoding (kTextEncodingMacHFS, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat),
			&converter);
		if (status == noErr) {
			ConvertFromUnicodeToPString (converter, name->length * sizeof (UniChar), name->unicode, fVolumeName);
			DisposeUnicodeToTextInfo (&converter);
		} else {
			HParamBlockRec pb;
			Erase_AC (&pb);
			pb.volumeParam.ioNamePtr = fVolumeName;
			pb.volumeParam.ioVRefNum = info->driveNumber;
			ThrowIfOSErr_AC (PBHGetVInfoSync (&pb));
		}
		
		LocalDateTime localTime;
		status = ConvertUTCToLocalDateTime (&info->createDate, &localTime);
		if (status == noErr) {
			fCreationDate = localTime.lowSeconds;
		} else {
			#if qLogging
				gLogFile << "Error getting creation date" << endl_AC;
			#endif
		}
		fAllocationBlockSize = info->blockSize;
		
		BlockMoveData (rootDirectory, &fRootDirectory, sizeof (fRootDirectory));
	}
		
	// See if it's got a mach_kernel
	{
		CFSSpec_AC kernel;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:mach_kernel", &kernel);
		fHasMachKernel = (err == noErr);
	}
		
	// See if it's an installer
	{
		CFSSpec_AC installer;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:CDIS:", &installer);
		// check for Darwin as well
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:packages:", &installer);
		fHasInstaller = (err == noErr);
	}
		
	// See if it's writeable
	{
		fIsWriteable = !(info->flags & (kFSVolFlagHardwareLockedMask | kFSVolFlagSoftwareLockedMask));
	}
	
	// See if it's HFS Plus
	{
		fIsHFSPlus = (info->signature == kHFSPlusSigWord);
	}
	
	// See if OldWorldSupport is installed
	{
		CFSSpec_AC supportSpec;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Library:Extensions:PatchedAppleNVRAM.kext", &supportSpec);
		fHasOldWorldSupport = (err == noErr);
	}
		
	// Do some logging
	#if qLogging
	{
		gLogFile << "Volume: ";
		gLogFile.WriteCharBytes ((char *) &fVolumeName[1], fVolumeName[0]);
		gLogFile << endl_AC;
		gLogFile << "Creation Date: " << fCreationDate << endl_AC;
	}
	#endif
	
	// Now get the Device and Partition
	fBootableDevice = XPFBootableDevice::DeviceWithInfo (info);
	fValidOpenFirmwareName = false;
	if (fBootableDevice) {
		fIsOnBootableDevice = true;
		fPartInfo = fBootableDevice->partitionWithInfoAndName (info, name);
		if (fPartInfo) {
			fValidOpenFirmwareName = fPartInfo->getValidOpenFirmwareName ();
			fPartInfo->setMountedVolume (this);
		} else {
			fValidOpenFirmwareName = false;
		}
	}
	
	#if qLogging
		if (fValidOpenFirmwareName) {
			gLogFile << "OpenFirmwareName: ";
			CStr255_AC openFirmwareName = getOpenFirmwareName ();
			gLogFile.WriteCharBytes ((char *) &openFirmwareName[1], openFirmwareName[0]);
			gLogFile << endl_AC;
		} else {
			gLogFile << "Could not find Open Firmware name." << endl_AC;
		}
	#endif
}

unsigned
MountedVolume::getBootStatus ()
{
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getHasMachKernel ()) return kNoMachKernel;
	if (!getIsOnBootableDevice ()) return kNotSCSI;
	if (!getValidOpenFirmwareName ()) return kNoOFName;
	if (!getHasBootX() && !getIsWriteable ()) return kNoBootX;

	return kStatusOK;
}

unsigned
MountedVolume::getInstallTargetStatus ()
{
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getIsOnBootableDevice ()) return kNotSCSI;
	if (!getValidOpenFirmwareName ()) return kNoOFName;
	if (!getIsWriteable ()) return kNotWriteable;

	return kStatusOK;
}

unsigned
MountedVolume::getInstallerStatus ()
{
	if (!getHasInstaller ()) return kNotInstaller;

	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getHasMachKernel ()) return kNoMachKernel;
	if (!getIsOnBootableDevice ()) return kNotSCSI;
	if (!getValidOpenFirmwareName ()) return kNoOFName;
	
	return kStatusOK;
}