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

// This is a patched subclass for IOCDBlockStorageDriver, to work
// around a bug in build 4K78.
// Ryan Rempel (ryan.rempel@utoronto.ca)

#include <IOKit/IOLib.h>
#include "PatchedAppleNVRAM.h"


#define super IONVRAMController
OSDefineMetaClassAndStructors(PatchedAppleNVRAM, IONVRAMController);


// ****************************************************************************
// start
//
// ****************************************************************************
bool PatchedAppleNVRAM::start(IOService *provider)
{
  IOItemCount numRanges;
  IOMemoryMap *map;
  
  numRanges = provider->getDeviceMemoryCount();
  
  if (numRanges == 1) {
 
    _nvramType = kNVRAMTypeIOMem;

    // Get the address of the data register.
    map = provider->mapDeviceMemoryWithIndex(0);
    if (map == 0) return false;
	_nvramData = (UInt8 *)map->getVirtualAddress();
	
  } else if (numRanges == 2) {

	_nvramType = kNVRAMTypePort;
    
     // Get the address of the port register.
     map = provider->mapDeviceMemoryWithIndex(0);
     if (map == 0) return false;
     _nvramPort = (UInt8 *)map->getVirtualAddress();
    
    // Get the address of the data register.
    map = provider->mapDeviceMemoryWithIndex(1);
    if (map == 0) return false;
	
    _nvramData = (UInt8 *)map->getVirtualAddress();
  
  } else {
	return false;
  }
  
  return super::start(provider);
}

// ****************************************************************************
// read
//
// Read data from the NVRAM and return it in buffer.
//
// ****************************************************************************
IOReturn PatchedAppleNVRAM::read(IOByteCount offset, UInt8 *buffer,
			  IOByteCount length)
{
  UInt32 cnt;
  
  if ((buffer == 0) || (length <= 0) || (offset < 0) ||
      (offset + length > kNVRAMImageSize)) return kIOReturnBadArgument;
  
  switch (_nvramType) {
  case kNVRAMTypeIOMem :
    for (cnt = 0; cnt < length; cnt++) {
      buffer[cnt] = _nvramData[(offset + cnt)  << 4];
	  eieio();
    }
    break;
    
  case kNVRAMTypePort:
    for (cnt = 0; cnt < length; cnt++) {
      *_nvramPort = (offset + cnt) >> 5;
      eieio();
      buffer[cnt] = _nvramData[((offset + cnt) & 0x1F) << 4];
	  eieio();
    }
    break;
    
  default :
    return kIOReturnNotReady;
  }
  
  return kIOReturnSuccess;
}


// ****************************************************************************
// write
//
// Write data from buffer into NVRAM.
//
// ****************************************************************************
IOReturn PatchedAppleNVRAM::write(IOByteCount offset, UInt8 *buffer,
			   IOByteCount length)
{
  UInt32 cnt;
  
  if ((buffer == 0) || (length <= 0) || (offset < 0) ||
      (offset + length > kNVRAMImageSize))
    return kIOReturnBadArgument;
  
  switch (_nvramType) {
  case kNVRAMTypeIOMem :
    for (cnt = 0; cnt < length; cnt++) {
      _nvramData[(offset + cnt)  << 4] = buffer[cnt];
      eieio();
    }
    break;
    
  case kNVRAMTypePort:
    for (cnt = 0; cnt < length; cnt++) {
      *_nvramPort = (offset + cnt) >> 5;
      eieio();
      _nvramData[((offset + cnt) & 0x1F) << 4] = buffer[cnt];
      eieio();
    }
    break;
    
  default :
    return kIOReturnNotReady;
  }
  
  return kIOReturnSuccess;
}
