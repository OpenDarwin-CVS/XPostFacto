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
#include "HFSPlusCatalog.h"
#include "HFSPlusCatalogNode.h"
#include "HFSPlusExtentsOverflow.h"
#include "XPFErrors.h"
#include "XPFLog.h"

HFSPlusCatalog::HFSPlusCatalog (HFSPlusVolume *volume)
{
	fVolume = volume;
	HFSPlusVolumeHeader *header = fVolume->getHeader();
	
	// collect the extents
	HFSPlusForkData *catalogFile = &header->catalogFile;
	UInt32 totalBlocksSeen = 0;
	for (int x = 0; x < 8; x++) {
		if (catalogFile->extents[x].startBlock == 0) break;
		totalBlocksSeen += catalogFile->extents[x].blockCount;
		fExtents.InsertElementInOrder (&catalogFile->extents[x]);
	}
	if (totalBlocksSeen < catalogFile->totalBlocks) {
		#if qLogging
			gLogFile << "Catalog too fragmented. Will consult extents overflow" << endl_AC;
		#endif
//		ThrowException_AC (kCatalogFileTooFragmented, 0);
		HFSPlusExtentsOverflow extentsOverflow (fVolume);
		extentsOverflow.addExtentsFor (kHFSCatalogFileID, &fExtents, &totalBlocksSeen);
	} 
	
	if (totalBlocksSeen != catalogFile->totalBlocks) {
		#if qLogging
			gLogFile << "Catalog file inconsistent" << endl_AC;
		#endif
		ThrowException_AC (kCatalogFileInconsistent, 0);
	} 
	
	BTHeaderRec headerNode;
	Ptr buffer = NULL;
	ThrowIfOSErr_AC (readAllocationBlocks (0, 1, (void **) &buffer));
	ThrowIfNULL_AC (buffer);
	BlockMoveData (buffer + sizeof (BTNodeDescriptor), &headerNode, sizeof (headerNode));
	DisposePtr (buffer);

	fNodeSize = headerNode.nodeSize;		
	fFirstLeafNode = headerNode.firstLeafNode;
}

OSErr 
HFSPlusCatalog::readAllocationBlocks (unsigned int start, unsigned int count, void **buffer)
{
	for (int x = 0; x < fExtents.GetSize (); x++) {
		if (start < fExtents[x].blockCount) {
			if (start + count > fExtents[x].blockCount) {
				#if qLogging
					gLogFile << "Read crosses extents" << endl_AC;
				#endif
				ThrowException_AC (kReadCrossesExtents, 0);
			}
			return fVolume->readAllocationBlocks (fExtents[x].startBlock + start, count, buffer);
			break;
		} else {
			start -= fExtents[x].blockCount;
		}
	}
	return paramErr;
}

OSErr 
HFSPlusCatalog::readBytes (long long start, long long count, void **buffer)
{
	UInt32 blockSize = fVolume->getBlockSize ();
	if ((start % blockSize != 0) || (count % blockSize != 0)) {
		#if qLogging
			gLogFile << "Unaligned allocation block read" << endl_AC;
		#endif
		ThrowException_AC (kUnalignedAllocationBlockRead, 0);
	}
	start /= blockSize;
	count /= blockSize;
	return readAllocationBlocks (start, count, buffer);
}

int 
HFSPlusCatalog::findEntry (HFSCatalogNodeID parentID, const HFSUniStr255& nodeName, HFSPlusCatalogFile *file)
{
	HFSPlusCatalogNode *node = new HFSPlusCatalogNode (this, fFirstLeafNode);
	int result = 0;
	while (result == 0) {
		result = node->findEntry (parentID, nodeName, file);
		if (result == 0) {
			// not found yet, but keep looking
			HFSPlusCatalogNode *nextNode = node->newNextNode ();
			delete node;
			node = nextNode;
			if (node == NULL) result = -1;
		}		
	}
	if (node) delete node;
	if (result == 1) {
		return noErr;
	} else {
		return paramErr;
	}
}

int
HFSPlusCatalog::findVolumeNameAndCreateDate (HFSUniStr255 *outName, UInt32 *outDate)
{
	HFSPlusCatalogNode *node = new HFSPlusCatalogNode (this, fFirstLeafNode);
	int result = 0;
	while (result == 0) {
		result = node->findVolumeNameAndCreateDate (outName, outDate);
		if (result == 0) {
			// not found yet, but keep looking
			HFSPlusCatalogNode *nextNode = node->newNextNode ();
			delete node;
			node = nextNode;
			if (node == NULL) result = -1;
		}		
	}
	if (node) delete node;
	if (result == 1) {
		return noErr;
	} else {
		return paramErr;
	}
}