/*

Copyright (c) 2000 - 2003
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
#include "XPFAuthorization.h"

#include <iostream.h>
#include "XPostFacto.h"
#include "XPFStrings.h"
#include "OFAliases.h"
#include "vers_rsrc.h"
#include "XPFFSRef.h"
#include <stdio.h>

#include "MoreDisks.h"

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
			MountedVolume *volume = MountedVolume::WithInfo (&info);
			if (volume) {
				volume->fStillThere = true;
				// catch name changes for the UI
				volume->setHFSName (&volName);
			} else {
				try {
					volume = new MountedVolume (&info, &volName, &rootDirectory);
					gVolumeList.InsertLast (volume);
#ifdef BUILDING_XPF
					gApplication->Changed (cNewMountedVolume, volume);
#endif
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
			
	#if qLogging
		gLogFile << "Finished initializing Mounted Volumes" << endl_AC;
	#endif
}

void
MountedVolume::readHelperFromStream (CFileStream_AC *stream)
{
	FSVolumeInfo volInfo;
	stream->ReadBytes (&volInfo, sizeof (volInfo));
	MountedVolume *helper = MountedVolume::WithInfo (&volInfo);
	if (helper) {
		fHelperDisk = helper;
	} else {
		fHelperDisk = getDefaultHelperDisk ();
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
		if (iter->getIsOnBootableDevice () && !iter->getRequiresBootHelper () && (iter.Current() != this)) {
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
		if (iter->getBootStatus () == kStatusOK) {
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
MountedVolume::WithOpenFirmwarePath (char *path)
{
	MountedVolume *retVal = NULL;
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
		CChar255_AC volPath = (CChar255_AC) iter->getOpenFirmwareName (false);
		if (OFAliases::MatchAliases (volPath, path)) {
			retVal = iter.Current ();
			break;
		}
	}	
#endif
	return retVal;
}

void
MountedVolume::installBootXIfNecessary (bool forceInstall)
{
#ifdef BUILDING_XPF
	if (((XPFApplication *) gApplication)->getDebugOptions () & kDisableBootX) return;
#endif
	
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

bool
MountedVolume::hasCurrentExtensions (bool useCacheConfig)
{
	FSRef extensionsDir;
	OSErr err = XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (&fRootDirectory, &extensionsDir, false);
	if (err != noErr) return false;

	SInt16 archiveCount = CountResources ('hfsA');
	for (SInt16 x = 1; x <= archiveCount; x++) {
		SInt16 resourceID;
		CStr255_AC resourceName;
		char resourceVersion[128], fileVersion[128];
		resourceVersion[0] = 0;
		fileVersion[0] = 0;
		
		// Get the resource version

		SetResLoad (false);
		Handle hfsA = GetIndResource ('hfsA', x);
		SetResLoad (true);
		GetResInfo (hfsA, &resourceID, NULL, resourceName);
		resourceName += ".kext";
		ReleaseResource (hfsA);

		Handle plist = GetResource ('plst', resourceID);
		char *key = strstr (*plist, "<key>CFBundleVersion</key>");
		if (key) {
			char *start = strstr (key, "<string>");
			if (start) {
				start += strlen ("<string>");
				char *end = strstr (key, "</string>");
				if (end) {
					*end = 0;
					strcpy (resourceVersion, start);
				}
			}
		}
		ReleaseResource (plist);
		
		// Get the file version
		
		FSRef extension, contents, infoplist;
		err = XPFFSRef::getFSRef (&extensionsDir, (CChar255_AC) resourceName, &extension);
		if (err == noErr) err = XPFFSRef::getFSRef (&extension, "Contents", &contents);
		if (err == noErr) err = XPFFSRef::getFSRef (&contents, "Info.plist", &infoplist);
		if (err != noErr) {
			// The file is not there. If it's not OWCCacheConfig, we just return false.
			if (resourceName != "OWCCacheConfig.kext") {
				return false;
			} else {
				if (useCacheConfig) {
					return false;
				} else {
					continue; // we don't need to check version numbers :-)
				}
			}
		} else {
			// The file is there. So just keep going, unless it's OWCCacheConfig and it's not
			// supposed to be there.
			if ((resourceName == "OWCCacheConfig.kext") && !useCacheConfig) return false;
		}
		
		CFSSpec_AC infospec;
		err = FSGetCatalogInfo (&infoplist, kFSCatInfoNone, NULL, NULL, &infospec, NULL);
		if (err != noErr) return false;
		CFile_AC versionFile;
		versionFile.Specify (infospec);
		err = versionFile.OpenDataFork (); if (err != noErr) return false;
		long dataSize;
		err = versionFile.GetDataLength (dataSize); if (err != noErr) return false;
		Ptr versionData = NewPtr (dataSize + 1);
		versionFile.ReadData (versionData, dataSize);
		versionData[dataSize] = 0;
		key = strstr (versionData, "<key>CFBundleVersion</key>");
		if (key) {
			char *start = strstr (key, "<string>");
			if (start) {
				start += strlen ("<string>");
				char *end = strstr (key, "</string>");
				if (end) {
					*end = 0;
					strcpy (fileVersion, start);
				}
			}
		}
		DisposePtr (versionData);	
		versionFile.CloseDataFork ();
		
		// Compare the versions
		UInt32 fileVersionInt, resourceVersionInt;
		VERS_parse_string (fileVersion, &fileVersionInt);
		VERS_parse_string (resourceVersion, &resourceVersionInt);
		
		if (resourceVersionInt > fileVersionInt) return false;
	}

	return true;
}

bool 
MountedVolume::hasCurrentStartupItems ()
{
	FSRef startupDir;
	OSErr err = XPFFSRef::getOrCreateStartupDirectory (&fRootDirectory, &startupDir, false);
	if (err != noErr) return false;

	SInt16 archiveCount = CountResources ('hfsS');
	for (SInt16 x = 1; x <= archiveCount; x++) {
		SInt16 resourceID;
		CStr255_AC resourceName;
		char resourceVersion[128], fileVersion[128];
		resourceVersion[0] = 0;
		fileVersion[0] = 0;
		
		// Get the resource version

		SetResLoad (false);
		Handle hfsA = GetIndResource ('hfsS', x);
		SetResLoad (true);
		GetResInfo (hfsA, &resourceID, NULL, resourceName);
		ReleaseResource (hfsA);

		Handle plist = GetResource ('plst', resourceID);
		char *key = strstr (*plist, "<key>CFBundleVersion</key>");
		if (key) {
			char *start = strstr (key, "<string>");
			if (start) {
				start += strlen ("<string>");
				char *end = strstr (key, "</string>");
				if (end) {
					*end = 0;
					strcpy (resourceVersion, start);
				}
			}
		}
		ReleaseResource (plist);
		
		// Get the file version
		
		FSRef startup, contents, infoplist;
		err = XPFFSRef::getFSRef (&startupDir, (CChar255_AC) resourceName, &startup);
		if (err == noErr) err = XPFFSRef::getFSRef (&startup, "Contents", &contents);
		if (err == noErr) err = XPFFSRef::getFSRef (&contents, "Info.plist", &infoplist);
		if (err != noErr) return false;
		
		CFSSpec_AC infospec;
		err = FSGetCatalogInfo (&infoplist, kFSCatInfoNone, NULL, NULL, &infospec, NULL);
		if (err != noErr) return false;
		CFile_AC versionFile;
		versionFile.Specify (infospec);
		err = versionFile.OpenDataFork (); if (err != noErr) return false;
		long dataSize;
		err = versionFile.GetDataLength (dataSize); if (err != noErr) return false;
		Ptr versionData = NewPtr (dataSize + 1);
		versionFile.ReadData (versionData, dataSize);
		versionData[dataSize] = 0;
		key = strstr (versionData, "<key>CFBundleVersion</key>");
		if (key) {
			char *start = strstr (key, "<string>");
			if (start) {
				start += strlen ("<string>");
				char *end = strstr (key, "</string>");
				if (end) {
					*end = 0;
					strcpy (fileVersion, start);
				}
			}
		}
		DisposePtr (versionData);	
		versionFile.CloseDataFork ();
		
		// Compare the versions
		UInt32 fileVersionInt, resourceVersionInt;
		VERS_parse_string (fileVersion, &fileVersionInt);
		VERS_parse_string (resourceVersion, &resourceVersionInt);
		
		if (resourceVersionInt > fileVersionInt) return false;
	}

	return true;
}

void
MountedVolume::turnOffIgnorePermissions ()
{
#ifdef __MACH__
	char path[256];
	ThrowIfOSErr_AC (FSRefMakePath (getRootDirectory (), (UInt8 *) path, 255));
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
#endif
}

MountedVolume::~MountedVolume ()
{
	RemoveAllDependencies ();
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
	char mountPoint[256], deviceBSDName[32], *shortBSDName;
	OSStatus err = FSRefMakePath (getRootDirectory (), (UInt8 *) mountPoint, 255);
	deviceBSDName[0] = 0;
	if (err != noErr) return NULL;
	int numFS = getfsstat (NULL, 0, MNT_NOWAIT);
	struct statfs *fs = (struct statfs *) malloc (numFS * sizeof (struct statfs));
	getfsstat (fs, numFS * sizeof (struct statfs), MNT_NOWAIT);
	int x;
	for (x = 0; x < numFS; x++) {
		if (!strcmp (fs[x].f_mntonname, mountPoint)) {
			strcpy (deviceBSDName, fs[x].f_mntfromname);
			err = 0;
			break;
		}
	}
	free (fs);
				
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

MountedVolume::MountedVolume (FSVolumeInfo *info, HFSUniStr255 *name, FSRef *rootDirectory)
{
	fValidOpenFirmwareName = false;
	fIsOnBootableDevice = false;
	fPartInfo = NULL;
	fBootableDevice = NULL;
	fHelperDisk = NULL;
	fCreationDate = 0;
	fMacOSXVersion = "";
	
#ifdef BUILDING_XPF
	gApplication->AddDependent (this); // for listening for volume deletions
#endif

	// Copy the info
	{				
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
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:packages:Essentials.pkg", &installer);
		if (err != noErr) err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Installation:RPMS:", &installer);
		fHasInstaller = (err == noErr);
	}
	
	// See what version of Mac OS X is installed, if any
	{
		CFSSpec_AC versionSpec;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Library:CoreServices:SystemVersion.plist", &versionSpec);
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
					if (key) {
						char *start = strstr (key, "<string>");
						if (start) {
							start += strlen ("<string>");
							char *end = strstr (key, "</string>");
							if (end) {
								*end = 0;
								fMacOSXVersion.CopyFrom (start);
							}
						}
					}
				}
				DisposePtr (versionData);	
			}
			versionFile.CloseDataFork ();
		}
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
		
		err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:Library:StartupItems:XPFStartupItem", &supportSpec);
		fHasStartupItemInstalled = (err == noErr);
	}
	
	// See if it has a Finder (i.e. whether it might be a Darwin disk)
	{
		CFSSpec_AC finderSpec;
		OSErr err = FSMakeFSSpec (info->driveNumber, fsRtDirID, "\p:System:Library:CoreServices:Finder.app:", &finderSpec);
		fHasFinder = (err == noErr);
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
	// We do this a little differently in Mac OS X

	fValidOpenFirmwareName = false;
	
#ifdef __MACH__
	io_object_t regEntry = getRegEntry ();
	if (regEntry) {
		fBootableDevice = XPFBootableDevice::DeviceForRegEntry (regEntry);
		if (fBootableDevice) {
			fIsOnBootableDevice = true;
			fPartInfo = fBootableDevice->partitionWithInfoAndName (info, name);
			if (fPartInfo) {
				fPartInfo->setMountedVolume (this);
				
				char alias[256], shortAlias[256];
				OFAliases::AliasFor (regEntry, alias, shortAlias);
				fOpenFirmwareName.CopyFrom (alias);
				fShortOpenFirmwareName.CopyFrom (shortAlias);
				
				fValidOpenFirmwareName = (alias[0] != 0);
			}
		}
		IOObjectRelease (regEntry);
	}
#else
	fBootableDevice = XPFBootableDevice::DeviceWithInfo (info);
	if (fBootableDevice) {
		fIsOnBootableDevice = true;
		if (fBootableDevice->isFirewireDevice ()) {
			partInfoRec partInfo;
			OSErr err = MoreGetPartitionInfo (info->driveNumber, &partInfo);
			if (err == noErr) {
				fValidOpenFirmwareName = fBootableDevice->getValidOpenFirmwareName ();
				fOpenFirmwareName.CopyFrom (fBootableDevice->getOpenFirmwareName (false));
				fShortOpenFirmwareName.CopyFrom (fBootableDevice->getOpenFirmwareName (true));
				char buffer[16];
				sprintf (buffer, ":%X", partInfo.partitionNumber);
				fOpenFirmwareName += buffer;
				fShortOpenFirmwareName += buffer;
			}
		} else {
			fPartInfo = fBootableDevice->partitionWithInfoAndName (info, name);
			if (fPartInfo) {
				fValidOpenFirmwareName = fPartInfo->getValidOpenFirmwareName ();
				fOpenFirmwareName.CopyFrom (fPartInfo->getOpenFirmwareName (false));
				fShortOpenFirmwareName.CopyFrom (fPartInfo->getOpenFirmwareName (true));
				fPartInfo->setMountedVolume (this);
			}
		}
	}
#endif

	if (getRequiresBootHelper ()) fHelperDisk = getDefaultHelperDisk ();
	
	#if qLogging
		if (fValidOpenFirmwareName) {
			gLogFile << "OpenFirmwareName: ";
			CStr255_AC openFirmwareName = getOpenFirmwareName (false);
			gLogFile.WriteCharBytes ((char *) &openFirmwareName[1], openFirmwareName[0]);
			gLogFile << endl_AC;
			gLogFile << "ShortOpenFirmwareName: ";
			CStr255_AC shortOpenFirmwareName = getOpenFirmwareName (true);
			gLogFile.WriteCharBytes ((char *) &shortOpenFirmwareName[1], shortOpenFirmwareName[0]);
			gLogFile << endl_AC;
		} else {
			gLogFile << "Could not find Open Firmware name." << endl_AC;
		}
	#endif	
}

bool
MountedVolume::getRequiresBootHelper () 
{
	if (fIsOnBootableDevice) {
		return (fBootableDevice->getNeedsHelper () || !getIsWriteable ());
	}
	return false;
}

unsigned
MountedVolume::getHelperStatus ()
{
	unsigned installTargetStatus = getInstallTargetStatus ();
	if (installTargetStatus != kStatusOK) return installTargetStatus;
	if (fBootableDevice->getNeedsHelper ()) return kNeedsHelper;
	return kStatusOK;
}

unsigned
MountedVolume::getBootStatus ()
{
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getHasMachKernel ()) return kNoMachKernel;
	if (!getIsOnBootableDevice ()) return kNotBootable;
	if (!getValidOpenFirmwareName ()) return kNoOFName;
	if (!getIsWriteable() && getHasInstaller ()) return kInstallOnly;

	return kStatusOK;
}

unsigned
MountedVolume::getInstallTargetStatus ()
{
	if (!getIsHFSPlus ()) return kNotHFSPlus;
	if (!getIsOnBootableDevice ()) return kNotBootable;
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
	if (!getIsOnBootableDevice ()) return kNotBootable;
	if (!getValidOpenFirmwareName ()) return kNoOFName;
	
	return kStatusOK;
}

void
MountedVolume::setHelperDisk (MountedVolume *disk)
{
	if (fHelperDisk != disk) {
		fHelperDisk = disk;
		Changed (cSetHelperDisk, this);
	}
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
	}
}
