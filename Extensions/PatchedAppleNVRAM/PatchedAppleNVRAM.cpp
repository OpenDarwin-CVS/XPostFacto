/*

 Copyright (c) 2005
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

/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as defined 
in and that are subject to the Apple Public Source License Version 2.0 (the 'License').  
You may not use this file except in compliance with the License.  Please obtain a copy 
of the License at http://www.opensource.apple.com/apsl/ and read it before using this file. 

The Original Code and all software distributed under the License are distributed on an 
'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND APPLE HEREBY 
DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see the License 
for the specific language governing rights and limitations under the License.

*/

// -----------------------------------------------------------------------------------------------
// This is a replacement for AppleNVRAM, to work around a bug on Old World machines.
//
// It also emulates the New World NVRAM structures on Old World machines.
// Ideally speaking, this is not the correct place to be dealing with NVRAM *structure* -- 
// this class is only really supposed to be responsible for reading and writing the NVRAM values,
// and is not concerned about structure or content. Ideally, one would use the IODTNVRAM class to
// deal with issues of structure and content. However, for a variety of reasons it is very awkward
// to replace IODTNVRAM with a subclass. It's a long story, but the interaction between IODTNVRAM and
// IOPlatformExpert, given what is public and private, and what is virtual and non-virtual, makes it
// very hard to effectively sublcass. There is a scheme which might work (it involves altering at least
// one header), but even in that case the changes in virtual functions in IODTNVRAM would mean that 
// serveral different versions would be required.
// 
// The nice thing about handling the content issues here is that it's easy to subclass here, and once
// we deal with it here we don't need to touch anything else.
//
// The reason why we want to emulate the New World NVRAM is that we want to pretend to be a New World
// machine, mostly to avoid a bug in the Mac OS X Installer, which is overly zealous about enforcing an 
// 8 GB limit on the target volume on Old World machines (the 8 GB limit is a real problem in some
// cases on Old World machines, but the Installer applies the limit too broadly). Unfortunately, bypassing
// the Installer's check does allow people to shoot themselves in the foot a bit, but it's better than not
// permitting an install at all (which is the effect of the Installer's bug in Tiger -- it was slightly
// less problematic pre-Tiger). 
//
// Ryan Rempel (ryan.rempel@utoronto.ca)
// -----------------------------------------------------------------------------------------------

#include "PatchedAppleNVRAM.h"
#include "../OpenOldWorldNVRAM/OpenOldWorldNVRAM.h"
#include <../Headers/IOPlatformExpert.h>

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/platform/ApplePlatformExpert.h>

#define super IONVRAMController

OSDefineMetaClassAndStructors(PatchedAppleNVRAM, IONVRAMController);

bool 
PatchedAppleNVRAM::start (IOService *provider)
{
	IOItemCount numRanges;
	IOMemoryMap *map;
  
	numRanges = provider->getDeviceMemoryCount();
  
	if (numRanges == 1) {
 
		fNVRAMType = kNVRAMTypeIOMem;

		// Get the address of the data register.
		map = provider->mapDeviceMemoryWithIndex (0);
		if (map == 0) return false;
		fNVRAMData = (UInt8 *) map->getVirtualAddress ();
	
	} else if (numRanges == 2) {

		fNVRAMType = kNVRAMTypePort;

		// Get the address of the port register.
		map = provider->mapDeviceMemoryWithIndex (0);
		if (map == 0) return false;
		fNVRAMPort = (UInt8 *) map->getVirtualAddress ();

		// Get the address of the data register.
		map = provider->mapDeviceMemoryWithIndex (1);
		if (map == 0) return false;
		fNVRAMData = (UInt8 *) map->getVirtualAddress ();

	} else {	
		return false;
	}
	
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

void 
PatchedAppleNVRAM::free ()
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

IOReturn
PatchedAppleNVRAM::read (IOByteCount offset, UInt8 *buffer, IOByteCount length)
{
	UInt32 cnt;
	
	IOByteCount callersOffset = offset;
	UInt8 *callersBuffer = buffer;
	IOByteCount callersLength = length;
	
	if ((buffer == 0) || (length <= 0) || (offset < 0) || (offset + length > kNVRAMImageSize)) return kIOReturnBadArgument;
	
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
	
	switch (fNVRAMType) {
		case kNVRAMTypeIOMem :
			for (cnt = 0; cnt < length; cnt++) {
				buffer[cnt] = fNVRAMData[(offset + cnt)  << 4];
				eieio();
			}
			break;
		
		case kNVRAMTypePort:
			for (cnt = 0; cnt < length; cnt++) {
				*fNVRAMPort = (offset + cnt) >> 5;
				eieio();
				buffer[cnt] = fNVRAMData[((offset + cnt) & 0x1F) << 4];
				eieio();
			}
			break;
	
		default :
			return kIOReturnNotReady;
			break;
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

IOReturn 
PatchedAppleNVRAM::write (IOByteCount offset, UInt8 *buffer, IOByteCount length)
{
	UInt32 cnt;
  
	if ((buffer == 0) || (length <= 0) || (offset < 0) || (offset + length > kNVRAMImageSize)) return kIOReturnBadArgument;
  
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
  
	switch (fNVRAMType) {
		case kNVRAMTypeIOMem :
			for (cnt = 0; cnt < length; cnt++) {
				fNVRAMData[(offset + cnt)  << 4] = buffer[cnt];
				eieio();
			}
			break;
    
		case kNVRAMTypePort:
			for (cnt = 0; cnt < length; cnt++) {
				*fNVRAMPort = (offset + cnt) >> 5;
				eieio();
				fNVRAMData[((offset + cnt) & 0x1F) << 4] = buffer[cnt];
				eieio();
			}
			break;
    
		default :
			return kIOReturnNotReady;
	}
  
	return kIOReturnSuccess;
}