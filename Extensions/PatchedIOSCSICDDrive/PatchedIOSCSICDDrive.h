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

#ifndef	_PATCHED_IOSCSICDDRIVE_H
#define	_PATCHED_IOSCSICDDRIVE_H

#include "IOSCSICDDrive.h"
#include "IOSCSICDDriveNub.h"

#define kPatchedIOSCSICDDriveClass "PatchedIOSCSICDDrive"

class PatchedIOSCSICDDrive : public IOSCSICDDrive {

    OSDeclareDefaultStructors(PatchedIOSCSICDDrive)
	
protected:
	
	bool bcdToHexConversionHasBeenDone;

public:

    virtual bool	deviceTypeMatches(UInt8 inqBuf[],UInt32 inqLen,SInt32 *score);

    virtual IOReturn	readTOC(IOMemoryDescriptor * buffer);

    virtual IOReturn	doAsyncReadCD(IOMemoryDescriptor *buffer,
                                      UInt32 block,UInt32 nblks,
                                      CDSectorArea sectorArea,
                                      CDSectorType sectorType,
                                      IOStorageCompletion completion);

};

#endif