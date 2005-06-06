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
/*
 *  1 Dec 1998 suurballe  Created.
 */

#include "OpenPMUNVRAMController.h"
#include "../OpenOldWorldNVRAM/OpenOldWorldNVRAM.h"

#include <IOKit/IOLib.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/platform/ApplePlatformExpert.h>
#include <IOKit/IOSyncer.h>

enum {
    kPMUNVRAMWrite	= 0x33,			// write NVRAM byte
    kPMUNVRAMRead	= 0x3B, 		// read NVRAM byte
};

#define super IONVRAMController

OSDefineMetaClassAndStructors(OpenPMUNVRAMController, IONVRAMController)

// **********************************************************************************
// init
//
// **********************************************************************************
bool 
OpenPMUNVRAMController::init (OSDictionary *properties, IOService *driver)
{
    fPMUdriver = driver;
	
    return super::init(properties);
}

// **********************************************************************************
// start
//
// **********************************************************************************
bool
OpenPMUNVRAMController::start (IOService *provider)
{
	// If we're "Old World", then pretend we're "New World"
	if (getPlatform()->getBootROMType() == kBootROMTypeOldWorld) {
		getPlatform()->setBootROMType (kBootROMTypeNewWorld);
		setProperty ("EmulatingNewWorld", true);
	
		fOldWorldBuffer = (UInt8*) IOMalloc (kNVRAMImageSize);
		fNewWorldBuffer = (UInt8*) IOMalloc (kNVRAMImageSize);
		
		bzero (fOldWorldBuffer, kNVRAMImageSize);
		bzero (fNewWorldBuffer, kNVRAMImageSize);		
	}
	
	bool retVal = super::start (provider);
	if (retVal) registerService (); // So the XPostFacto application can find us
	
	return retVal;
}

// **********************************************************************************
// free
//
// **********************************************************************************
void
OpenPMUNVRAMController::free ()
{
	if (fOldWorldBuffer) {
		IOFree (fOldWorldBuffer, kNVRAMImageSize);
		fOldWorldBuffer = NULL;
	}
	
	if (fNewWorldBuffer) {
		IOFree (fNewWorldBuffer, kNVRAMImageSize);
		fNewWorldBuffer = NULL;
	}	
}

// **********************************************************************************
// read
//
// The NVRAM driver is calling to read part of the NVRAM.  We translate this into
// single-byte PMU commands and enqueue them to its command queue.
//
// **********************************************************************************
IOReturn 
OpenPMUNVRAMController::read (IOByteCount offset, UInt8 *buffer, IOByteCount length)
{
    UInt8 *callersBuffer = buffer;
	IOByteCount callersOffset = offset;
	IOByteCount callersLength = length;

    if ( (buffer == NULL) ||
         (length == 0) ||
         (length > 8192) ||
         (offset > 8192) ||
         ((length + offset) > 8192) ) {
      return kIOReturnBadArgument;
    }

	static bool buffered = false;
	if (buffered) {
		bcopy (fNewWorldBuffer + callersOffset, callersBuffer, callersLength);
		return kIOReturnSuccess;
	}

	if (fOldWorldBuffer) {
		// If we're converting from Old World to New World, then we always need to read everything
		// (since the New World offsets won't be the same as Old World offsets). We then apply the
		// caller's offset and length later.
		buffer = fOldWorldBuffer;  
		offset = 0;
		length = kNVRAMImageSize;
	}
	
    IOByteCount	i;
	UInt8 *client_buffer = buffer;
    UInt32 our_offset = offset;

    for (i = 0; i < length; i++) {
        UInt8 oBuffer[2];
        IOByteCount iLen = 1;
        oBuffer[0] = our_offset >> 8;
        oBuffer[1] = our_offset++;
        sendPMUCommand (kPMUNVRAMRead, 2, oBuffer, &iLen, client_buffer++);
    }

	if (fOldWorldBuffer) {
		OpenOldWorldNVRAM *owNVRAM = OpenOldWorldNVRAM::withBuffers ((char *) fOldWorldBuffer, (char *) fNewWorldBuffer);
		owNVRAM->copyOldWorldToNewWorld ();
		owNVRAM->release ();
		
		bcopy (fNewWorldBuffer + callersOffset, callersBuffer, callersLength); 
		buffered = true;
	}
  
    return kIOReturnSuccess;
}


// **********************************************************************************
// write
//
// The NVRAM driver is calling to write part of the NVRAM.  We translate this into
// single-byte PMU commands and enqueue them to our command queue.
//
// **********************************************************************************
IOReturn OpenPMUNVRAMController::write ( IOByteCount offset, UInt8 * buffer, IOByteCount length )
{
    if ( (buffer == NULL) ||
         (length == 0) ||
         (length > 8192) ||
         (offset > 8192) ||
         ((length + offset) > 8192) ) {
      return kIOReturnBadArgument;
    }

	if (fNewWorldBuffer) {
		bcopy (buffer, fNewWorldBuffer + offset, length);

		OpenOldWorldNVRAM *owNVRAM = OpenOldWorldNVRAM::withBuffers ((char *) fOldWorldBuffer, (char *) fNewWorldBuffer);
		owNVRAM->copyNewWorldToOldWorld ();
		owNVRAM->release ();

		// When converting to Old World, we always need to write everything, because the caller's offset
		// and length won't correspond to the Old World offset and length.
		buffer = fOldWorldBuffer;
		offset = 0;
		length = kNVRAMImageSize;
	}
  
    IOByteCount	i;
    UInt32		our_offset = offset;
    UInt8 *		client_buffer = buffer;

    for ( i = 0; i < length ; i++ ) {
        UInt8 oBuffer[3], iBuffer = 0;
        IOByteCount iLen = sizeof(iBuffer);
        oBuffer[0] = our_offset >> 8;
        oBuffer[1] = our_offset++;
        oBuffer[2] = *client_buffer++;
        sendPMUCommand (kPMUNVRAMWrite, 3, oBuffer, &iLen, &iBuffer);
    }

    return kIOReturnSuccess;
}

typedef struct SendMiscCommandParameterBlock {
    int command;
    IOByteCount sLength;
    UInt8 *sBuffer;
    IOByteCount *rLength;
    UInt8 *rBuffer;
} SendMiscCommandParameterBlock;

void
OpenPMUNVRAMController::sendPMUCommand (UInt32 command, IOByteCount sendLength, UInt8 *sendBuffer, IOByteCount *readLength, UInt8 *readBuffer)
{
    SendMiscCommandParameterBlock param = {command, sendLength, sendBuffer, readLength, readBuffer};
	if (fPMUdriver) fPMUdriver->callPlatformFunction ("sendMiscCommand", true, (void *) &param, NULL, NULL, NULL);
}
