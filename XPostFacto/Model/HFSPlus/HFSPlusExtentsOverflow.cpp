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
#include "HFSPlusExtentsOverflow.h"
#include "HFSPlusExtentsOverflowNode.h"
#include "XPFErrors.h"
#include "XPFLog.h"

HFSPlusExtentsOverflow::HFSPlusExtentsOverflow (HFSPlusVolume *volume)
{
	fVolume = volume;
	HFSPlusVolumeHeader *header = fVolume->getHeader();
	
	// collect the extents
	HFSPlusForkData *extentsFile = &header->extentsFile;
	UInt32 totalBlocksSeen = 0;
	for (int x = 0; x < 8; x++) {
		if (extentsFile->extents[x].startBlock == 0) break;
		totalBlocksSeen += extentsFile->extents[x].blockCount;
		fExtents.InsertLast (extentsFile->extents[x]);
	}
	if (totalBlocksSeen < extentsFile->totalBlocks) {
		#if qLogging
			gLogFile << "Extents file too fragmented" << endl_AC;
		#endif
		ThrowException_AC (kExtentsFileTooFragmented, 0);
	} else if (totalBlocksSeen != extentsFile->totalBlocks) {
		#if qLogging
			gLogFile << "Extents file inconsistent" << endl_AC;
		#endif
		ThrowException_AC (kExtentsFileInconsistent, 0);
	} 
	
	BTHeaderRec headerNode;
	Ptr buffer = NULL;
	ThrowIfOSErr_AC (readAllocationBlocks (0, 1, (void **) &buffer));
	ThrowIfNULL_AC (buffer);
	BlockMoveData (buffer + sizeof (BTNodeDescriptor), &headerNode, sizeof (headerNode));
	DisposePtr (buffer);

	fNodeSize = headerNode.nodeSize;		
	fFirstLeafNode = headerNode.firstLeafNode;
	
/*	HFSPlusExtentsOverflowNode *node, *nextNode;
	node = new HFSPlusExtentsOverflowNode (this, fFirstLeafNode);
	ThrowIfNULL_AC (node);

	for (int x = 0; x < 10; x++) {
		nextNode = node->newNextNode ();
		delete node;
		node = nextNode;
		ThrowIfNULL_AC (node);
	}
*/
}

void 
HFSPlusExtentsOverflow::addExtentsFor (HFSCatalogNodeID nodeID, TemplateArray_AC<HFSPlusExtentDescriptor> *extentsArray, UInt32 *totalBlocksSeen)
{
	HFSPlusExtentsOverflowNode *node = new HFSPlusExtentsOverflowNode (this, fFirstLeafNode);
	HFSPlusExtentsOverflowNode *nextNode;
	OSErr err = noErr;
	while (err == noErr) {
		if (!node) break;
		err = node->addExtentsFor (nodeID, extentsArray, totalBlocksSeen);
		nextNode = node->newNextNode ();
		delete node;
		node = nextNode;
	}
	if (node) delete node;
}

OSErr 
HFSPlusExtentsOverflow::readAllocationBlocks (unsigned int start, unsigned int count, void **buffer)
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
HFSPlusExtentsOverflow::readBytes (long long start, long long count, void **buffer)
{
	UInt32 blockSize = fVolume->getBlockSize ();

	UInt32 extraBytesAtStart = start % blockSize;
	if (extraBytesAtStart) {
		start -= extraBytesAtStart;
		count += extraBytesAtStart;
	}
	
	UInt32 extraBytesAtEnd = count % blockSize;
	if (extraBytesAtEnd) count += blockSize - extraBytesAtEnd;
	
	if ((start % blockSize != 0) || (count % blockSize != 0)) {
		#if qLogging
			gLogFile << "Unaligned allocation block read" << endl_AC;
		#endif
		ThrowException_AC (kUnalignedAllocationBlockRead, 0);
	}
	start /= blockSize;
	count /= blockSize;
	OSErr err = readAllocationBlocks (start, count, buffer);
	if (err) return err;
	
	if (extraBytesAtStart) {
		UInt8 *mark = (UInt8 *) *buffer;
		mark += extraBytesAtStart;
		BlockMoveData (mark, *buffer, count * blockSize - extraBytesAtStart);
	}
	
	return noErr;
}

