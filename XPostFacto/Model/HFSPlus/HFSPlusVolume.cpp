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
#include "XPFBootableDevice.h"
#include "XPostFacto.h"

#ifdef BUILDING_XPF
	#include "XPFApplication.h"
#endif

#ifdef __MACH__
	#include <unistd.h>
	#include <sys/attr.h> 
#endif

HFSPlusVolume::HFSPlusVolume (XPFPartition *thePartition, unsigned long offsetToData)
{
	fPartition = thePartition;
	fOffsetIntoPartition = offsetToData;
	fHeader = NULL;
	
	gLogFile << "offsetToData: " << fOffsetIntoPartition << endl_AC;
	
	ThrowIfOSErr_AC (readBlocks (2, 1, (void **) &fHeader));
	
	gLogFile << "HFSPlusVolume fOffsetIntoPartition: " << fOffsetIntoPartition << " blockSize: " << fHeader->blockSize << endl_AC;
	
	Erase_AC (&fVolumeName);
	HFSPlusCatalog catalog (this);
	catalog.findVolumeNameAndCreateDate (&fVolumeName, &fHeader->createDate);
}

HFSPlusVolume::~HFSPlusVolume ()
{
	if (fHeader) DisposePtr ((Ptr) fHeader);
}

MountedVolume*
HFSPlusVolume::getMountedVolume ()
{
	return fPartition ? fPartition->getMountedVolume () : NULL;
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
HFSPlusVolume::readBlocks (UInt32 start, UInt32 count, void **buffer)
{
	gLogFile << "HFSPlusVolume::readBlocks fOffsetIntoPartition: " << fOffsetIntoPartition << " start: " << start << endl_AC;
	return fPartition->readBlocks (start + fOffsetIntoPartition, count, buffer);
}

OSErr 
HFSPlusVolume::readAllocationBlocks (UInt32 start, UInt32 count, void **buffer)
{
	UInt32 sectorsInAllocationBlock = fHeader->blockSize / 512;
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
	
	char path[1024];
	MountedVolume *vol = getMountedVolume ();
	if (!vol) return 0;
	FSRefMakePath (vol->getRootDirectory (), (UInt8 *) path, 1023);
	if (path[strlen (path) - 1] != '/') strcat (path, "/");
	strcat (path, "BootX.image");

	params.bitmapcount = 5;
    params.reserved = 0;
    params.commonattr = 0;
    params.volattr =  0;
    params.dirattr = 0;
    params.fileattr = ATTR_FILE_DATAEXTENTS;
    params.forkattr = 0;
    
    XPFSetUID myUID (0);
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

/*
	#if qLogging
		gLogFile << "BootX extents" << endl_AC;
		for (int x = 0; x < 8; x++) {
			if (file.dataFork.extents[x].startBlock == 0) break;
			gLogFile << "startBlock: " << file.dataFork.extents[x].startBlock << endl_AC;
			gLogFile << "blockCount: " << file.dataFork.extents[x].blockCount << endl_AC;
		}
	#endif
*/
	
	if (file.dataFork.extents[0].blockCount == file.dataFork.totalBlocks) {
		return fOffsetIntoPartition + file.dataFork.extents[0].startBlock * (fHeader->blockSize / 512);
	} else {
		return 0;
	}
#endif
}
