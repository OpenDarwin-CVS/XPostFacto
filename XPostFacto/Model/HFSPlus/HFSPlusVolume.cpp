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

#include "HFSPlusVolume.h"
#include "XPFPartition.h"
#include "HFSPlusCatalog.h"
#include "MountedVolume.h"
#include "XPFErrors.h"
#include "XPFLog.h"
#include "XCOFFDecoder.h"
#include "MoreFilesExtras.h"
#include "HFSPlusExtentsOverflow.h"
#include "FastUnicodeCompare.h"
#include "XPFAuthorization.h"

#ifdef BUILDING_XPF
	#include "XPFApplication.h"
#endif

#ifdef __MACH__
	#include <unistd.h>
	#include <sys/attr.h> 
#endif

HFSPlusVolume::HFSPlusVolume (XPFPartition *thePartition, unsigned long offsetToData)
{
	fBootXInstallationComplete = false;
	fPartition = thePartition;
	fOffsetIntoPartition = offsetToData;
	fHeader = NULL;
	ThrowIfOSErr_AC (readBlocks (2, 1, (void **) &fHeader));
	Erase_AC (&fVolumeName);
	HFSPlusCatalog catalog (this);
	catalog.findVolumeName (&fVolumeName);

//	HFSPlusExtentsOverflow extentsOverflow (this);

}

HFSPlusVolume::~HFSPlusVolume ()
{
	if (fHeader) DisposePtr ((Ptr) fHeader);
}

bool 
HFSPlusVolume::matchInfo (FSVolumeInfo *info)
{
	// Because of time zone weirdness, we need to compare creation dates not for equality as such,
	// but for possible equality based on time zone differences. There is probably a deterministic
	// way to do this, but this approach ought to be good enough.
	unsigned difference;
	if (info->createDate.lowSeconds > fHeader->createDate) {
		difference = info->createDate.lowSeconds - fHeader->createDate;
	} else {
		difference = fHeader->createDate - info->createDate.lowSeconds;
	}
	if (difference > (24 * 60 * 60)) return false; // more than 24 hour difference
	if (difference % (30 * 60) != 0) return false; // not an even 30 minute difference 
	return ((info->blockSize == fHeader->blockSize) && (info->totalBlocks == fHeader->totalBlocks));
}

bool 
HFSPlusVolume::matchInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name)
{
	if (!matchInfo (info)) return false;
	return (FastUnicodeCompare (	name->unicode, name->length, 
									fVolumeName.unicode, fVolumeName.length) == 0);
}

OSErr
HFSPlusVolume::readBlocks (unsigned int start, unsigned int count, void **buffer)
{
	return fPartition->readBlocks (start + fOffsetIntoPartition, count, buffer);
}

OSErr 
HFSPlusVolume::readAllocationBlocks (unsigned int start, unsigned int count, void **buffer)
{
	int sectorsInAllocationBlock = fHeader->blockSize / 512;
	return readBlocks (start * sectorsInAllocationBlock, count * sectorsInAllocationBlock, buffer);
}

unsigned long 
HFSPlusVolume::getBootXStartBlock ()
{
#ifdef __MACH__
	struct {
  	  	u_int32_t length;
 	   	HFSPlusExtentRecord extents;
	} attrBuffer;
	
	attrlist params;
	
	char path[256];
	MountedVolume *vol = getMountedVolume ();
	if (!vol) return 0;
	FSRefMakePath (vol->getRootDirectory (), (UInt8 *) path, 255);
	if (path[strlen (path) - 1] != '/') strcat (path, "/");
	strcat (path, "BootX.image");

	params.bitmapcount = 5;
    params.reserved = 0;
    params.commonattr = 0;
    params.volattr =  0;
    params.dirattr = 0;
    params.fileattr = ATTR_FILE_DATAEXTENTS;
    params.forkattr = 0;
    
    XPFSetUID (0);
    int err = getattrlist (path, &params, &attrBuffer, sizeof (attrBuffer), 1); 
    if (err) return 0;
	if (attrBuffer.extents[1].blockCount == 0) {
		return fOffsetIntoPartition + attrBuffer.extents[0].startBlock * (fHeader->blockSize / 512);
	} else {
		return 0;
	}
#else
	HFSPlusCatalog catalog (this);
	HFSUniStr255 bootXName = {11, {'B', 'o', 'o', 't', 'X', '.', 'i', 'm', 'a', 'g', 'e'}};
	HFSPlusCatalogFile file; 
	OSErr err = catalog.findEntry (fsRtDirID, bootXName, &file);
	if (err != noErr) {
		#if qLogging
			gLogFile << "Could not find BootX in catalog" << endl_AC;
		#endif
		ThrowException_AC (kErrorFindingBootXInCatalog, 0);
	}
	#if qLogging
		gLogFile << "BootX extents" << endl_AC;
		for (int x = 0; x < 8; x++) {
			if (file.dataFork.extents[x].startBlock == 0) break;
			gLogFile << "startBlock: " << file.dataFork.extents[x].startBlock << endl_AC;
			gLogFile << "blockCount: " << file.dataFork.extents[x].blockCount << endl_AC;
		}
	#endif
	if (file.dataFork.extents[0].blockCount == file.dataFork.totalBlocks) {
		return fOffsetIntoPartition + file.dataFork.extents[0].startBlock * (fHeader->blockSize / 512);
	} else {
		return 0;
	}
#endif
}

void
HFSPlusVolume::installBootX ()
{
#ifdef BUILDING_XPF
	if (((XPFApplication *) gApplication)->getDebugOptions () & kDisableBootX) return;
#endif

	MountedVolume* volume = getMountedVolume ();
	if (!volume) {
		#if qLogging
			gLogFile << "Could not find mounted volume for partition" << endl_AC;
		#endif
		return;
	}
		
#ifdef __MACH__
	// Check to make sure we've got partition info to work with
	io_object_t partInfo = volume->getPartitionInfo ();
	if (partInfo) {
		IOObjectRelease (partInfo);
	} else {
		ThrowException_AC (kWritePartitionOSX, 0);
	}
#endif

	XPFSetUID myUID (0);
	
	FSRef bootXRef;
	FSSpec bootXSpec;
	try {
		UniChar bootXName[] = {'B', 'o', 'o', 't', 'X', '.', 'i', 'm', 'a', 'g', 'e'};
		OSErr err = FSCreateFileUnicode (volume->getRootDirectory (), 
				sizeof (bootXName) / sizeof (UniChar), bootXName, kFSCatInfoNone, NULL, &bootXRef, NULL);
		if (err == dupFNErr) {
			err = noErr;
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (volume->getRootDirectory (), 
					sizeof (bootXName) / sizeof (UniChar), bootXName, kTextEncodingUnknown, &bootXRef));
			ThrowIfOSErr_AC (FSDeleteObject (&bootXRef));
			ThrowIfOSErr_AC (FSCreateFileUnicode (volume->getRootDirectory (), 
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
#else
	ThrowIfOSErr_AC (FlushVol (NULL, volume->getIOVDrvInfo ()));
#endif

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
 	
	fPartition->setLgBootStart (getBootXStartBlock ());
	if (fPartition->getLgBootStart () == 0) {
		#if qLogging
			gLogFile << "Error finding start block for BootX" << endl_AC;
		#endif
		ThrowException_AC (kErrorExtractingBootX, 0);
	} else {
		fPartition->writePartition ();
	}
}

void
HFSPlusVolume::installBootXIfNecessary (bool forceInstall)
{
	MountedVolume* volume = getMountedVolume ();
	if (!volume) return;
	if (fBootXInstallationComplete) return;
	
	ThrowIfNULL_AC (fHeader);
	
	if (forceInstall) {
		#if qLogging
			gLogFile << "Forcing BootX to install" << endl_AC;
		#endif
		installBootX ();
		fBootXInstallationComplete = true;
		return;
	}
	
	FSSpec bootXImageSpec;
	OSErr err;
	err = FSMakeFSSpec (volume->getIOVDrvInfo(), fsRtDirID, "\p:BootX.image", &bootXImageSpec);
	if (err != noErr) {
		#if qLogging
			gLogFile << "BootX.image not found." << endl_AC;
		#endif
		installBootX ();
	} else {
		// We have a BootX.file. So, we'll see whether it has moved.
		unsigned long firstBootXBlock = getBootXStartBlock ();
		if ((firstBootXBlock == 0) || (firstBootXBlock != fPartition->getLgBootStart ())) {
			#if qLogging
				gLogFile << "BootX.image moved. Reinstalling" << endl_AC;
			#endif
			installBootX ();
		} else {
			// it hasn't moved. So we'll see whether we need to install a new version.
			VersRecHndl installedVersion = NULL;
			SInt16 resourceFork = 0;
			try {	
				resourceFork = FSpOpenResFile (&bootXImageSpec, fsRdPerm);
				ThrowIfResError_AC ();
				installedVersion = (VersRecHndl) Get1Resource ('vers', 1);
				ThrowIfNULL_AC (installedVersion);
				ThrowIfResError_AC ();
				DetachResource ((Handle) installedVersion);
			}
			catch (...) {
				// There was a problem accessing the version info. So we'll just reinstall
				#if qLogging
					gLogFile << "Could not access BootX version info. Reinstalling" << endl_AC;
				#endif
				if (resourceFork > 0) CloseResFile (resourceFork);
				installBootX ();	
				fBootXInstallationComplete = true;
				return;		
			}
			if (resourceFork > 0) CloseResFile (resourceFork);

			VersRecHndl ourVersion = (VersRecHndl) GetResource ('vers', 3);
			ThrowIfNULL_AC (ourVersion);
			ThrowIfResError_AC ();
			DetachResource ((Handle) ourVersion);
			
			if (CompareVersions (&(*ourVersion)->numericVersion, &(*installedVersion)->numericVersion) == 1) {
				#if qLogging
					gLogFile << "Our BootX version is newer. Reinstalling." << endl_AC;
				#endif
				installBootX ();
			}
			
			DisposeHandle ((Handle) ourVersion);
			DisposeHandle ((Handle) installedVersion);
		}
	}			
	fBootXInstallationComplete = true;
}

SInt16 
HFSPlusVolume::CompareVersions (NumVersion *vers1, NumVersion *vers2)
{
    UInt16 nonRelRev1, nonRelRev2;
         
    if (vers1->majorRev        > vers2->majorRev)        return  1;
    if (vers1->majorRev        < vers2->majorRev)        return -1;
    if (vers1->minorAndBugRev  > vers2->minorAndBugRev)  return  1;
    if (vers1->minorAndBugRev  < vers2->minorAndBugRev)  return -1;
    if (vers1->stage           > vers2->stage)           return  1;
    if (vers1->stage           < vers2->stage)           return -1;
         
    nonRelRev1 = vers1->nonRelRev;
    nonRelRev2 = vers2->nonRelRev;
         
    if (vers1->stage == finalStage) {
        if (vers1->nonRelRev == 0)             nonRelRev1 = 0xFFFF;
        if (vers2->nonRelRev == 0)             nonRelRev2 = 0xFFFF;
    }
         
    if (nonRelRev1 > nonRelRev2)                         return  1;
    if (nonRelRev1 < nonRelRev2)                         return -1;
         
    return 0;
}
         
