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

#ifndef __HFSPLUSVOLUME_H__
#define __HFSPLUSVOLUME_H__

#include "XPFPartition.h"

#include <HFSVolumes.h>

class MountedVolume;

class HFSPlusVolume {

	private:

		HFSPlusVolumeHeader *fHeader;
	
		unsigned long fOffsetIntoPartition;
		XPFPartition *fPartition;
		bool fBootXInstallationComplete;
		HFSUniStr255 fVolumeName;

		unsigned long getBootXStartBlock ();
		MountedVolume *getMountedVolume () {if (fPartition) return fPartition->getMountedVolume (); else return NULL;}

	public:
		HFSPlusVolume (XPFPartition *thePartition, unsigned long offsetToVolume);
		~HFSPlusVolume ();

		HFSPlusVolumeHeader* getHeader () {return fHeader;}
		UInt32 getBlockSize () {return fHeader->blockSize;}
		
		OSErr readBlocks (unsigned int start, unsigned int count, void **buffer);
		OSErr readAllocationBlocks (unsigned int start, unsigned int count, void **buffer);
		
		void installBootXIfNecessary (bool forceInstall = false);
		void installBootX ();
		UInt32 getBootXVersion ();

		bool matchInfo (FSVolumeInfo *info);
		bool matchInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name);

		static SInt16 CompareVersions( NumVersion *vers1, NumVersion *vers2 );

};

#endif