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

#include "HFSPlusCatalog.h"
#include "HFSPlusVolume.h"
#include "XPFLog.h"
#include "XPFBootableDevice.h"
#include "XPFAuthorization.h"
#include "XPostFacto.h"

union VolumeHeader {
	HFSMasterDirectoryBlock hfs;
	HFSPlusVolumeHeader hfsplus;
};

XPFPartition::XPFPartition (XPFBootableDevice *device, Partition *part, int partNumber)
{
	fBootableDevice = device;
	fPartitionNumber = partNumber;
	fMountedVolume = NULL;
	fCreationDate = 0;
	fHFSPlusVolume = NULL;
	BlockMoveData (part, &fPartition, sizeof (fPartition));
	if (!strcmp ((char *) fPartition.pmParType, "Apple_HFS")) {
		VolumeHeader *header = NULL;
		ThrowIfOSErr_AC (readBlocks (2, 1, (void **) &header));
		ThrowIfNULL_AC (header);
		if (header->hfs.drSigWord == kHFSSigWord) {
			fCreationDate = header->hfs.drCrDate;
			if (header->hfs.drEmbedSigWord == kHFSPlusSigWord) {
				fOffsetToHFSPlusVolume = header->hfs.drAlBlSt 
								+ header->hfs.drEmbedExtent.startBlock * (header->hfs.drAlBlkSiz / 512);
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

	fOpenFirmwareName.CopyFrom (device->getOpenFirmwareName (false));
	fShortOpenFirmwareName.CopyFrom (device->getOpenFirmwareName (true));
	char buffer[8];
	snprintf (buffer, 8, ":%d", fPartitionNumber);
	fOpenFirmwareName += buffer;
	fShortOpenFirmwareName += buffer;
	
	if (fCreationDate) fHFSPlusVolume = new HFSPlusVolume (this, fOffsetToHFSPlusVolume);

	#if qLogging
		if (fCreationDate) gLogFile << "Partition: " << partNumber << " CreationDate: " << fCreationDate << endl_AC;
	#endif
}

XPFPartition::~XPFPartition ()
{
	if (fHFSPlusVolume) delete fHFSPlusVolume;
}

OSErr
XPFPartition::readBlocks (unsigned int start, unsigned int count, void **buffer)
{
	return fBootableDevice->readBlocks (start + fPartition.pmPyPartStart + 
			fPartition.pmLgDataStart, count, (UInt8 **) buffer);
}

OSErr 
XPFPartition::writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer)
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
	// The idea here is that we have done a fresh BootX installation.
		
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

bool 
XPFPartition::getValidOpenFirmwareName () {
	return fBootableDevice->getValidOpenFirmwareName ();
}

void
XPFPartition::writePartition ()
{
	#if qLogging
		gLogFile << "Writing partition info ..." << endl_AC;
	#endif
	
	#if __MACH__
		bool success = false;
		
		if (!fMountedVolume) return;

		io_object_t partInfo = fMountedVolume->getPartitionInfo ();
		if (partInfo) {		
			CFTypeRef keys[6] = {
				CFSTR ("Processor Type"), 
				CFSTR ("Boot Block"), 
				CFSTR ("Boot Bytes"), 
				CFSTR ("Boot Address"), 
				CFSTR ("Boot Entry"), 
				CFSTR ("Boot Checksum")
			};
			
			CFTypeRef values[6] = {
				CFStringCreateWithCString (NULL, (char *) fPartition.pmProcessor, kCFStringEncodingASCII),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmLgBootStart),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootSize),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootAddr),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootEntry),
				CFNumberCreate (NULL, kCFNumberLongType, &fPartition.pmBootCksum)
			};
 						
			CFDictionaryRef props = CFDictionaryCreate (NULL, keys, values, 6, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			if (props) {
				kern_return_t kr = IORegistryEntrySetCFProperties (partInfo, props);
				if (kr == KERN_SUCCESS) success = true;
				CFRelease (props);
			}
			
			for (int x = 0; x < 6; x++) CFRelease (values[x]);
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
	// If it is a mounted volume, we ask it to install. 
	// Otherwise, we make sure the processor field isn't set to powerpc
	if (fMountedVolume) {
		fMountedVolume->installBootXImageFile ();
	} else {
		if (!strcmp ((char *) fPartition.pmProcessor, "powerpc")) {
			#if qLogging
				gLogFile << "Disabling boot partition from previous installation" << endl_AC;
			#endif
			strcpy ((char *) fPartition.pmProcessor, "");
			this->writePartition ();
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


