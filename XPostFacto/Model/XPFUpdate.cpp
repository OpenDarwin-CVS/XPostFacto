/*

Copyright (c) 2003
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

#include "XPFUpdate.h"
#include "MountedVolume.h"
#include "XPostFacto.h"
#include "XPFFSRef.h"
#include "vers_rsrc.h"
#include "XPFApplication.h"
#include "XPFPrefs.h"
#include "XPFAuthorization.h"
#include "FSRefCopying.h"
#include "HFSPlusArchive.h"
#include "XPFProgressWindow.h"
#include <stdio.h>

#ifdef __MACH__
	#include <sys/types.h>
	#include <sys/stat.h>
#endif


XPFUpdate::XPFUpdate (MountedVolume *target, MountedVolume *helper, MountedVolume *installCD)
{
	SInt16 archiveCount;

	fTarget = target;
	fHelper = helper;
	fInstallCD = installCD;
	
	if (fTarget) fTarget->turnOffIgnorePermissions ();
	if (fHelper) fHelper->turnOffIgnorePermissions ();
	if (fInstallCD) fInstallCD->turnOffIgnorePermissions ();
	
	// Construct the list
	
	fItemList.InsertLast (new XPFBootXUpdate (this));
	
	archiveCount = CountResources ('hfsA');
	for (SInt16 x = 1; x <= archiveCount; x++) {
		fItemList.InsertLast (new XPFExtensionsUpdate (this, x));
	}

	archiveCount = CountResources ('hfsS');
	for (SInt16 x = 1; x <= archiveCount; x++) {
		fItemList.InsertLast (new XPFStartupItemUpdate (this, x));
	}
}

OSErr
XPFUpdate::getOrCreateExtensionsDirectory (FSRef *directory, bool create)
{
	OSErr err = fnfErr;
	MountedVolume *bootDisk, *rootDisk;
	
	rootDisk = fInstallCD ? fInstallCD : fTarget;
	bootDisk = fHelper ? fHelper : fTarget;
	
	if (rootDisk->getIsWriteable ()) {
		err = XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (rootDisk->getRootDirectory (), directory, create);
	} else if (bootDisk) {
		FSRef helperDir;
		err = XPFFSRef::getOrCreateHelperDirectory (
			bootDisk->getRootDirectory (), 
			(CChar255_AC) rootDisk->getOpenFirmwareName (true),
			&helperDir,
			create
		);			
		if (err == noErr) err = XPFFSRef::getOrCreateLibraryExtensionsDirectory (&helperDir, directory, create);
	}
	return err;
}

XPFUpdateItem *
XPFUpdate::getItemWithResourceID (ResNumber resourceID)
{
	for (XPFUpdateListIterator iter (&fItemList); iter.Current (); iter.Next ()) {
		if (iter->getResourceID () == resourceID) return iter.Current ();
	}
	return NULL;
}

XPFUpdateItem *
XPFUpdate::getItemWithResourceName (CStr255_AC name)
{
	for (XPFUpdateListIterator iter (&fItemList); iter.Current (); iter.Next ()) {
		if (iter->getResourceName () == name) return iter.Current ();
	}
	return NULL;
}

bool
XPFUpdate::getRequiresAction ()
{
	for (XPFUpdateListIterator iter (&fItemList); iter.Current (); iter.Next ()) {
		if (iter->getAction () != kActionNone) return true;
	}
	return false;
}

bool
XPFUpdate::getRequiresSynchronization ()
{
	if (!fTarget) return false;
	if (!fHelper) return false;
	
	FSRef helperDir;
	OSErr err = XPFFSRef::getOrCreateHelperDirectory (
		fHelper->getRootDirectory (),
		(CChar255_AC) fTarget->getOpenFirmwareName (true),
		&helperDir,
		false
	);
	if (err != noErr) return true;
	
	FSRef rootFSRef, bootFSRef;

	err = XPFFSRef::getKernelFSRef (fTarget->getRootDirectory (), &rootFSRef);
	if (err == noErr) err = XPFFSRef::getKernelFSRef (&helperDir, &bootFSRef);
	if (err != noErr) return true;
	if (!XPFFSRef::isCatalogInfoTheSame (&rootFSRef, &bootFSRef)) return true;
	
	OSErr rootErr = XPFFSRef::getExtensionsCacheFSRef (fTarget->getRootDirectory (), &rootFSRef, false);
	OSErr bootErr = XPFFSRef::getExtensionsCacheFSRef (&helperDir, &bootFSRef, false);
	if (rootErr == fnfErr) {
		if (bootErr != fnfErr) return true;
	} else if (rootErr != noErr) {
		return true;
	} else if (bootErr != noErr) {
		return true;
	}
	if (!XPFFSRef::isCatalogInfoTheSame (&rootFSRef, &bootFSRef)) return true;
	
	err = XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (fTarget->getRootDirectory (), &rootFSRef, false);
	if (err != noErr) return true;
	err = XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (&helperDir, &bootFSRef, false);
	if (err != noErr) return true;
	if (!XPFFSRef::isCatalogInfoTheSame (&rootFSRef, &bootFSRef)) return true;
	
	return false;
}

XPFUpdateItem::XPFUpdateItem (XPFUpdate *update)
{
	fUpdate = update;
	
	fInstalledVersion = 0;
	fAvailableVersion = 0;
	
	fMinBuild = 0;
	fMaxBuild = 0;
	
	fResourceID = 0;
}

void
XPFUpdateItem::parsePListString (char *plist, char *search, char *result)
{
	result[0] = 0;
	char fullSearch [128];
	sprintf (fullSearch, "<key>%s</key>", search);
	char *key = strstr (plist, fullSearch);
	if (key) {
		char *start = strstr (key, "<string>");
		if (start) {
			start += strlen ("<string>");
			char *end = strstr (key, "</string>");
			if (end) {
				char s = *end;
				*end = 0;
				strcpy (result, start);
				*end = s;
			}
		}
	}
}

void
XPFUpdateItem::getResourceInfo (ResType resType, SInt16 resIndex)
{
	SetResLoad (false);
	Handle hfsA = GetIndResource (resType, resIndex);
	SetResLoad (true);
	GetResInfo (hfsA, &fResourceID, NULL, fResourceName);
	ReleaseResource (hfsA);
		
	Handle plist = GetResource ('plst', fResourceID);
	(*plist)[GetHandleSize(plist) - 1] = 0;
	char minBuild[32], maxBuild[32], availableVersion[32];
	parsePListString (*plist, "CFBundleVersion", availableVersion);
	parsePListString (*plist, "OWCInstallMinBuild", minBuild);
	parsePListString (*plist, "OWCInstallMaxBuild", maxBuild);
	if (minBuild [0]) fMinBuild = strtoul (minBuild, NULL, 0);
	if (maxBuild [0]) fMaxBuild = strtoul (maxBuild, NULL, 0);
	VERS_parse_string (availableVersion, &fAvailableVersion);		
	ReleaseResource (plist);
}

void
XPFUpdateItem::getFileInfo (FSRef *file)
{		
	CFSSpec_AC infospec;
	OSErr err = FSGetCatalogInfo (file, kFSCatInfoNone, NULL, NULL, &infospec, NULL);
	if (err != noErr) return;
	CFile_AC versionFile;
	versionFile.Specify (infospec);
	err = versionFile.OpenDataFork (); if (err != noErr) return;
	long dataSize;
	err = versionFile.GetDataLength (dataSize); if (err != noErr) return;
	Ptr versionData = NewPtr (dataSize + 1);
	versionFile.ReadData (versionData, dataSize);
	versionData[dataSize] = 0;
	char installedVersion [32];
	parsePListString (versionData, "CFBundleVersion", installedVersion);
	VERS_parse_string (installedVersion, &fInstalledVersion);
	DisposePtr (versionData);	
	versionFile.CloseDataFork ();
}

bool
XPFUpdateItem::getIsQualified ()
{
	MountedVolume *rootDisk = fUpdate->getInstallCD ();
	if (rootDisk == NULL) rootDisk = fUpdate->getTarget ();
	UInt32 currentBuild = rootDisk->getMacOSXMajorVersion ();
	if (!currentBuild) return true;
	if (fMinBuild && (currentBuild < fMinBuild)) return false;
	if (fMaxBuild && (currentBuild > fMaxBuild)) return false;
	
	// This isn't quite how I should handle this, but it will work for now
	if (fResourceName == "OWCCacheConfig.kext") {
		return ((XPFApplication *) gApplication)->getPrefs()->getEnableCacheEarly ();
	} else if (fResourceName == "PatchedRagePro.kext") {
		return ((XPFApplication *) gApplication)->getPrefs()->getUsePatchedRagePro ();
	}
	
	return true;
}
			
XPFUpdateAction
XPFUpdateItem::getAction ()
{
	if (!getIsQualified ()) return fInstalledVersion ? kActionDelete : kActionNone;
	if (VERS_compare (fAvailableVersion, fInstalledVersion) == 1) return kActionInstall;	
	return kActionNone;
}

void
XPFUpdateItem::setEnabled (bool enabled)
{
	if (fEnabled != enabled) {
		fEnabled = enabled;
		Changed (cSetEnabled, this);
	}
}

XPFBootXUpdate::XPFBootXUpdate (XPFUpdate *update)
	: XPFUpdateItem (update)
{
	MountedVolume *bootDisk = fUpdate->getHelper ();
	if (!bootDisk) bootDisk = fUpdate->getTarget ();
	fInstalledVersion = bootDisk->getBootXVersion ();
	
	VersRecHndl ourVersion = (VersRecHndl) GetResource ('vers', 3);
	ThrowIfNULL_AC (ourVersion);
	ThrowIfResError_AC ();
			
	memcpy (&fAvailableVersion, &(*ourVersion)->numericVersion, sizeof (fAvailableVersion));
	
	fResourceName.CopyFrom ("BootX");
				
	ReleaseResource ((Handle) ourVersion);
}

void
XPFBootXUpdate::doUpdate ()
{
	MountedVolume *bootDisk = fUpdate->getHelper ();
	if (!bootDisk) bootDisk = fUpdate->getTarget ();
	bootDisk->installBootXIfNecessary (true);	
}

void
XPFBootXUpdate::uninstall ()
{
	// Does nothing for the moment. I suppose I could try to reinstall the original BootX
	// from /System/Library/CoreServices, but it may or may not be feasible.
}

XPFExtensionsUpdate::XPFExtensionsUpdate (XPFUpdate *update, SInt16 resIndex)
	: XPFUpdateItem (update)
{		
	getResourceInfo ('hfsA', resIndex);
	fResourceName += ".kext";

	FSRef extensionsDir;
	OSErr err = update->getOrCreateExtensionsDirectory (&extensionsDir, false);
	
	if (err == noErr) {	
		FSRef extension, contents, infoplist;
		err = XPFFSRef::getFSRef (&extensionsDir, (CChar255_AC) fResourceName, &extension);
		if (err == noErr) err = XPFFSRef::getFSRef (&extension, "Contents", &contents);
		if (err == noErr) err = XPFFSRef::getFSRef (&contents, "Info.plist", &infoplist);
		if (err == noErr) getFileInfo (&infoplist);
	}	
}

void
XPFExtensionsUpdate::doUpdate ()
{
	FSRef extensionsDir;	
	ThrowIfOSErr_AC (fUpdate->getOrCreateExtensionsDirectory (&extensionsDir, true));
	
	XPFUpdateAction action = getAction ();
	if (action == kActionInstall) {
		CResourceStream_AC stream ('hfsA', fResourceID);
		HFSPlusArchive archive (&stream, NULL, NULL);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (&extensionsDir));
	} else if (action == kActionDelete) {
		uninstall ();
	}
}

void
XPFExtensionsUpdate::uninstall ()
{
	FSRef extensionsDir, existingDir;	
	ThrowIfOSErr_AC (fUpdate->getOrCreateExtensionsDirectory (&extensionsDir, true));
	OSErr err = XPFFSRef::getFSRef (&extensionsDir, (CChar255_AC) getResourceName (), &existingDir);
	if (err == noErr) {
		XPFSetUID myUID (0);
		FSRefDeleteDirectory (&existingDir);
	}	
}

XPFStartupItemUpdate::XPFStartupItemUpdate (XPFUpdate *update, SInt16 resIndex)
	: XPFUpdateItem (update)
{
	getResourceInfo ('hfsS', resIndex);

	FSRef *rootDirectory, startupDir;
	rootDirectory = update->getTarget()->getRootDirectory ();
	OSErr err = XPFFSRef::getOrCreateStartupDirectory (rootDirectory, &startupDir, false);
			
	if (err == noErr) {	
		FSRef startupItem, contents, infoplist;
		err = XPFFSRef::getFSRef (&startupDir, (CChar255_AC) fResourceName, &startupItem);
		if (err == noErr) err = XPFFSRef::getFSRef (&startupItem, "Contents", &contents);
		if (err == noErr) err = XPFFSRef::getFSRef (&contents, "Info.plist", &infoplist);
		if (err == noErr) getFileInfo (&infoplist);
	}	
}

XPFUpdateAction 
XPFStartupItemUpdate::getAction ()
{
	if (!fUpdate->getTarget()->getIsWriteable()) return kActionNone;
	return XPFUpdateItem::getAction ();
}

void
XPFStartupItemUpdate::moveToTmp ()
{
#ifdef __MACH__
	// On Mac OS X, if we're installing in the current root directory, then we need to move
	// the old startup item (if any) out of the way first, because otherwise we'll try to delete
	// a running binary, which doesn't work.
	char rootPath [1024];
	ThrowIfOSErr_AC (FSRefMakePath (fUpdate->getTarget()->getRootDirectory(), (UInt8 *) rootPath, 1023));
	if (!strcmp (rootPath, "/")) {
		struct stat sb;
		char dstPath [256];
		for (int x = 0; x < 100; x++) {
			sprintf (dstPath, "/tmp/XPFStartupItem.%d", x);
			if (stat (dstPath, &sb)) break; // we break on an error--i.e. no file!
		}
		XPFSetUID myUID (0);
		rename ("/Library/StartupItems/XPFStartupItem", dstPath);
		// we don't care about rename errors--for instance, if the src doesn't exist, that's fine
	}
#endif 
}

void
XPFStartupItemUpdate::doUpdate ()
{
	FSRef startupDir;
	ThrowIfOSErr_AC (XPFFSRef::getOrCreateStartupDirectory (fUpdate->getTarget()->getRootDirectory (), &startupDir, true));
	
	XPFUpdateAction action = getAction ();
	if (action == kActionInstall) {
		moveToTmp ();
		CResourceStream_AC stream ('hfsS', fResourceID);
		HFSPlusArchive archive (&stream, NULL, NULL);
		XPFSetUID myUID (0);
		ThrowIfOSErr_AC (archive.extractArchiveTo (&startupDir));
	} else if (action == kActionDelete) {
		uninstall ();
	}
}

void
XPFStartupItemUpdate::uninstall ()
{
	moveToTmp ();
	FSRef existingDir, startupDir;
	ThrowIfOSErr_AC (XPFFSRef::getOrCreateStartupDirectory (fUpdate->getTarget()->getRootDirectory (), &startupDir, true));
	OSErr err = XPFFSRef::getFSRef (&startupDir, (CChar255_AC) getResourceName (), &existingDir);
	if (err == noErr) {
		XPFSetUID myUID (0);
		FSRefDeleteDirectory (&existingDir);
	}	
}