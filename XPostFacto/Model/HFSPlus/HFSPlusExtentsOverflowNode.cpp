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

#include "HFSPlusExtentsOverflowNode.h"
#include "HFSPlusExtentsOverflow.h"
#include "XPFLog.h"
#include <iostream.h>

HFSPlusExtentsOverflowNode::HFSPlusExtentsOverflowNode (HFSPlusExtentsOverflow *extentsOverflow, UInt32 nodeNumber)
{
	fExtentsOverflow = extentsOverflow;
	fData = NULL;
	
	// read in the data!
	ThrowIfOSErr_AC (fExtentsOverflow->readBytes (nodeNumber * fExtentsOverflow->getNodeSize (), fExtentsOverflow->getNodeSize (), (void **) &fData));
	BlockMoveData (fData, &fNodeDescriptor, sizeof (fNodeDescriptor));
		
//	displayRecords ();
}

HFSPlusExtentsOverflowNode*
HFSPlusExtentsOverflowNode::newNextNode ()
{
	if (fNodeDescriptor.fLink) {
		return new HFSPlusExtentsOverflowNode (fExtentsOverflow, fNodeDescriptor.fLink);
	} else {
		return NULL;
	}
}

void
HFSPlusExtentsOverflowNode::displayRecords () {
	UInt16 *recordOffset = (UInt16 *) (fData + fExtentsOverflow->getNodeSize ());
	for (int x = 0; x < fNodeDescriptor.numRecords; x++) {
		recordOffset--;
		HFSPlusExtentKey *key = (HFSPlusExtentKey *) (fData + *recordOffset);
		#if qLogging
			gLogFile << "fileID: " << key->fileID << endl_AC;
		#endif		
		HFSPlusExtentDescriptor *record = (HFSPlusExtentDescriptor *) (fData + *recordOffset + key->keyLength + sizeof (UInt16));
		for (int y = 0; y < 8; y++) {
			if (record[y].startBlock == 0) break;
			#if qLogging
				gLogFile << "startBlock: " << record[y].startBlock 
						<< " blockCount: " << record[y].blockCount << endl_AC;
			#endif
		}
	}
}

OSErr
HFSPlusExtentsOverflowNode::addExtentsFor (HFSCatalogNodeID nodeID, TemplateArray_AC<HFSPlusExtentDescriptor> *extentsArray, UInt32 *totalBlocksSeen)
{
	UInt16 *recordOffset = (UInt16 *) (fData + fExtentsOverflow->getNodeSize ());
	for (int x = 0; x < fNodeDescriptor.numRecords; x++) {
		recordOffset--;
		HFSPlusExtentKey *key = (HFSPlusExtentKey *) (fData + *recordOffset);
		if (key->fileID < nodeID) {
			// haven't got far enough yet, so just keep going
			continue;	
		} else if (key->fileID == nodeID) {
			// got one!
			HFSPlusExtentDescriptor *record = (HFSPlusExtentDescriptor *) (fData + *recordOffset + key->keyLength + sizeof (UInt16));
			for (int y = 0; y < 8; y++) {
				if (record[y].startBlock == 0) break;
				*totalBlocksSeen += record[y].blockCount;
				extentsArray->InsertElementInOrder (&record[y]);
			}
		} else {
			// we've gone too far, so signal an error
			return 1;
		}
	}
	return noErr;
}

HFSPlusExtentsOverflowNode::~HFSPlusExtentsOverflowNode ()
{
	if (fData) DisposePtr (fData);
}