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

#include "HFSPlusCatalogNode.h"
#include "HFSPlusCatalog.h"
#include "XPFLog.h"
#include <iostream.h>

#include "FastUnicodeCompare.h"

HFSPlusCatalogNode::HFSPlusCatalogNode (HFSPlusCatalog *catalog, UInt32 nodeNumber)
{
	fCatalog = catalog;
	fData = NULL;
	
	// read in the data!
	ThrowIfOSErr_AC (fCatalog->readBytes (nodeNumber * fCatalog->getNodeSize (), fCatalog->getNodeSize (), (void **) &fData));
	BlockMoveData (fData, &fNodeDescriptor, sizeof (fNodeDescriptor));
	
	#if qLogging
		gLogFile << "Node Data" << endl_AC;
		gLogFile << "nodeNumber: " << nodeNumber << endl_AC;
		gLogFile << "bLink: " << fNodeDescriptor.bLink << endl_AC;
		gLogFile << "fLink: " << fNodeDescriptor.fLink << endl_AC;
		gLogFile << "kind: " << (int) fNodeDescriptor.kind << endl_AC;
		gLogFile << "numRecords: " << fNodeDescriptor.numRecords << endl_AC; 
	#endif
}

HFSPlusCatalogNode*
HFSPlusCatalogNode::newNextNode ()
{
	if (fNodeDescriptor.fLink) {
		return new HFSPlusCatalogNode (fCatalog, fNodeDescriptor.fLink);
	} else {
		return NULL;
	}
}

int
HFSPlusCatalogNode::findEntry (HFSCatalogNodeID parentID, const HFSUniStr255& nodeName, HFSPlusCatalogFile *entry)
{
	if (entry == NULL) return paramErr;
	UInt16 *recordOffset = (UInt16 *) (fData + fCatalog->getNodeSize ());
	for (int x = 0; x < fNodeDescriptor.numRecords; x++) {
		recordOffset--;
		HFSPlusCatalogKey *key = (HFSPlusCatalogKey *) (fData + *recordOffset);
		HFSPlusCatalogFile *file = (HFSPlusCatalogFile *) (fData + *recordOffset + key->keyLength + sizeof (UInt16));
		#if qLogging
			gLogFile << "parentID: " << key->parentID << endl_AC;
		#endif
		if (parentID < key->parentID) {
			#if qLogging
				gLogFile << "Gone past desired parent ID" << endl_AC;
			#endif
			return -1; // we've gone too far
		} else if (key->parentID == parentID) {
			// we might have one. Check the names.
			SInt32 compare = FastUnicodeCompare (nodeName.unicode, nodeName.length, 
									key->nodeName.unicode, key->nodeName.length);
			if (compare == 0) {
				// We've got one
				BlockMoveData (file, entry, sizeof (HFSPlusCatalogFile));
				return 1;
			} else if (compare == -1) {
				// We've gone too far
				#if qLogging
					gLogFile << "Gone past desired file name. Either BootX.image not here, or keys out of order" << endl_AC;
				#endif
				// I'm going to not do anything special here. We'll catch it with the parent, ID,
				// in case just the name is out of order
				// return -1;
			}
		}
	}
	return 0;
}

int 
HFSPlusCatalogNode::findVolumeName (HFSUniStr255 *outName)
{
	UInt16 *recordOffset = (UInt16 *) (fData + fCatalog->getNodeSize ());
	for (int x = 0; x < fNodeDescriptor.numRecords; x++) {
		recordOffset--;
		HFSPlusCatalogKey *key = (HFSPlusCatalogKey *) (fData + *recordOffset);
		#if qLogging
			gLogFile << "parentID: " << key->parentID << endl_AC;
		#endif
		if (fsRtParID < key->parentID) {
			#if qLogging
				gLogFile << "Gone past desired parent ID" << endl_AC;
			#endif
			return -1; // we've gone too far
		} else if (key->parentID == fsRtParID) {
			// We've got one
			BlockMoveData (&key->nodeName, outName, sizeof (HFSUniStr255));
			return 1;
		}
	}
	return 0;
}

HFSPlusCatalogNode::~HFSPlusCatalogNode ()
{
	if (fData) DisposePtr (fData);
}