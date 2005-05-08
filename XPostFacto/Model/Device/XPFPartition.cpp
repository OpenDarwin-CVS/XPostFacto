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


#include "MoreFilesExtras.h"
#include "MountedVolume.h"
#include "XPFErrors.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "HFSPlusCatalog.h"
#include "HFSPlusVolume.h"
#include "XPFLog.h"
#include "XPFBootableDevice.h"
#include "XPFAuthorization.h"
#include "XPostFacto.h"
#include "XPFPlatform.h"

union VolumeHeader {
	HFSMasterDirectoryBlock hfs;
	HFSPlusVolumeHeader hfsplus;
};

void
static XPFDumpMasterDirectoryBlock (HFSMasterDirectoryBlock *block) 
{
	gLogFile << "HFSMasterDirectoryBlock Info" << endl_AC;
	gLogFile << "drSigWord: " << block->drSigWord << endl_AC;
	gLogFile << "drCrDate: " << block->drCrDate << endl_AC;
	gLogFile << "drLsMod: " << block->drLsMod << endl_AC;
	gLogFile << "drAtrb: " << block->drAtrb << endl_AC;
	gLogFile << "drNmFls: " << block->drNmFls << endl_AC;
	gLogFile << "drVBMSt: " << block->drVBMSt << endl_AC;
	gLogFile << "drAllocPtr: " << block->drAllocPtr << endl_AC;
	gLogFile << "drNmAlBlks: " << block->drNmAlBlks << endl_AC;
	gLogFile << "drAlBlkSiz: " << block->drAlBlkSiz << endl_AC;
	gLogFile << "drClpSiz: " << block->drClpSiz << endl_AC;
	gLogFile << "drAlBlSt: " << block->drAlBlSt << endl_AC;
	gLogFile << "drNxtCNID: " << block->drNxtCNID << endl_AC;
	gLogFile << "drFreeBks: " << block->drFreeBks << endl_AC;
	gLogFile << "drVN: " << (CChar255_AC) block->drVN << endl_AC;
	gLogFile << "drVolBkUp: " << block->drVolBkUp << endl_AC;
	gLogFile << "drVSeqNum: " << block->drVSeqNum << endl_AC;
	gLogFile << "drWrCnt: " << block->drWrCnt << endl_AC;
	gLogFile << "drXTClpSiz: " << block->drXTClpSiz << endl_AC;
	gLogFile << "drCTClpSiz: " << block->drCTClpSiz << endl_AC;
	gLogFile << "drNmRtDirs: " << block->drNmRtDirs << endl_AC;
	gLogFile << "drFilCnt: " << block->drFilCnt << endl_AC;
	gLogFile << "drDirCnt: " << block->drDirCnt << endl_AC;
	gLogFile << "drEmbedSigWord: " << block->drEmbedSigWord << endl_AC;
	gLogFile << "drEmbedExtent.startBlock: " << block->drEmbedExtent.startBlock << endl_AC;
	gLogFile << "drXTFlSize: " << block->drXTFlSize << endl_AC;
	gLogFile << "drCTFlSize: " << block->drCTFlSize << endl_AC;
	
	unsigned long offset = block->drAlBlSt + block->drEmbedExtent.startBlock * (block->drAlBlkSiz / 512);
	gLogFile << "Calculated offset: " << offset << endl_AC;
}

XPFPartition::XPFPartition (XPFBootableDevice *device, Partition *part, int partNumber)
{
	fBootableDevice = device;
	fPartitionNumber = partNumber;
	fMountedVolume = NULL;
	fCreationDate = 0;
	fHFSPlusVolume = NULL;
	fOffsetToHFSPlusVolume = 0;
	
	BlockMoveData (part, &fPartition, sizeof (fPartition));
	gLogFile << "Partition: " << partNumber << " pmParType: " << (char *) part->pmParType << endl_AC;
	
	if (!strcmp ((char *) fPartition.pmParType, "Apple_HFS")) {
//		gLogFile << "pmPyPartStart: " << fPartition.pmPyPartStart << " pmLgDataStart: " << fPartition.pmLgDataStart << endl_AC;
	
		VolumeHeader *header = NULL;
		ThrowIfOSErr_AC (readBlocks (2, 1, (void **) &header));
		ThrowIfNULL_AC (header);
		
		if (header->hfs.drSigWord == kHFSSigWord) {
//			XPFDumpMasterDirectoryBlock ((HFSMasterDirectoryBlock *) header);
		
			fCreationDate = header->hfs.drCrDate;
			if (header->hfs.drEmbedSigWord == kHFSPlusSigWord) {
				fOffsetToHFSPlusVolume = (UInt32) header->hfs.drAlBlkSiz;
				fOffsetToHFSPlusVolume /= 512;
				fOffsetToHFSPlusVolume *= (UInt32) header->hfs.drEmbedExtent.startBlock;
				fOffsetToHFSPlusVolume += (UInt32) header->hfs.drAlBlSt;
			} else {
				fCreationDate = 0;
				#if qLogging
					gLogFile << "Not an HFS Plus volume" << endl_AC;
				#endif
			}
		} else if (header->hfsplus.signature == kHFSPlusSigWord) {
			fCreationDate = header->hfsplus.createDate;
			fOffsetToHFSPlusVolume = 0;
		} else {
			#if qLogging
				gLogFile << "Could not find correct signature for volume header" << endl_AC;
			#endif
			ThrowException_AC (kErrorFindingVolumeHeader, 0);
		}
		DisposePtr ((Ptr) header);
	}

	fExtendsPastEightGB = (part->pmPyPartStart + part->pmPartBlkCnt) > (8UL * 1024 / 512 * 1024 * 1024 );
	
	checkOpenFirmwareName ();

	if (fCreationDate) fHFSPlusVolume = new HFSPlusVolume (this, fOffsetToHFSPlusVolume);
	
	fBootableDevice->AddDependent (this);
}

void
XPFPartition::checkOpenFirmwareName ()
{
	sprintf (fOpenFirmwareName, "%s:%d", fBootableDevice->getOpenFirmwareName (false), fPartitionNumber);
	sprintf (fShortOpenFirmwareName, "%s:%d", fBootableDevice->getOpenFirmwareName (true), fPartitionNumber);

	Changed (cSetOpenFirmwareName, this);
}
	
void 
XPFPartition::DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace)
{
	#pragma unused (changedObject, changeData, dependencySpace)
	
	switch (theChange) {			
		case cSetBus:
			Changed (cSetBus, this);
			break;
			
		case cSetOpenFirmwareName:
			checkOpenFirmwareName ();
			break;
	}
}

XPFPartition::~XPFPartition ()
{
	if (fHFSPlusVolume) delete fHFSPlusVolume;
}

OSErr
XPFPartition::readBlocks (UInt32 start, UInt32 count, void **buffer)
{
	return fBootableDevice->readBlocks (start + fPartition.pmPyPartStart + 
			fPartition.pmLgDataStart, count, (UInt8 **) buffer);
}

OSErr
XPFPartition::writeBlocks (UInt32 start, UInt32 count, UInt8 *buffer)
{
	return fBootableDevice->writeBlocks (start + fPartition.pmPyPartStart +
			fPartition.pmLgDataStart, count, buffer);
}

bool
XPFPartition::matchInfo (FSVolumeInfo *info)
{
	if (!fHFSPlusVolume) return false;
	return fHFSPlusVolume->matchInfo (info);
}

bool 
XPFPartition::matchInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name)
{
	if (!fHFSPlusVolume) return false;
	return fHFSPlusVolume->matchInfoAndName (info, name);
}

void
XPFPartition::setBootXValues (unsigned loadAddress, unsigned entryPoint, unsigned size) 
{
	// The idea here is that we have done a fresh BootX installation and need to set up
	// the correct partition information for Old World systems. It probably wouldn't cause
	// any problems on New World machines, but we'll leave it alone anyway out of an abundance
	// of caution.
	
	if (XPFPlatform::GetPlatform()->getIsNewWorld ()) return;
		
	fPartition.pmPartStatus |=  kPartitionAUXIsValid |
								kPartitionAUXIsAllocated |
								kPartitionAUXIsInUse |
								kPartitionAUXIsBootValid |
								kPartitionAUXIsReadable |
								kPartitionAUXIsWriteable |
								kPartitionAUXIsBootCodePositionIndependent;
								
	fPartition.pmBootSize 	= size;
	fPartition.pmBootAddr 	= loadAddress;
	fPartition.pmBootEntry 	= entryPoint + 12; // we're skipping the prologue, which causes problems on the 6500
	fPartition.pmBootCksum	= 0;
	strcpy ((char *) fPartition.pmProcessor, "powerpc");
	#if qLogging
		gLogFile << "pmBootSize: " << size << " pmBootAddr: " << loadAddress
			<< " pmBootEntry: " << entryPoint << endl_AC;
	#endif
}

void
XPFPartition::writePartition ()
{
	// The idea here is that we have done a fresh BootX installation and need to set up
	// the correct partition information for Old World systems. It probably wouldn't cause
	// any problems on New World machines, but we'll leave it alone anyway out of an abundance
	// of caution.
	
	if (XPFPlatform::GetPlatform()->getIsNewWorld ()) return;

	gLogFile << "Writing partition info ..." << endl_AC;
	
	#if __MACH__
		bool success = false;
		
		if (!fMountedVolume) return;

		io_object_t partInfo = fMountedVolume->getPartitionInfo ();
		if (partInfo) {		
			CFTypeRef keys[7] = {
				CFSTR ("Processor Type"), 
				CFSTR ("Boot Block"), 
				CFSTR ("Boot Bytes"), 
				CFSTR ("Boot Address"), 
				CFSTR ("Boot Entry"), 
				CFSTR ("Boot Checksum"),
				CFSTR ("Partition Status")
			};
						
			CFTypeRef values[7] = {
				CFStringCreateWithCString (NULL, (char *) fPartition.pmProcessor, kCFStringEncodingASCII),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmLgBootStart),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootSize),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootAddr),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootEntry),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootCksum),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmPartStatus)
			};
			
			CFDictionaryRef props = CFDictionaryCreate (NULL, keys, values, 7, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			if (props) {
				kern_return_t kr = IORegistryEntrySetCFProperties (partInfo, props);
				if (kr == KERN_SUCCESS) success = true;
				CFRelease (props);
			}
			
			for (int x = 0; x < 7; x++) CFRelease (values[x]);
			IOObjectRelease (partInfo); 
		}
		
		if (!success) ThrowException_AC (kWritePartitionOSX, 0);
		
	#else		

		ThrowIfOSErr_AC (fBootableDevice->writeBlocks (fPartitionNumber, 1, (UInt8 *) &fPartition));

	#endif
}

OSErr
XPFPartition::writeBootBlocks (void *buffer)
{
#ifdef __MACH__
	char path[1024];
	OSErr err = FSRefMakePath (fMountedVolume->getRootDirectory (), (UInt8 *) path, 1023);
	if (err) {
		gLogFile << "Could not get path for mountpoint" << endl_AC;
	} else { 
	    XPFSetUID myUID (0);
		int fd = open (path, O_RDONLY);
	    if (fd == -1) {
			gLogFile << "Could not open mountpoint to write boot blocks" << endl_AC;
	    } else {
			fbootstraptransfer_t btrans;
			btrans.fbt_offset = 0;
			btrans.fbt_length = 1024;
			btrans.fbt_buffer = buffer;
	    
		    err = fcntl (fd, F_WRITEBOOTSTRAP, &btrans);
		    if (err) gLogFile << "Error " << err << " writing boot blocks" << endl_AC;
		    close (fd);
	    }
	}
	return err;
#else
	return writeBlocks (0, 2, (UInt8 *) buffer);
#endif
}

OSErr
XPFPartition::readBootBlocks (void **buffer)
{
	return readBlocks (0, 2, buffer);
}

void
XPFPartition::installBootX ()
{
	if (fMountedVolume) {
		fMountedVolume->installBootXFile ();
	} else {
		if (!XPFPlatform::GetPlatform()->getIsNewWorld()) {
			// If we're on Old World and there is no mounted volume corresponding
			// to the partition, then we make sure the processor field isn't set to powerpc
			if (!strcmp ((char *) fPartition.pmProcessor, "powerpc")) {
				gLogFile << "Disabling boot partition from previous installation" << endl_AC;
				strcpy ((char *) fPartition.pmProcessor, "");
				this->writePartition ();
			}
		}
	}
}

UInt32
XPFPartition::getBootXVersion ()
{
	return fMountedVolume ? fMountedVolume->getBootXVersion () : 0;
}

bool 
XPFPartition::getClaimsBootXInstalled ()
{
	return !strcmp ((char *) fPartition.pmProcessor, "powerpc");
}

unsigned long 
XPFPartition::getBootXStartBlock ()
{
	return fHFSPlusVolume ? fHFSPlusVolume->getBootXStartBlock () : 0;
}

#if qDebug

void
XPFPartition::dump ()
{
	#if qDebug
		printf ("partitionNumber:%u, isAppleHFS:%d, bootXVersion:0x%X, creationDate:%u\r",
			 fPartitionNumber, getBootXVersion (), fCreationDate);
	#endif
}

#endif


