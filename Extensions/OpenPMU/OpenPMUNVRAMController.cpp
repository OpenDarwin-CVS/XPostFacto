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

#include <IOKit/IOSyncer.h>
#include "OpenPMUNVRAMController.h"
#include "OpenPMU.h"

#define super IONVRAMController
OSDefineMetaClassAndStructors(OpenPMUNVRAMController, IONVRAMController)


// **********************************************************************************
// init
//
// **********************************************************************************
bool OpenPMUNVRAMController::init ( OSDictionary * properties, OpenPMU * driver )
{
    PMUdriver = driver;

    return super::init(properties);
}


// **********************************************************************************
// read
//
// The NVRAM driver is calling to read part of the NVRAM.  We translate this into
// single-byte PMU commands and enqueue them to its command queue.
//
// **********************************************************************************
IOReturn OpenPMUNVRAMController::read ( IOByteCount offset, UInt8 * buffer, IOByteCount length )
{
    IOByteCount	i;
    UInt8 *		client_buffer = buffer;
    UInt32		our_offset = offset;

    if ( (buffer == NULL) ||
         (length == 0) ||
         (length > 8192) ||
         (offset > 8192) ||
         ((length + offset) > 8192) ) {
      return kIOReturnBadArgument;
    }

    for ( i = 0; i < length ; i++ ) {
        UInt8 oBuffer[2];
        IOByteCount iLen = 1;
        oBuffer[0] = our_offset >> 8;
        oBuffer[1] = our_offset++;
        PMUdriver->sendMiscCommand (kPMUNVRAMRead, 2, oBuffer, &iLen, client_buffer++);
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
    IOByteCount	i;
    UInt32		our_offset = offset;
    UInt8 *		client_buffer = buffer;

    if ( (buffer == NULL) ||
         (length == 0) ||
         (length > 8192) ||
         (offset > 8192) ||
         ((length + offset) > 8192) ) {
      return kIOReturnBadArgument;
    }

    for ( i = 0; i < length ; i++ ) {
        UInt8 oBuffer[3], iBuffer = 0;
        IOByteCount iLen = sizeof(iBuffer);
        oBuffer[0] = our_offset >> 8;
        oBuffer[1] = our_offset++;
        oBuffer[2] = *client_buffer++;
        PMUdriver->sendMiscCommand (kPMUNVRAMWrite, 3, oBuffer, &iLen, &iBuffer);
    }

    return kIOReturnSuccess;
}
