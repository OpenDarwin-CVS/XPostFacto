/*

Copyright (c) 2000 - 2005
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

#include "MountedVolume.h"
#include "MoreFilesExtras.h"
#include "FastUnicodeCompare.h"

#ifdef __MACH__
	#include <sys/mount.h>
	#include <IOKit/IOKitLib.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/stat.h>
#else
	#include <Devices.h>
	#include <Files.h>
	#include <HFSVolumes.h>
	#include <UnicodeConverter.h>
	#include <AppleDiskPartitions.h>
	#include <Sound.h>
#endif

#ifdef BUILDING_XPF
	#include "XPFApplication.h"
#endif

#include "XPFLog.h"
#include "XPFErrors.h"
#include "XPFBootableDevice.h"
#include "XPFPartition.h"
#include "XPFAuthorization.h"
#include "XCOFFDecoder.h"
#include "XPFPlatform.h"
#include "FastUnicodeCompare.h"

#include <iostream.h>
#include "XPostFacto.h"
#include "XPFStrings.h"
#include "OFAliases.h"
#include "vers_rsrc.h"
#include "XPFFSRef.h"
#include <stdio.h>
#include <fcntl.h>

#include "MoreDisks.h"

#define kExpectedFirstHFSPartition 6
#define kBootXRAMRequired (96 * 1024 * 1024)

MountedVolumeList MountedVolume::gVolumeList;

void
MountedVolume::Initialize ()
{
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
		FSRef rootDirectory;
		err = FSGetVolumeInfo (kFSInvalidVolumeRefNum, item, NULL, 
				kFSVolInfoCreateDate | kFSVolInfoBlocks | kFSVolInfoSizes | kFSVolInfoFlags | 
				kFSVolInfoFSInfo | kFSVolInfoDriveInfo, &info, &volName, &rootDirectory);
		if (err == noErr) {
			FSCatalogInfo rootInfo;
			FSGetCatalogInfo (&rootDirectory, kFSCatInfoCreateDate, &rootInfo, NULL, NULL, NULL);
			info.createDate.lowSeconds = rootInfo.createDate.lowSeconds;
			MountedVolume *volume = MountedVolume::WithRootDirectory (&rootDirectory);
			if (volume) {
				volume->fStillThere = true;
				// catch name changes for the UI
				volume->setHFSName (&volName);
			} else if (info.totalBlocks) {
				volume = new MountedVolume (&info, &volName, &rootDirectory);
				gVolumeList.InsertLast (volume);
#ifdef BUILDING_XPF
				gApplication->Changed (cNewMountedVolume, volume);
#endif
				volume->fStillThere = true;
			}
		}	
	} while (err == noErr);

	// Now we see whether anything has gone away. If so, we need to delete it from the
	// list, and invalidate its device
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current (); iter.Next ()) {
		MountedVolume *current = iter.Current ();
		if (!current->fStillThere) {
			if (current->fBootableDevice) current->fBootableDevice->invalidate ();
			gVolumeList.Delete (current);
#ifdef BUILDING_XPF
			gApplication->Changed (cDeleteMountedVolume, current);
#endif
			delete current;
		}
	}
	
	XPFBootableDevice::DeleteInvalidDevices ();
	
	// Now, we check all the volumes to see if any need a helper and haven't got one
	// already.
	for (MountedVolumeIterator iter (&gVolumeList); iter.Current (); iter.Next ()) {
		MountedVolume *current = iter.Current ();
		if (current->getRequiresBootHelper () && !current->getHelperDisk ()) {
			current->setHelperDisk (current->getDefaultHelperDisk ());
		}
	}
}

MountedVolume*
MountedVolume::WithCreationDate (unsigned int date)
{
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->fCreationDate == date) {
			return iter.Current ();
		}		
	}
	return NULL;
}

MountedVolume*
MountedVolume::WithRootDirectory (FSRef *rootDirectory)
{
	if (!rootDirectory) return NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (FSCompareFSRefs (iter->getRootDirectory (), rootDirectory) == noErr) {
			return iter.Current ();
		}		
	}
	return NULL;
}

MountedVolume*
MountedVolume::WithInfo (FSVolumeInfo *info)
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
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

MountedVolume*
MountedVolume::getDefaultHelperDisk ()
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getBootableDevice () && !iter->getRequiresBootHelper () && (iter.Current() != this)) {
			if (retVal) {
				if (iter->getFreeBytes () > retVal->getFreeBytes ()) retVal = iter.Current (); 
			} else {
				retVal = iter.Current ();
			}
		}
	}
	return retVal;
}

MountedVolume *
MountedVolume::GetDefaultInstallerDisk () 
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getInstallerStatus () == kStatusOK) {
			if (retVal) {
				if (iter->getFreeBytes () > retVal->getFreeBytes ()) retVal = iter.Current (); 
			} else {
				retVal = iter.Current ();
			}
		}
	}
	return retVal;	
}

MountedVolume *
MountedVolume::GetDefaultInstallTarget ()
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getInstallTargetStatus () == kStatusOK) {
			if (retVal) {
				if (iter->getFreeBytes () > retVal->getFreeBytes ()) retVal = iter.Current (); 
			} else {
				retVal = iter.Current ();
			}
		}
	}
	return retVal;	
}

MountedVolume *
MountedVolume::GetDefaultRootDisk ()
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if ((iter->getBootStatus () == kStatusOK) && !iter->getHasInstaller ()) {
			if (retVal) {
				if (iter->getFreeBytes () > retVal->getFreeBytes ()) retVal = iter.Current (); 
			} else {
				retVal = iter.Current ();
			}
		}
	}
	return retVal;	
}

MountedVolume *
MountedVolume::GetDefaultMacOS9Disk ()
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (iter->getMacOS9BootStatus () == kStatusOK) {
			if (retVal) {
				if (iter->getFreeBytes () > retVal->getFreeBytes ()) retVal = iter.Current (); 
			} else {
				retVal = iter.Current ();
			}
		}
	}
	return retVal;
}

#ifdef __MACH__

MountedVolume*
MountedVolume::WithRegistryEntry (io_object_t entry)
{
	MountedVolume *retVal = NULL;
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		io_object_t regEntry = iter->getRegEntry ();
		if (!regEntry) continue;
		if (IOObjectIsEqualTo (regEntry, entry)) {
			retVal = iter.Current ();
			IOObjectRelease (regEntry);
			break;
		}		
		IOObjectRelease (regEntry);
	}	
	return retVal;
}

#endif

MountedVolume*
MountedVolume::WithOpenFirmwarePath (char *pathArg)
{
	MountedVolume *retVal = NULL;
	
	// We break the path at a comma-backslash, in case there is a file specifier in the path
	// This would be true on New World machines, which specify a path to BootX
	char path[256];
	strcpy (path, pathArg);
	char *comma = strstr (path, ",\\");
	if (comma) *comma = 0;

#ifdef __MACH__
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_object_t entry = NULL;
	io_iterator_t iter = NULL;
	IOServiceGetMatchingServices (iokitPort, IOOpenFirmwarePathMatching (iokitPort, NULL, path), &iter);
	if (iter) {
		entry = IOIteratorNext (iter);
		IOObjectRelease (iter);
	}
	if (entry) {
		retVal = MountedVolume::WithRegistryEntry (entry);
		IOObjectRelease (entry);
	}
#else
	for (MountedVolumeIterator iter (GetVolumeList ()); iter.Current(); iter.Next()) {
		if (OFAliases::MatchAliases (iter->getOpenFirmwareName (false), path)) {
			retVal = iter.Current ();
			break;
		}
	}	
#endif

	return retVal;
}

bool 
MountedVolume::getExtendsPastEightGB () 
{
	return fPartition ? fPartition->getExtendsPastEightGB () : false;
}

UInt32
MountedVolume::getActiveBootXVersion () 
{
	// On New World, we can just check for BootX on this volume.
	// On Old World, we have to ask the device, because OF on Old World won't
	// necessarily use our BootX if the device has others.
	if (XPFPlatform::GetPlatform()->getIsNewWorld ()) {
		return getBootXVersion ();
	} else {
		return fBootableDevice ? fBootableDevice->getActiveBootXVersion () : 0;
	}
}

OSErr
MountedVolume::writeBootBlocksIfNecessary (bool forceInstall)
{
	Ptr bootBlocks = NULL;

	if (!forceInstall) {
		OSErr err = fPartition->readBootBlocks ((void **) &bootBlocks);
		if (err != noErr) return err;
		if ((bootBlocks[0] == 0x4C) && (bootBlocks[1] == 0x4B)) {
			DisposePtr (bootBlocks);
			return noErr;
		}
		DisposePtr (bootBlocks);
	}
		
	gLogFile << "Writing boot blocks ..." << endl_AC;
	bootBlocks = NewPtr (1024);
		
#ifdef __MACH__
	FILE *bootBlockData = fopen ("/usr/share/misc/bootblockdata", "r");
	if (bootBlockData == NULL) {
		gLogFile << "Could not open /usr/share/misc/bootblockdata" << endl_AC;
		DisposePtr (bootBlocks);
		return fnfErr;
	}
	size_t bytesRead = fread (bootBlocks, 1, 1024, bootBlockData);
	if (bytesRead != 1024) {
		gLogFile << "Could not read /usr/share/misc/bootBlockData" << endl_AC;
		DisposePtr (bootBlocks);
		return fnfErr;
	}
	fclose (bootBlockData);
#else
	Handle bootBlockResource = GetResource ('boot', 1);
	if (bootBlockResource == NULL) {
		gLogFile << "Could not find boot block resource" << endl_AC;
		DisposePtr (bootBlocks);
		return fnfErr;
	}
	if (GetHandleSize (bootBlockResource) != 1024) {
		gLogFile << "Boot block resource wrong size" << endl_AC;
		DisposePtr (bootBlocks);
		return fnfErr;
	}
	BlockMoveData (*bootBlockResource, bootBlocks, 1024);
	ReleaseResource (bootBlockResource);
#endif

	if ((bootBlocks[0] != 0x4C) || (bootBlocks[1] != 0x4B)) {
		gLogFile << "Wrong signature in boot block data" << endl_AC;
		DisposePtr (bootBlocks);
		return fnfErr;
	}
	
	OSErr err = fPartition->writeBootBlocks (bootBlocks);

	DisposePtr (bootBlocks);
	return err;
}

void
MountedVolume::installBootX ()
{
#ifdef BUILDING_XPF
	if (((XPFApplication *) gApplication)->getDebugOptions () & kDisableBootX) return;
#endif
	
	if (!getIsWriteable ()) ThrowException_AC (kVolumeNotWriteable, 0);

	if (fBootableDevice) fBootableDevice->installBootXToPartition (fPartition);
}

void
MountedVolume::installBootXFile ()
{
	if (XPFPlatform::GetPlatform()->getIsNewWorld ()) {
		installBootXBootInfoFile ();
	} else {
		installBootXImageFile ();
	}
}

void
MountedVolume::installBootXImageFile ()
{
#ifdef __MACH__
	// Check to make sure we've got partition info to work with
	io_object_t partInfo = getPartitionInfo ();
	if (partInfo) {
		IOObjectRelease (partInfo);
	} else {
		ThrowException_AC (kWritePartitionOSX, 0);
	}
#endif

	ThrowIfNULL_AC (fPartition);

	XPFSetUID myUID (0);
	
	FSRef bootXRef;
	FSSpec bootXSpec;
	try {
		UniChar bootXName[] = {'B', 'o', 'o', 't', 'X', '.', 'i', 'm', 'a', 'g', 'e'};
		OSErr err = FSCreateFileUnicode (getRootDirectory (), 
				sizeof (bootXName) / sizeof (UniChar), bootXName, kFSCatInfoNone, NULL, &bootXRef, NULL);
		if (err == dupFNErr) {
			err = noErr;
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (getRootDirectory (), 
					sizeof (bootXName) / sizeof (UniChar), bootXName, kTextEncodingUnknown, &bootXRef));
			ThrowIfOSErr_AC (FSDeleteObject (&bootXRef));
			ThrowIfOSErr_AC (FSCreateFileUnicode (getRootDirectory (), 
				sizeof (bootXName) / sizeof (UniChar), bootXName, kFSCatInfoNone, NULL, &bootXRef, NULL));
		} else {
			ThrowIfOSErr_AC (err);
		}
	}
	catch (...) {
		#if qLogging
			gLogFile << "Error creating BootX file" << endl_AC;
		#endif
		ThrowException_AC (kErrorExtractingBootX, 0);
	}
	
	CResourceStream_AC stream ('BooX', 128);
	
	SInt16 forkRefNum = 0;
	try {
		XCOFFDecoder decoder (&stream);

		HFSUniStr255 dataForkName;
		FSGetDataForkName (&dataForkName);
		ThrowIfOSErr_AC (FSOpenFork (&bootXRef, dataForkName.length, dataForkName.unicode, 
							fsRdWrPerm, &forkRefNum));
		UInt64 actualCount;
		ThrowIfOSErr_AC (FSAllocateFork (forkRefNum, kFSAllocAllOrNothingMask | kFSAllocContiguousMask,
							fsFromStart, 0, decoder.getSize (), &actualCount));
		if (actualCount < decoder.getSize ()) ThrowException_AC (kErrorExtractingBootX, 0);
		
		decoder.unpackToFork (forkRefNum);
		fPartition->setBootXValues (decoder.getLoadAddress(), decoder.getEntryPoint(), decoder.getSize());
		
		FSCloseFork (forkRefNum);
	}
	catch (...) {
		if (forkRefNum) FSCloseFork (forkRefNum);
		#if qLogging
			gLogFile << "Error extracting BootX" << endl_AC;
		#endif
		throw;
	}
	
#ifdef __MACH__
	sync ();
	sync ();
#endif

	ThrowIfOSErr_AC (FlushVol (NULL, getIOVDrvInfo ()));

	ThrowIfOSErr_AC (FSGetCatalogInfo (&bootXRef, kFSCatInfoNone, NULL, NULL, &bootXSpec, NULL));
 	FSpSetIsInvisible (&bootXSpec);
	
#ifdef BUILDING_XPF	
	// Now I add versioning info to the resource fork.
	Handle bootXVersion = GetResource ('vers', 3);
	ThrowIfResError_AC ();
	Handle xpfVersion = GetResource ('vers', 1);
	ThrowIfResError_AC ();
	DetachResource (bootXVersion);
	DetachResource (xpfVersion);
	ThrowIfResError_AC ();	
	FSpCreateResFile (&bootXSpec, '    ', '    ', smRoman);
	ThrowIfResError_AC ();
	SInt16 resourceFork = FSpOpenResFile (&bootXSpec, fsRdWrPerm);
	ThrowIfResError_AC ();
 	AddResource (bootXVersion, 'vers', 1, "\p");
 	ThrowIfResError_AC ();
 	AddResource (xpfVersion, 'vers', 2, "\p");
 	ThrowIfResError_AC ();
	CloseResFile (resourceFork);
	ThrowIfResError_AC ();
#endif

	XPFBootableDevice::DisableCDDriver (); 	
 	try {
		writeBootBlocksIfNecessary ();
 	
		fPartition->setLgBootStart (fPartition->getBootXStartBlock ());
		if (fPartition->getLgBootStart () == 0) {
			#if qLogging
				gLogFile << "Error finding start block for BootX" << endl_AC;
			#endif
			ThrowException_AC (kErrorExtractingBootX, 0);
		} else {
			fPartition->writePartition ();
		}
	}
	catch (...) {
		XPFBootableDevice::EnableCDDriver ();
		throw;
	}
	XPFBootableDevice::EnableCDDriver ();
	
	checkBootXVersion ();
}

void
MountedVolume::installBootXBootInfoFile ()
{
	XPFSetUID myUID (0);
	
	FSRef bootXRef;
	FSSpec bootXSpec;
	try {
		UniChar bootXName[] = {'B', 'o', 'o', 't', 'X', '.', 'b', 'o', 'o', 't', 'i', 'n', 'f', 'o'};
		OSErr err = FSCreateFileUnicode (getRootDirectory (), 
				sizeof (bootXName) / sizeof (UniChar), bootXName, kFSCatInfoNone, NULL, &bootXRef, NULL);
		if (err == dupFNErr) {
			err = noErr;
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (getRootDirectory (), 
					sizeof (bootXName) / sizeof (UniChar), bootXName, kTextEncodingUnknown, &bootXRef));
			ThrowIfOSErr_AC (FSDeleteObject (&bootXRef));
			ThrowIfOSErr_AC (FSCreateFileUnicode (getRootDirectory (), 
				sizeof (bootXName) / sizeof (UniChar), bootXName, kFSCatInfoNone, NULL, &bootXRef, NULL));
		} else {
			ThrowIfOSErr_AC (err);
		}
	}
	catch (...) {
		gLogFile << "Error creating BootX.bootinfo file" << endl_AC;
		ThrowException_AC (kErrorExtractingBootX, 0);
	}
	
	CResourceStream_AC stream ('BooX', 129);
	
	SInt16 forkRefNum = 0;
	try {
		HFSUniStr255 dataForkName;
		FSGetDataForkName (&dataForkName);
		ThrowIfOSErr_AC (FSOpenFork (&bootXRef, dataForkName.length, dataForkName.unicode, fsRdWrPerm, &forkRefNum));
		char buffer[4096];
		for (SInt32 bytesLeft = stream.GetSize (); bytesLeft > 0; bytesLeft -= 4096) {
			stream.ReadBytes (buffer, Min_AC (bytesLeft, 4096));
			ThrowIfOSErr_AC (FSWriteFork (forkRefNum, fsAtMark, 0, Min_AC (bytesLeft, 4096), buffer, NULL));
		}
		FSCloseFork (forkRefNum);
	}
	catch (...) {
		if (forkRefNum) FSCloseFork (forkRefNum);
		gLogFile << "Error extracting BootX.bootinfo" << endl_AC;
		throw;
	}
	
	ThrowIfOSErr_AC (FSGetCatalogInfo (&bootXRef, kFSCatInfoNone, NULL, NULL, &bootXSpec, NULL));
 	FSpSetIsInvisible (&bootXSpec);
	
#ifdef BUILDING_XPF	
	// Now I add versioning info to the resource fork.
	Handle bootXVersion = GetResource ('vers', 3);
	ThrowIfResError_AC ();
	Handle xpfVersion = GetResource ('vers', 1);
	ThrowIfResError_AC ();
	DetachResource (bootXVersion);
	DetachResource (xpfVersion);
	ThrowIfResError_AC ();	
	FSpCreateResFile (&bootXSpec, '    ', '    ', smRoman);
	ThrowIfResError_AC ();
	SInt16 resourceFork = FSpOpenResFile (&bootXSpec, fsRdWrPerm);
	ThrowIfResError_AC ();
 	AddResource (bootXVersion, 'vers', 1, "\p");
 	ThrowIfResError_AC ();
 	AddResource (xpfVersion, 'vers', 2, "\p");
 	ThrowIfResError_AC ();
	CloseResFile (resourceFork);
	ThrowIfResError_AC ();
#endif

	checkBootXVersion ();
}

void
MountedVolume::turnOffIgnorePermissions ()
{
#ifdef __MACH__
	if (fTurnedOffIgnorePermissions) return;

	char path[1024];
	ThrowIfOSErr_AC (FSRefMakePath (getRootDirectory (), (UInt8 *) path, 1023));
	XPFSetUID myUID (0);
	pid_t pid = fork ();
	if (pid) {
		if (pid != -1) {
			int status;
			waitpid (pid, &status, 0);
		}
	} else {
		setuid (0);	// because vsdbutil checks the uid, rather than the euid
		execl ("/usr/sbin/vsdbutil", "vsdbutil", "-a", path, NULL);
		ThrowException_AC (kInternalError, 0);	// the execl shouldn't return
	}
	
	fTurnedOffIgnorePermissions = true;
#endif
}

UInt32
MountedVolume::getBootXBootInfoVersion ()
{
	UInt32 retVal;
	OSErr err;
	VersRecHndl installedVersion = NULL;
	SInt16 resourceFork = 0;
	retVal = kBootXNotInstalled;

	FSSpec bootXBootInfoSpec;
	err = FSMakeFSSpec (getIOVDrvInfo(), fsRtDirID, "\p:BootX.bootinfo", &bootXBootInfoSpec);
	if (err != noErr) return retVal;

	try {	
		resourceFork = FSpOpenResFile (&bootXBootInfoSpec, fsRdPerm);
		ThrowIfResError_AC ();
		installedVersion = (VersRecHndl) Get1Resource ('vers', 1);
		ThrowIfNULL_AC (installedVersion);
		ThrowIfResError_AC ();
		memcpy (&retVal, &(*installedVersion)->numericVersion, sizeof (retVal));
	}
	catch (...) {
		// There was a problem accessing the version info. So we'll just return.
		gLogFile << "Could not access BootX.bootinfo version info." << endl_AC;
	}
	
	if (installedVersion) ReleaseResource ((Handle) installedVersion);
	if (resourceFork > 0) CloseResFile (resourceFork);
	
	return retVal;
}

UInt32
MountedVolume::getBootXImageVersion ()
{
	UInt32 retVal;
	OSErr err;
	VersRecHndl installedVersion = NULL;
	SInt16 resourceFork = 0;
	retVal = kBootXImproperlyInstalled;

	do {
		// If the partition info doesn't claim that it is installed, then it's not installed
		if (!fPartition || !fPartition->getClaimsBootXInstalled ()) {
			retVal = kBootXNotInstalled;
			continue;
		}

		// OK, check to see if there is a file there 
		FSSpec bootXImageSpec;
		err = FSMakeFSSpec (getIOVDrvInfo(), fsRtDirID, "\p:BootX.image", &bootXImageSpec);
		if (err != noErr) continue;

		// We have a BootX.file. So, we'll see whether it has moved.
		unsigned long firstBootXBlock = 0;
		try {
			XPFBootableDevice::DisableCDDriver ();
			firstBootXBlock = fPartition->getBootXStartBlock ();
			XPFBootableDevice::EnableCDDriver ();
		}
		catch (...) {
			gLogFile << "Error getting BootX start block" << endl_AC;
			XPFBootableDevice::EnableCDDriver ();
		}
			
		if ((firstBootXBlock == 0) || (firstBootXBlock != fPartition->getLgBootStart ())) {
			gLogFile << "BootX.image moved." << endl_AC;
			continue;
		}

		// it hasn't moved. So we'll get the version.
		try {	
			resourceFork = FSpOpenResFile (&bootXImageSpec, fsRdPerm);
			ThrowIfResError_AC ();
			installedVersion = (VersRecHndl) Get1Resource ('vers', 1);
			ThrowIfNULL_AC (installedVersion);
			ThrowIfResError_AC ();
			memcpy (&retVal, &(*installedVersion)->numericVersion, sizeof (retVal));
		}
		catch (...) {
			// There was a problem accessing the version info. So we'll just return.
			gLogFile << "Could not access BootX.image version info." << endl_AC;
			continue;		
		}
		
	} while (false);

	if (installedVersion) ReleaseResource ((Handle) installedVersion);
	if (resourceFork > 0) CloseResFile (resourceFork);
	
	return retVal;
}

void
MountedVolume::checkBootXVersion ()
{
	if (XPFPlatform::GetPlatform()->getIsNewWorld()) {
		fBootXVersion = getBootXBootInfoVersion ();
	} else {
		fBootXVersion = getBootXImageVersion ();
	}
	Changed (cSetBootXVersion, this);
}

void
MountedVolume::setHFSName (HFSUniStr255 *name)
{
	if (FastUnicodeCompare (name->unicode, name->length, fHFSName.unicode, fHFSName.length)) {
		BlockMoveData (name, &fHFSName, sizeof (HFSUniStr255));
		setVolumeName (&fHFSName);
		Changed (cSetVolumeName, this);
	}
}

void
MountedVolume::setVolumeName (HFSUniStr255 *name)
{
	UnicodeToTextInfo converter;		
	ThrowIfOSErr_AC (CreateUnicodeToTextInfoByEncoding (
		CreateTextEncoding (kTextEncodingMacHFS, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat),
		&converter));
	ConvertFromUnicodeToPString (converter, name->length * sizeof (UniChar), name->unicode, fVolumeName);
	DisposeUnicodeToTextInfo (&converter);
}

#ifdef __MACH__

io_object_t
MountedVolume::getRegEntry () {
	char mountPoint[1024], deviceBSDName[32], *shortBSDName;
	deviceBSDName[0] = 0;
	
	OSStatus err = FSRefMakePath (getRootDirectory (), (UInt8 *) mountPoint, 1023);
	if (err != noErr) return NULL;
	
	int numFS = getfsstat (NULL, 0, MNT_NOWAIT);
	struct statfs *fs = (struct statfs *) malloc (numFS * sizeof (struct statfs));
	getfsstat (fs, numFS * sizeof (struct statfs), MNT_NOWAIT);

	for (int x = 0; x < numFS; x++) {
		if (!strcmp (fs[x].f_mntonname, mountPoint)) {
			strcpy (deviceBSDName, fs[x].f_mntfromname);
			break;
		}
	}
	
	free (fs);
				
	if (!deviceBSDName[0]) return NULL;
				
	shortBSDName = deviceBSDName;
	if (!strncmp (shortBSDName, "/dev/", 5)) shortBSDName += 5;
		
	mach_port_t iokitPort;
	io_object_t retVal = NULL;
	io_iterator_t iter = NULL;
	
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	IOServiceGetMatchingServices (iokitPort, IOBSDNameMatching (iokitPort, NULL, shortBSDName), &iter);
	if (iter) {
		retVal = IOIteratorNext (iter);
		IOObjectRelease (iter);
	}
	
	return retVal;
}

io_object_t
MountedVolume::getPartitionInfo () {
	io_object_t retVal = NULL;
	io_object_t regEntry = getRegEntry ();
	if (regEntry) {
		io_iterator_t iter = NULL;
		IORegistryEntryCreateIterator (regEntry, kIOServicePlane, 0, &iter);
		if (iter) {
			io_object_t child;
			while ((child = IOIteratorNext (iter)) != NULL) {
				if (IOObjectConformsTo (child, "XPFPartitionInfo")) {
					retVal = child;
					break;
				} else {
					IOObjectRelease (child);
				}
			}
			IOObjectRelease (iter);
		}
		IOObjectRelease (regEntry);
	}
	return retVal;
}

#endif

OSErr
MountedVolume::blessCoreServicesFolder ()
{
	if (!fCoreServicesFolderNodeID) return noErr;
	if (fCoreServicesFolderNodeID == fBlessedFolderID) return noErr;
	
	FSVolumeInfo volInfo;
	OSErr err = FSGetVolumeInfo (fInfo.driveNumber, 0, NULL, kFSVolInfoFinderInfo, &volInfo, NULL, NULL);
	if (err == noErr) {
		UInt32 *finderInfo = (UInt32 *) volInfo.finderInfo;
		finderInfo[0] = fCoreServicesFolderNodeID;
		finderInfo[5] = fCoreServicesFolderNodeID;
		err = FSSetVolumeInfo (fInfo.driveNumber, kFSVolInfoFinderInfo, &volInfo);
	}
	
	checkBlessedFolder ();
	
	return err;
}

OSErr
MountedVolume::blessMacOS9SystemFolder ()
{
	if (!fMacOS9SystemFolderNodeID) return noErr;
	if (fMacOS9SystemFolderNodeID == fBlessedFolderID) return noErr;
	
	FSVolumeInfo volInfo;
	OSErr err = FSGetVolumeInfo (fInfo.driveNumber, 0, NULL, kFSVolInfoFinderInfo, &volInfo, NULL, NULL);
	if (err == noErr) {
		UInt32 *finderInfo = (UInt32 *) volInfo.finderInfo;
		finderInfo[0] = fMacOS9SystemFolderNodeID;
		finderInfo[3] = fMacOS9SystemFolderNodeID;
		err = FSSetVolumeInfo (fInfo.driveNumber, kFSVolInfoFinderInfo, &volInfo);
	}
	
	checkBlessedFolder ();
	
	return err;
}

static bool IsMacOS9SystemFolder (FSRef *directory)
{
	bool hasFinder = false;
	bool hasSystem = false;
	FSIterator iterator = NULL;
	OSErr err = FSOpenIterator (directory, kFSIterateFlat, &iterator);
	
	while ((err == noErr) && (!hasFinder || !hasSystem)) {
		ItemCount actualObjects;
		FSCatalogInfo catInfo;
		FSRef item;
		err = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catInfo, &item, NULL, NULL);
		if ((err == noErr) && (actualObjects == 1)) {
			if (!(catInfo.nodeFlags & kFSNodeIsDirectoryMask)) {
				FileInfo *fileInfo = (FileInfo *) catInfo.finderInfo;
				if ((fileInfo->fileType == 'zsys') && (fileInfo->fileCreator == 'MACS')) hasSystem = true;
				if ((fileInfo->fileType == 'FNDR') && (fileInfo->fileCreator == 'MACS')) hasFinder = true;
			}
		}
	}
	
	if (iterator) FSCloseIterator (iterator);
	return hasSystem && hasFinder;	
}

void
MountedVolume::fixSymlinkAtPath (char *path)
{
	if (getSymlinkStatusForPath (path) == kSymlinkStatusCannotFix) return;
	
	char linkPath[256];
	sprintf (linkPath, "private/%s", path);
	
#ifdef __MACH__	
	char filePath[1024];
	ThrowIfOSErr_AC (FSRefMakePath (getRootDirectory (), (UInt8 *) filePath, 1023));
	strcat (filePath, "/");
	strcat (filePath, path);
		
	XPFSetUID myUID (0);
	unlink (filePath);
	symlink (linkPath, filePath);
#else
	FSRef filePath;
	OSErr err = XPFFSRef::getOrCreateFile (getRootDirectory (), path, 0755, &filePath);
	if (err != noErr) return;

	HFSUniStr255 dataForkName;
	FSGetDataForkName (&dataForkName);
	SInt16 forkRefNum;
	err = FSOpenFork (&filePath, dataForkName.length, dataForkName.unicode, fsWrPerm, &forkRefNum);
	if (err != noErr) return;

	FSWriteFork (forkRefNum, fsFromStart, 0, strlen (linkPath), linkPath, NULL);
	FSSetForkSize (forkRefNum, fsAtMark, 0);
	FSCloseFork (forkRefNum);
	
	FSCatalogInfo catInfo;
	err = FSGetCatalogInfo (&filePath, kFSCatInfoPermissions | kFSCatInfoFinderInfo, &catInfo, NULL, NULL, NULL);
	if (err == noErr) {
		FileInfo *info = (FileInfo *) &catInfo.finderInfo;
		info->fileType = 'slnk';
		info->fileCreator = 'rhap';
		info->finderFlags |= kIsInvisible | kIsAlias;
		catInfo.permissions[2] &= ~S_IFMT;
		catInfo.permissions[2] |= S_IFLNK;
		FSSetCatalogInfo (&filePath, kFSCatInfoPermissions | kFSCatInfoFinderInfo, &catInfo);
	}
#endif

	checkSymlinks ();
}

unsigned
MountedVolume::getSymlinkStatusForPath (char *path)
{
	int linkSize;
	char linkContents[256] = {0};
	OSErr err;
	mode_t mode;
	
#ifdef __MACH__
	char filePath[1024];
	ThrowIfOSErr_AC (FSRefMakePath (getRootDirectory (), (UInt8 *) filePath, 1023));
	strcat (filePath, "/");
	strcat (filePath, path);
#else
	FSRef filePath;
	err = XPFFSRef::getFSRef (getRootDirectory (), path, &filePath);
	if (err != noErr) return kSymlinkStatusMissing;
#endif
	
#ifdef __MACH__
	struct stat sb;
	err = lstat (filePath, &sb);
	if (err) return kSymlinkStatusMissing;
	mode = sb.st_mode;
#else
	FSCatalogInfo catInfo;
	err = FSGetCatalogInfo (&filePath, kFSCatInfoNodeFlags | kFSCatInfoPermissions, &catInfo, NULL, NULL, NULL);
	if (err != noErr) return kSymlinkStatusMissing;
	if (catInfo.nodeFlags & kFSNodeIsDirectoryMask) return kSymlinkStatusCannotFix;
	mode = catInfo.permissions[2];
#endif
	
	switch (mode & S_IFMT) {
		case S_IFLNK:
#ifdef __MACH__		
			// In Mac OS X, we do the readlink and break
			// Otherwise, we fall through to read the file
			linkSize = readlink (filePath, linkContents, 255);
			if (linkSize == -1) return kSymlinkStatusCannotFix;
			linkContents[linkSize] = 0;
			break;
#endif

		case S_IFREG:
		case 0:	
			// We need to check the content, because that will affect whether we think we
			// can fix it or not. That is, we'll decline to fix it if the content is wrong.
#ifdef __MACH__
			int file = open (filePath, O_RDONLY, 0);
			if (file == -1) return kSymlinkStatusCannotFix;
			linkSize = read (file, linkContents, 255);
			close (file);
#else
			HFSUniStr255 dataForkName;
			FSGetDataForkName (&dataForkName);
			SInt16 forkRefNum;
			err = FSOpenFork (&filePath, dataForkName.length, dataForkName.unicode, fsRdPerm, &forkRefNum);
			if (err != noErr) return kSymlinkStatusCannotFix;
			err = FSReadFork (forkRefNum, fsFromStart, 0, 255, linkContents, (ByteCount *) &linkSize);
			FSCloseFork (forkRefNum);
			if ((err != noErr) && (err != eofErr)) return kSymlinkStatusCannotFix;			
#endif
			if (linkSize != -1) linkContents[linkSize] = 0;
			break;
			
		default:
			return kSymlinkStatusCannotFix;
			break;
	}
	
	if (strncmp (linkContents, "private/", strlen ("private/"))) return kSymlinkStatusCannotFix;
	if (strcmp (linkContents + strlen ("private/"), path)) return kSymlinkStatusCannotFix;
	
	// If the content was OK, then we return based on whether the link attribute was set
	return (mode & S_IFMT) == S_IFLNK ? kSymlinkStatusOK : kSymlinkStatusInvalid;
}

const UInt64 kMaxCacheFileSize = 16 * 1024 * 1024;

#ifndef __MACH__
	#undef S_IWGRP
	#undef S_IWOTH
	#define	S_IWGRP	0000020			/* W for group */
	#define	S_IWOTH	0000002			/* W for other */
#endif		
		
bool
MountedVolume::isCacheFileOK (FSRef *ref)
{
	FSCatalogInfo catInfo;
	OSErr err = FSGetCatalogInfo (ref, kFSCatInfoPermissions | kFSCatInfoDataSizes | kFSCatInfoContentMod, &catInfo, NULL, NULL, NULL);
	if (err == noErr) {
		if (catInfo.dataPhysicalSize > kMaxCacheFileSize) {
			gLogFile << "Cache file too large" << endl_AC;
			return false;
		}
		
		if (catInfo.dataPhysicalSize == 0) {
			gLogFile << "Cache file has zero size" << endl_AC;
			return false;
		}

		UTCDateTime dateTime;
		err = GetUTCDateTime (&dateTime, kUTCDefaultOptions);
		if ((err == noErr) && (*(UInt64*) &catInfo.contentModDate > *(UInt64*) &dateTime)) {
			gLogFile << "Cache file modification date is in future" << endl_AC;
			return false;
		}
		
#ifdef __MACH__
		if (!fTurnedOffIgnorePermissions && (catInfo.permissions[0] == geteuid ()) && (catInfo.permissions[1] == 99)) {
			catInfo.permissions[0] = 0;
			catInfo.permissions[1] = 0;		
		}
#endif

		if (catInfo.permissions[0] != 0) {
			// It appears that BootX only checks the owner, not the group
			gLogFile << "Cache file owner or group is incorrect" << endl_AC;
			return false;
		}
		
		if ((catInfo.permissions[2] & S_IWGRP) || (catInfo.permissions[2] & S_IWOTH)) {
			gLogFile << "Cache file is writeable by group or world" << endl_AC;
			return false;
		}
	}
	
	return true;
}

void
MountedVolume::checkExtensionsCaches ()
{
	fExtensionCachesOK = true;
	if (!fBootableDevice) return;

	FSRef ref;
	OSErr err;
	
	err = XPFFSRef::getExtensionsCacheFSRef (&fRootDirectory, &ref, false);
	if ((err == noErr) && !isCacheFileOK (&ref)) {
		fExtensionCachesOK = false;
		gLogFile << "--> Extensions.mkext" << endl_AC;
		return;
	}
	
	err = XPFFSRef::getKextCacheFSRef (&fRootDirectory, &ref, false);
	if ((err == noErr) && !isCacheFileOK (&ref)) {
		fExtensionCachesOK = false;
		gLogFile << "--> Extensions.kextcache" << endl_AC;
		return;
	}
	
	err = XPFFSRef::getOrCreateKernelCacheDirectory (&fRootDirectory, &ref, false);
	if (err == noErr) {
		FSIterator iterator = NULL;
		err = FSOpenIterator (&ref, kFSIterateFlat, &iterator);
		while (err == noErr) {
			ItemCount actualObjects;
			FSRef item;
			FSCatalogInfo catInfo;
			HFSUniStr255 filename;
			err = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, kFSCatInfoNodeFlags, &catInfo, &item, NULL, &filename);
			if ((err == noErr) && (actualObjects == 1) && !(catInfo.nodeFlags & kFSNodeIsDirectoryMask)) {
				UniChar kernelCache[] = {'k', 'e', 'r', 'n', 'e', 'l', 'c', 'a', 'c', 'h', 'e'};
				if (!FastUnicodeCompare (filename.unicode, 11, kernelCache, 11) && !isCacheFileOK (&item)) {
					fExtensionCachesOK = false;
					gLogFile << "--> com.apple.kernelcache" << endl_AC;
				}
			}
		}
		if (iterator) FSCloseIterator (iterator);
	}
	
	Changed (cSetExtensionsCacheStatus, this);
}

void
MountedVolume::checkSymlinks ()
{
	fSymlinkStatus = kSymlinkStatusOK;

	if (!fHasMachKernel) return;
	
	unsigned status = getSymlinkStatusForPath ("etc");
	if (status > fSymlinkStatus) fSymlinkStatus = status;
	status = getSymlinkStatusForPath ("var");
	if (status > fSymlinkStatus) fSymlinkStatus = status;
	status = getSymlinkStatusForPath ("tmp");
	if (status > fSymlinkStatus) fSymlinkStatus = status;
	
	if (!getIsWriteable () && (fSymlinkStatus != kSymlinkStatusOK)) fSymlinkStatus = kSymlinkStatusCannotFix;
	
	Changed (cSetSymlinkStatus, this);
}

void
MountedVolume::checkBlessedFolder ()
{
	fMacOS9SystemFolderNodeID = 0;
	fCoreServicesFolderNodeID = 0;
	fBlessedFolderID = 0;
	
	UInt32 currentlyBlessedMacOS9FolderNodeID = 0;
	UInt32 currentlyBlessedCoreServicesFolderNodeID = 0;
	
	FSVolumeInfo volInfo;
	OSErr err = FSGetVolumeInfo (fInfo.driveNumber, 0, NULL, kFSVolInfoFinderInfo, &volInfo, NULL, NULL);
	if (err == noErr) {
		UInt32 *finderInfo = (UInt32 *) volInfo.finderInfo;
		fBlessedFolderID = finderInfo[0];
		currentlyBlessedMacOS9FolderNodeID = finderInfo[3];
		currentlyBlessedCoreServicesFolderNodeID = finderInfo[5];
	}
	
	FSIterator iterator = NULL;
	err = FSOpenIterator (&fRootDirectory, kFSIterateFlat, &iterator);
	
	while ((err == noErr) && (fMacOS9SystemFolderNodeID == 0)) {
		ItemCount actualObjects;
		FSCatalogInfo catInfo;
		FSRef item;
		err = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, kFSCatInfoNodeFlags | kFSCatInfoNodeID, &catInfo, &item, NULL, NULL);
		if ((err == noErr) && (actualObjects == 1)) {
			if (catInfo.nodeFlags & kFSNodeIsDirectoryMask) {
				if (IsMacOS9SystemFolder (&item)) {
					fMacOS9SystemFolderNodeID = catInfo.nodeID;
					// stop looking if we have found what is currently blessed
					// otherwise, we keep going even though we've found one
					if (currentlyBlessedMacOS9FolderNodeID == catInfo.nodeID) break;
				}
			}
		}
	}
	
	if (iterator) FSCloseIterator (iterator);
	
	FSRef coreServicesFolder;
	err = XPFFSRef::getOrCreateCoreServicesDirectory (&fRootDirectory, &coreServicesFolder, false);
	if (err == noErr) {
		FSCatalogInfo catInfo;
		err = FSGetCatalogInfo (&coreServicesFolder, kFSCatInfoNodeFlags | kFSCatInfoNodeID, &catInfo, NULL, NULL, NULL);
		if ((err == noErr) && (catInfo.nodeFlags & kFSNodeIsDirectoryMask)) {
			fCoreServicesFolderNodeID = catInfo.nodeID;
		}
	}

	// If I couldn't find blessed folders, then may as well use what's currently there, just in case.
	if (!fMacOS9SystemFolderNodeID) fMacOS9SystemFolderNodeID = currentlyBlessedMacOS9FolderNodeID;
	if (!fCoreServicesFolderNodeID) fCoreServicesFolderNodeID = currentlyBlessedCoreServicesFolderNodeID;

	Changed (cSetBlessedFolderID, this);
	
	gLogFile << "Blessed folder: " << fBlessedFolderID << " Mac OS 9 System Folder: " << fMacOS9SystemFolderNodeID << endl_AC;
}

void
MountedVolume::checkMacOSXVersion ()
{
	CFSSpec_AC versionSpec;
	OSErr err = FSMakeFSSpec (fInfo.driveNumber, fsRtDirID, "\p:System:Library:CoreServices:SystemVersion.plist", &versionSpec);
	if (err == fnfErr) err = FSMakeFSSpec (fInfo.driveNumber, fsRtDirID, "\p:System:Library:Extensions:System.kext:Info.plist", &versionSpec);
	if (err == fnfErr) err = FSMakeFSSpec (fInfo.driveNumber, fsRtDirID, "\p:System:Library:Extensions:System.kext:Contents:Info.plist", &versionSpec);
	if (err == noErr) {
		CFile_AC versionFile;
		versionFile.Specify (versionSpec);
		err = versionFile.OpenDataFork ();
		long dataSize;
		err = versionFile.GetDataLength (dataSize);
		if (dataSize && (dataSize < 4096) && (err == noErr)) {
			Ptr versionData = NewPtr (dataSize + 1);
			err = versionFile.ReadData (versionData, dataSize);
			if (err == noErr) {
				versionData[dataSize] = 0;
				char *key = strstr (versionData, "<key>ProductUserVisibleVersion</key>");
				if (!key) key = strstr (versionData, "<key>ProductVersion</key>");
				if (!key) key = strstr (versionData, "<key>CFBundleVersion</key>");
				if (key) {
					char *start = strstr (key, "<string>");
					if (start) {
						start += strlen ("<string>");
						char *end = strstr (key, "</string>");
						if (end) {
							char tmp = *end;
							*end = 0;
							fMacOSXVersion.CopyFrom (start);
							*end = tmp;
						}
					}
				}
				key = strstr (versionData, "<key>ProductBuildVersion</key>");
				if (!key) {
					key = strstr (versionData, "<key>CFBundleVersion</key>");
					if (key) fIsDarwin = true;
				}
				if (key) {
					char *start = strstr (key, "<string>");
					if (start) {
						start += strlen ("<string>");
						char *end = strstr (key, "</string>");
						if (end) {
							*end = 0;
							fMacOSXMajorVersion = strtoul (start, 0, 0);
						}
					}
				}
			}
			DisposePtr (versionData);	
		}
		versionFile.CloseDataFork ();
	}
}

void
MountedVolume::checkDeviceAndPartition ()
{
#ifdef __MACH__
	io_object_t regEntry = getRegEntry ();
	if (regEntry) {
		fBootableDevice = XPFBootableDevice::DeviceForRegEntry (regEntry);
		if (fBootableDevice) fPartition = fBootableDevice->partitionWithInfoAndName (&fInfo, &fHFSName);
		IOObjectRelease (regEntry);
	}
#else
	fBootableDevice = XPFBootableDevice::DeviceWithInfo (&fInfo);
	
	if (!fBootableDevice && MoreDriveSupportsDriverGestalt (fInfo.driveNumber)) {
		DriverGestaltParam pbGestalt;
		Erase_AC (&pbGestalt);

		pbGestalt.ioVRefNum = fInfo.driveNumber;
		pbGestalt.ioCRefNum	= fInfo.driverRefNum;
		pbGestalt.csCode = kDriverGestaltCode;
		pbGestalt.driverGestaltSelector = kdgNameRegistryEntry;

		OSErr status = PBStatusSync ((ParmBlkPtr) &pbGestalt);

		if (status == noErr) {
			char alias [1024] = {0};
			OFAliases::AliasFor (GetDriverGestaltNameRegistryResponse (&pbGestalt)->entryID, alias, NULL);
			fBootableDevice = XPFBootableDevice::DeviceWithOpenFirmwareName (alias);
		}		
	}

	if (!fBootableDevice) fBootableDevice = XPFBootableDevice::DeviceWithInfoAndName (&fInfo, &fHFSName);

	if (fBootableDevice) {
		if (fBootableDevice->isFirewireDevice ()) {
			// Try MoreGetPartitionInfo
			partInfoRec partInfo;
			OSErr err = MoreGetPartitionInfo (fInfo.driveNumber, &partInfo);

			if (err != noErr) {
				// Try DriverGestalt
				DriverGestaltParam pbGestalt;
				Erase_AC (&pbGestalt);

				pbGestalt.ioVRefNum = fInfo.driveNumber;
				pbGestalt.ioCRefNum	= fInfo.driverRefNum;
				pbGestalt.csCode = kDriverGestaltCode;
				pbGestalt.driverGestaltSelector = kdgOpenFirmwareBootSupport;

				err = PBStatusSync ((ParmBlkPtr) &pbGestalt);

				if (err == noErr) partInfo.partitionNumber = GetDriverGestaltOFBootSupportResponse (&pbGestalt)->bootPartitionMapEntry;
			}
			
			if (err != noErr) {
				// This is a terrible hack, but I happen to know that the Mac OS X Installer puts
				// it's HFS volume on partition 9, so ...
				if (fHasInstaller) {
					partInfo.partitionNumber = 9;
					err = noErr;
				}
			}
			
			if (err == noErr) fPartitionNumber = partInfo.partitionNumber;
		} else {
			fPartition = fBootableDevice->partitionWithInfoAndName (&fInfo, &fHFSName);
		}
	}
#endif

	if (fPartition) {
		fPartition->setMountedVolume (this);
		fPartitionNumber = fPartition->getPartitionNumber ();
		fPartition->AddDependent (this);
	}
}

MountedVolume::MountedVolume (FSVolumeInfo *info, HFSUniStr255 *name, FSRef *rootDirectory)
{
	fPartition = NULL;
	fBootableDevice = NULL;
	fHelperDisk = NULL;
	fCreationDate = 0;
	fMacOSXVersion = "";
	fMacOSXMajorVersion = 0;
#ifdef __MACH__
	fTurnedOffIgnorePermissions = false;
#else
	fTurnedOffIgnorePermissions = true;
#endif
	fSymlinkStatus = kSymlinkStatusOK;
	fIsAttachedToPCICard = false;
	fBootXVersion = 0;
	fPartitionNumber = 0;
	fOpenFirmwareName[0] = 0;
	fShortOpenFirmwareName[0] = 0;
	fIsDarwin = false;
	fFullyInitialized = false;
	fExtensionCachesOK = true;
	fMacOS9SystemFolderNodeID = 0;
	fCoreServicesFolderNodeID = 0;
	fBlessedFolderID = 0;
	fAllocationBlockSize = 0;
	fIsWriteable = true;
	fHasMachKernel = false;
	fIsHFSPlus = false;
	fHasInstaller = false;
	fHasFinder = false;
	fIsDarwin = false;
	
	UInt32 physicalRAMSize;
	Gestalt (gestaltPhysicalRAMSize, (SInt32 *) &physicalRAMSize);
	fMachineHasSufficientRAM = physicalRAMSize >= kBootXRAMRequired;

	// Wrap entire method in try block, to catch exceptions
	// The idea is to show all devices, even if there is an error initializing them
	try {
	
		#ifdef BUILDING_XPF
			gApplication->AddDependent (this); // for listening for volume deletions
		#endif
		
		// Copy the info
		#ifdef __MACH__
			// It looks like I'm not getting a useable drive number from the info
			// So I'll try getting it from the rootDirectory instead
			FSSpec rootDirSpec;
			FSGetCatalogInfo (rootDirectory, kFSCatInfoNone, NULL, NULL, &rootDirSpec, NULL);
			info->driveNumber = rootDirSpec.vRefNum;
		#endif
 	
		BlockMoveData (info, &fInfo, sizeof (FSVolumeInfo));
		BlockMoveData (name, &fHFSName, sizeof (HFSUniStr255));	
		setVolumeName (&fHFSName);
		fCreationDate = info->createDate.lowSeconds;
		fAllocationBlockSize = info->blockSize;
		BlockMoveData (rootDirectory, &fRootDirectory, sizeof (fRootDirectory));
			
		// See if it's got a mach_kernel
		CFSSpec_AC kernel;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:mach_kernel", &kernel);
		fHasMachKernel = (err == noErr);
			
		// See if it's an installer
		CFSSpec_AC installer;
		err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:CDIS:", &installer);
		// check for Darwin as well
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:packages:Essentials.pkg", &installer);
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:RPMS:", &installer);
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:AppleBinary:", &installer);
		fHasInstaller = (err == noErr);
				
		// See if it's writeable
		fIsWriteable = !(info->flags & (kFSVolFlagHardwareLockedMask | kFSVolFlagSoftwareLockedMask));
		
		// Pretend that all installers are not writeable
		// Because they are constructed in a special way, and we shouldn't interfere with that
		if (fHasInstaller) fIsWriteable = false;
		
		// See if it's HFS Plus
		fIsHFSPlus = (info->signature == kHFSPlusSigWord);
		
		// See if it has a Finder (i.e. whether it might be a Darwin disk)
		CFSSpec_AC finderSpec;
		err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Library:CoreServices:Finder.app:", &finderSpec);
		fHasFinder = (err == noErr);
		
		gLogFile << "MountedVolume::MountedVolume fVolumeName: " << (CChar255_AC) fVolumeName << " Creation Date: " << fCreationDate << endl_AC;
		
		checkDeviceAndPartition ();
		
		// We are only interested in the rest if we have a bootable device
		if (fBootableDevice) {
			checkSymlinks ();
			checkBlessedFolder ();
			checkBootXVersion ();
			checkMacOSXVersion ();
			checkOpenFirmwareName ();
			checkExtensionsCaches ();
		}
					
		gLogFile << "MountedVolume::MountedVolume OpenFirmwareName: " << fShortOpenFirmwareName << endl_AC;

		fFullyInitialized = true;
	}

	catch (CException_AC &ex) {
		CString_AC errString;
		LookupErrString (ex.GetError (), errReasonID, errString);
		gLogFile << "MountedVolume::MountedVolume error: " << ex.GetError() << ": " << (CChar255_AC) errString << endl_AC;
	}

	catch (...) {
		gLogFile << "MountedVolume::MountedVolume error: unknown" << endl_AC;
	}
}

void
MountedVolume::checkOpenFirmwareName ()
{
#ifdef __MACH__
	OFAliases::AliasFor (getRegEntry (), fOpenFirmwareName, fShortOpenFirmwareName);
#else
	if (fBootableDevice && fPartitionNumber) {
		sprintf (fOpenFirmwareName, "%s:%d", fBootableDevice->getOpenFirmwareName (false), fPartitionNumber);
		sprintf (fShortOpenFirmwareName, "%s:%d", fBootableDevice->getOpenFirmwareName (true), fPartitionNumber);
	}
#endif

	// There are more robust ways to figure this out, but this will probably do
	fIsAttachedToPCICard = !strncmp (fOpenFirmwareName, "pci", 3); 

	Changed (cSetOpenFirmwareName, this);
}

bool
MountedVolume::getRequiresBootHelper () 
{
	if (!fBootableDevice) return false;
	return fBootableDevice->getNeedsHelper () || !getIsWriteable ();
}

bool
MountedVolume::getWillRunOnCurrentCPU ()
{
	if (!fMacOSXMajorVersion) return true;

	bool retVal = true;

	long cpuType;
	if (Gestalt (gestaltNativeCPUtype, &cpuType) == noErr) {
		switch (cpuType) {
			case gestaltCPU601:
				retVal = false;
				break;
				
			case gestaltCPU603:
			case gestaltCPU604:
			case gestaltCPU603e:
			case gestaltCPU603ev:
			case gestaltCPU604e:
			case gestaltCPU604ev:
				retVal = fMacOSXMajorVersion < 7; 
				break;
		}
	}
	
	if (!retVal) gLogFile << "Gestalt gestaltNativeCPUType: " << cpuType << endl_AC;
	
	return retVal;
}

unsigned
MountedVolume::getHelperStatus ()
{
	if (!fFullyInitialized) return kNotInitialized;
	if (!getBootableDevice ()) return kNotBootable;
	if (!strcmp (getOpenFirmwareName (false), "")) return kNotBootable;
	if (!getIsWriteable ()) return kNotWriteable;
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!fPartitionNumber) return kNoPartitionNumber;
	if (fBootableDevice->getNeedsHelper ()) return kNeedsHelper;
	if (!fMachineHasSufficientRAM) return kNotSufficientRAM;

	return kStatusOK;
}

unsigned
MountedVolume::getBootStatus ()
{
	if (!fFullyInitialized) return kNotInitialized;
	if (!getBootableDevice ()) return kNotBootable;
	if (!strcmp (getOpenFirmwareName (false), "")) return kNotBootable;
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getHasMachKernel ()) return kNoMachKernel;
	if (!fPartitionNumber) return kNoPartitionNumber;
//	if (!getWillRunOnCurrentCPU ()) return kCPUNotSupported;
	if (!fMachineHasSufficientRAM) return kNotSufficientRAM;

	return kStatusOK;
}

unsigned
MountedVolume::getMacOS9BootStatus ()
{
	if (!fFullyInitialized) return kNotInitialized;
	if (!getBootableDevice ()) return kNotBootable;
	if (!strcmp (getOpenFirmwareName (false), "")) return kNotBootable;
	if (!fPartitionNumber) return kNoPartitionNumber;
	if (!fMacOS9SystemFolderNodeID) return kNoMacOS9SystemFolder;

	return kStatusOK;
}

unsigned
MountedVolume::getBootWarning (bool forInstall)
{
	// We only worry about warnings if there is no fatal problem
	if (getInstallTargetStatus () && getInstallerStatus ()) return kStatusOK;
		
	if (!fExtensionCachesOK) return kExtensionsCacheInvalid;

	if (fSymlinkStatus == kSymlinkStatusCannotFix) return kInvalidSymlinksCannotFix;
	if (fSymlinkStatus != kSymlinkStatusOK) return kInvalidSymlinks;

	if (!getWillRunOnCurrentCPU ()) return kCPUNotSupported;

	if (fBootableDevice) {
		// FIXME -- I suppose that I should really check the *helper* disk for things that
		// need to be warned about if we're using a helper (e.g. 8 GB limit etc.). But that
		// would require some UI changes as well.
	
		if (!getHelperDisk ()) {
			XPFPartition* firstPart = fBootableDevice->getFirstHFSPartition ();
			if (firstPart && firstPart->getPartitionNumber () < kExpectedFirstHFSPartition) return kFewerPartitionsThanExpected;

			if (fPartition && !fPartition->getHasHFSWrapper ()) return kNoHFSWrapper;
		}

		if (fBootableDevice->isReallyATADevice () && 
			getExtendsPastEightGB () && 
			!XPFPlatform::GetPlatform()->getIsNewWorld() &&
			!fIsAttachedToPCICard && 
			!getHelperDisk ()
		) return k8GBWarning;
	}
	
	if (getBus () != getDefaultBus ()) return kUsingNonDefaultBus;
	
	return kStatusOK;
}

unsigned
MountedVolume::getInstallTargetStatus ()
{
	if (!fFullyInitialized) return kNotInitialized;
	if (!getBootableDevice ()) return kNotBootable;
	if (!strcmp (getOpenFirmwareName (false), "")) return kNotBootable;
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getIsWriteable ()) return kNotWriteable;
	if (!fPartitionNumber) return kNoPartitionNumber;
	if (!fMachineHasSufficientRAM) return kNotSufficientRAM;

	return kStatusOK;
}

unsigned
MountedVolume::getInstallerStatus ()
{
	if (!fFullyInitialized) return kNotInitialized;
	if (!getHasInstaller ()) return kNotInstaller;

	if (!getBootableDevice ()) return kNotBootable;
	if (!strcmp (getOpenFirmwareName (false), "")) return kNotBootable;
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getHasMachKernel ()) return kNoMachKernel;
	if (!fPartitionNumber) return kNoPartitionNumber;
//	if (!getWillRunOnCurrentCPU ()) return kCPUNotSupported;
	if (!fMachineHasSufficientRAM) return kNotSufficientRAM;
	
	return kStatusOK;
}

void
MountedVolume::setHelperDisk (MountedVolume *disk, bool callChanged)
{
	if (fHelperDisk != disk) {
		fHelperDisk = disk;
		if (callChanged) Changed (cSetHelperDisk, this);
	}
}

XPFBus*
MountedVolume::getBus ()
{
	return fBootableDevice ? fBootableDevice->getBus () : NULL;
}

XPFBus*
MountedVolume::getDefaultBus ()
{
	return fBootableDevice ? fBootableDevice->getDefaultBus () : NULL;
}

void 
MountedVolume::setBus (XPFBus *bus)
{
	if (fBootableDevice) fBootableDevice->setBus (bus);
}

CVoidList_AC*
MountedVolume::getBusList ()
{
	return fBootableDevice ? fBootableDevice->getBusList () : NULL;
}

void 
MountedVolume::DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace)
{
	#pragma unused (changedObject, dependencySpace)
	
	MountedVolume *volume = (MountedVolume *) changeData;
	
	switch (theChange) {			
		case cDeleteMountedVolume:
			if (volume == fHelperDisk) setHelperDisk (getDefaultHelperDisk ());
			break;
			
		case cSetBootXVersion:
			Changed (cSetBootXVersion, this);
			break;
			
		case cSetBus:
			Changed (cSetBus, this);
			break;
			
		case cSetOpenFirmwareName:
			checkOpenFirmwareName ();
			break;
	}
}
