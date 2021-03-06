/*

Copyright (c) 2001 - 2004
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

#ifndef __XPFPARTITION_H__
#define __XPFPARTITION_H__

#ifndef __MACH__
	#include <AppleDiskPartitions.h>
#endif

class XPFBootableDevice;
class MountedVolume;
class HFSPlusVolumeHeader;
class HFSPlusVolume;

class XPFPartition : public MDependable_AC
{
	private:

		unsigned int fPartitionNumber;
		Partition fPartition;
		MountedVolume *fMountedVolume;
		
		unsigned int fCreationDate;
		UInt32 fBlockSize;
		UInt32 fTotalBlocks;
		
		bool fExtendsPastEightGB;
		XPFBootableDevice *fBootableDevice;
		char fOpenFirmwareName[256];
		char fShortOpenFirmwareName[256];
		unsigned long fOffsetToHFSPlusVolume;
		
		HFSPlusVolume *fHFSPlusVolume;
		
		void checkOpenFirmwareName ();
	
	public:
	
		XPFPartition (XPFBootableDevice *device, Partition *part, int partNumber);
		~XPFPartition ();

		MountedVolume *getMountedVolume () {return fMountedVolume;}
		void setMountedVolume (MountedVolume *vol) {fMountedVolume = vol;}

		unsigned int getPartitionNumber () {return fPartitionNumber;}
		unsigned int getCreationDate () {return fCreationDate;}
		bool getExtendsPastEightGB () {return fExtendsPastEightGB;}
		bool getHasHFSWrapper () {return fOffsetToHFSPlusVolume;}
		const char* getOpenFirmwareName (bool useShortName) {return useShortName ? fShortOpenFirmwareName : fOpenFirmwareName;}
		bool getIsHFSPlusVolume () {return fHFSPlusVolume;}

		bool getClaimsBootXInstalled ();
		UInt32 getBootXVersion ();
		void installBootX ();
		void setBootXValues (unsigned loadAddress, unsigned entryPoint, unsigned size);
		unsigned long getBootXStartBlock ();
		
		unsigned long getLgBootStart () {return fPartition.pmLgBootStart;}
		void setLgBootStart (unsigned long start) {fPartition.pmLgBootStart = start;}

		char* getProcessor () {return (char *) fPartition.pmProcessor;}
		
		bool matchInfo (FSVolumeInfo *info);
		bool matchInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name);
				
		OSErr readBlocks (UInt32 start, UInt32 count, void **buffer);
		OSErr writeBlocks (UInt32 start, UInt32 count, UInt8 *buffer);
		void writePartition ();
		OSErr writeBootBlocks (void *buffer);
		OSErr readBootBlocks (void **buffer);
		
		void DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace);
		
		void dump ();
};

typedef TemplateAutoList_AC <XPFPartition> PartitionList;
typedef TemplateAutoList_AC <XPFPartition>::Iterator PartitionIterator;

#endif
