/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <IOKit/IOLib.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/scsi/IOSCSIDeviceInterface.h>

#include "PatchedIOSCSICDDrive.h"

#define	super	IOSCSICDDrive
OSDefineMetaClassAndStructors(PatchedIOSCSICDDrive,IOSCSICDDrive)

static void __inline ConvertBCDToHex(UInt8 *value)
{
    *value = (((*value) >> 4) * 10) + ((*value) & 0x0f);
}

static void ConvertDescriptorsFromBCDToHex (CDTOCDescriptor descriptors[], UInt32 count)
{
    /* Convert BCD-encoded values in TOC to hex values. */

	for (UInt32 index = 0; index < count; index++) {
		if (descriptors[index].point <= 0x99) {
			ConvertBCDToHex (&descriptors[index].point);
		}
		if ((descriptors[index].point & 0xf0) == 0xb0) {
			ConvertBCDToHex(&descriptors[index].address.minute);
			ConvertBCDToHex(&descriptors[index].address.second);
			ConvertBCDToHex(&descriptors[index].address.frame);
			ConvertBCDToHex(&descriptors[index].zero);
		}
		if (descriptors[index].point <= 0x99 ||	
				(descriptors[index].point >= 0xa0 && descriptors[index].point <= 0xc0)) {
			ConvertBCDToHex(&descriptors[index].p.minute);
			if (descriptors[index].point != 0xa0) {
				ConvertBCDToHex(&descriptors[index].p.second);
			}
			ConvertBCDToHex(&descriptors[index].p.frame);
		}
	}
}

IOReturn
PatchedIOSCSICDDrive::doAsyncReadCD(IOMemoryDescriptor *buffer,
                             UInt32 block,UInt32 nblks,
                             CDSectorArea sectorArea,
                             CDSectorType sectorType,
                             IOStorageCompletion completion)
{
	// On older drives, you get the wrong values here when doing a CD audio read
	// so we just fix them and call super's implementation. I am making the 
	// assumption that the drives which require this fix are the same as the drives
	// which require the BCD to Hex conversion. That may not be a completely
	// safe assumption, but there you are.
		
	if (bcdToHexConversionHasBeenDone && (sectorArea == 248) && (sectorType == 0)) {
		sectorArea = kCDSectorAreaUser;
		sectorType = kCDSectorTypeCDDA;
	}

	return super::doAsyncReadCD (buffer, block, nblks, sectorArea, sectorType, completion);
}

bool	
PatchedIOSCSICDDrive::deviceTypeMatches(UInt8 inqBuf[],UInt32 inqLen,SInt32 *score)
{
	bool retVal = super::deviceTypeMatches (inqBuf, inqLen, score);
	if (retVal) *score = 20000;  // we're outmaching both our superclass and IOSCSIPeripheralDeviceType05
	return retVal;
}

IOReturn
PatchedIOSCSICDDrive::readTOC(IOMemoryDescriptor *buffer)
{
    // This works around a bug that may be present in our superclass (it may
	// release buffer without retaining it). But the bug may be fixed, so
	// we check for that.
	
	buffer->retain();
	int bufferRetainCount = buffer->getRetainCount();
	IOReturn result = super::readTOC(buffer);
	if (buffer->getRetainCount() == bufferRetainCount) buffer->release();
	
	// Now we see whether BCD to hex conversion is required. The strategy is to figure out
	// the total number of blocks in two ways, one of which is definitely accurate, and the
	// other which is only accurate when BCD to hex conversion is unnecessary. If the two 
	// totals are off by more than 150 blocks (about two seconds), then we convert BCD to hex.
	
	bcdToHexConversionHasBeenDone = false;
	
	UInt64 theActualNumberOfBlocks;
	this->reportMaxValidBlock(&theActualNumberOfBlocks);
	theActualNumberOfBlocks++; // we end up calculating the next block past the last one
	
	UInt32 theHighestBlockWeHaveSeen = 0;
	UInt32 theCorrectedHighestBlock = 0;
	
	IOByteCount tocMaxSize;
    CDTOC *toc = (CDTOC *) (buffer->getVirtualSegment(0, &tocMaxSize));

    if (toc && tocMaxSize >= sizeof(UInt32)) {
        UInt32 count = (tocMaxSize - sizeof(UInt32)) / sizeof(CDTOCDescriptor);
        for (UInt32 index = 0; index < count; index++) {
			if (toc->descriptors[index].session == toc->sessionLast) {
				theHighestBlockWeHaveSeen = max (theHighestBlockWeHaveSeen, CDConvertMSFToLBA (toc->descriptors[index].p));
			}
		}
		int theDifference = theActualNumberOfBlocks - theHighestBlockWeHaveSeen;
		if ((theDifference < -150) || (theDifference > 150)) {
			ConvertDescriptorsFromBCDToHex (toc->descriptors, count);
			bcdToHexConversionHasBeenDone = true;
	        for (UInt32 index = 0; index < count; index++) {
				if (toc->descriptors[index].session == toc->sessionLast) {
					theCorrectedHighestBlock = max (theCorrectedHighestBlock, CDConvertMSFToLBA (toc->descriptors[index].p));
				}
			}
		}
    }

	setProperty ("BCDToHexConversion", bcdToHexConversionHasBeenDone);
	setProperty ("ActualNumberOfBlocks", theActualNumberOfBlocks, 64);
	setProperty ("HighestBlockSeen", theHighestBlockWeHaveSeen, 32);
	if (bcdToHexConversionHasBeenDone) {
		setProperty ("CorrectedHighestBlock", theCorrectedHighestBlock, 32);
	}
	
    return(result);
}
