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
// This is a library extension to convert Old World NVRAM structures to and from New World structures.
// Originally, I had put this in PatchedAppleNVRAM, but I realized that the PMU NVRAM driver would
// need it to. So now it is a "library" extension.
//
// The reason why we want to emulate the New World NVRAM is that we want to pretend to be a New World
// machine, mostly to avoid a bug in the Mac OS X Installer, which is overly zealous about enforcing an 
// 8 GB limit on the target volume on Old World machines (the 8 GB limit is a real problem in some
// cases on Old World machines, but the Installer applies the limit too broadly). Unfortunately, bypassing
// the Installer's check does allow people to shoot themselves in the foot a bit, but it's better than not
// permitting an install at all (which is the effect of the Installer's bug in Tiger -- it was slightly
// less problematic pre-Tiger). 
//
// I'm using the first 4K of the Old World NVRAM (the "OS Partition") for three things.
//
// 1. For strings which don't fit in the Old World "Open Firmware" partition.
//
// 2. For "New World" NVRAM partitions that don't translated to Old World NVRAM.
//    (For instance, the "panic" partition).
//
// 3. For "New World" Open Firmware variables that don't exist on Old World
//
// The 4K starts with "New World"-style NVRAM partitions. I put the "strings that don't
// fit" at the end of the 4K (outside any partition).
//
// Ryan Rempel (ryan.rempel@utoronto.ca)
// -----------------------------------------------------------------------------------------------

#include "OpenOldWorldNVRAM.h"

#include <IOKit/IOLib.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/platform/ApplePlatformExpert.h>
#include <IOKit/IONVRAM.h>

#define DEBUG_NVRAM 0

#define super OSObject

OSDefineMetaClassAndStructors(OpenOldWorldNVRAM, OSObject);

const UInt32 kNWOFPartitionSizeInBytes = 4096;

const UInt32 kOWOFPartitionSize = 2048;
const UInt32 kOWOFPartitionOffset = 0x1800;
const UInt32 kOWXPRAMPartitionOffset = 0x1300;
const UInt32 kOWOSPartitionSize = 4096;
const UInt32 kOWOSPartitionOffset = 0;

const UInt32 kXPRAMPartitionSize = 0x100;
const UInt32 kNameRegistryPartitionSize = 0x400;

const UInt8 kOFPartitionSignature = 0x70;
const UInt8 kXPRAMPartitionSignature = 0xA0;
const UInt8 kFreePartitionSignature = 0x7F;

struct OWVariablesHeader {
	UInt16   owMagic;
	UInt8    owVersion;
	UInt8    owPages;
	UInt16   owChecksum;
	UInt16   owHere;
	UInt16   owTop;
	UInt16   owNext;
	UInt32   owFlags;
	UInt32   owNumbers[9];
	struct {
		UInt16 offset;
		UInt16 length;
	}        owStrings[10];
};

#if DEBUG_NVRAM
	static void debugDumpBuffer (char *name, UInt8* buffer, UInt32 offset = 0);
#endif

enum {
	kOWVariableOffsetNumber = 8,
	kOWVariableOffsetString = 17,
	kUsePropName = 255,
};

struct OFVariable {
	char		*variableName;
	UInt32     variableType;
	UInt32     variablePerm;
	SInt32     variableOffset;
};

OFVariable gOldWorldOFVariables[] = {
	{"little-endian?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 0},
	{"real-mode?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 1},
	{"auto-boot?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 2},
	{"diag-switch?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 3},
	{"fcode-debug?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 4},
	{"oem-banner?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 5},
	{"oem-logo?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 6},
	{"use-nvramrc?", kOFVariableTypeBoolean, kOFVariablePermUserRead, 7},
	{"real-base", kOFVariableTypeNumber, kOFVariablePermUserRead, 8},
	{"real-size", kOFVariableTypeNumber, kOFVariablePermUserRead, 9},
	{"virt-base", kOFVariableTypeNumber, kOFVariablePermUserRead, 10},
	{"virt-size", kOFVariableTypeNumber, kOFVariablePermUserRead, 11},
	{"load-base", kOFVariableTypeNumber, kOFVariablePermUserRead, 12},
	{"pci-probe-list", kOFVariableTypeNumber, kOFVariablePermUserRead, 13},
	{"screen-#columns", kOFVariableTypeNumber, kOFVariablePermUserRead, 14},
	{"screen-#rows", kOFVariableTypeNumber, kOFVariablePermUserRead, 15},
	{"selftest-#megs", kOFVariableTypeNumber, kOFVariablePermUserRead, 16},
	{"boot-device", kOFVariableTypeString, kOFVariablePermUserRead, 17},
	{"boot-file", kOFVariableTypeString, kOFVariablePermUserRead, 18},
	{"diag-device", kOFVariableTypeString, kOFVariablePermUserRead, 19},
	{"diag-file", kOFVariableTypeString, kOFVariablePermUserRead, 20},
	{"input-device", kOFVariableTypeString, kOFVariablePermUserRead, 21},
	{"output-device", kOFVariableTypeString, kOFVariablePermUserRead, 22},
	{"oem-banner", kOFVariableTypeString, kOFVariablePermUserRead, 23},
	{"oem-logo", kOFVariableTypeString, kOFVariablePermUserRead, 24},
	{"nvramrc", kOFVariableTypeString, kOFVariablePermUserRead, 25},
	{"boot-command", kOFVariableTypeString, kOFVariablePermUserRead, 26},
	{0, kOFVariableTypeData, kOFVariablePermUserRead, -1}
};

struct NVRAMPartition {
	UInt8 signature;
	UInt8 checksum;
	UInt16 lengthIn16ByteUnits;
	char partitionName[12];
	char data[];
};

OpenOldWorldNVRAM *
OpenOldWorldNVRAM::withBuffers (char *fOldWorldBuffer, char *fNewWorldBuffer)
{
	OpenOldWorldNVRAM *nvram = new OpenOldWorldNVRAM;
	nvram->fOldWorldBuffer = fOldWorldBuffer;
	nvram->fNewWorldBuffer = fNewWorldBuffer;
	
	return nvram;
}

void
OpenOldWorldNVRAM::copyOldWorldToNewWorld ()
{	
	// The idea here is that fOldWorldBuffer has been filled with the actual contents
	// of NVRAM. We are now going to copy that to fNewWorldBuffer, using the New World
	// NVRAM structures.

	// First, we'll create a New World partition to hold the Open Firmware variables
	// The length on my B&W G3 is 4K, so we'll go with that (we may want less later as 
	// we balance things out a bit).
	
#if DEBUG_NVRAM
	debugDumpBuffer ("fOldWorldBuffer before conversion", fOldWorldBuffer);
#endif

	fOWHeader = (OWVariablesHeader *) (fOldWorldBuffer + kOWOFPartitionOffset);
	
	NVRAMPartition *ofPartition = (NVRAMPartition *) fNewWorldBuffer;
	ofPartition->signature = kOFPartitionSignature;
	ofPartition->checksum = 0;
	ofPartition->lengthIn16ByteUnits = kNWOFPartitionSizeInBytes / 16;
	strncpy (ofPartition->partitionName, kIODTNVRAMOFPartitionName, sizeof (ofPartition->partitionName));
	bzero (ofPartition->data, (ofPartition->lengthIn16ByteUnits - 1) * 16);
			
	// Now, we cycle through the Old World Open Firmware and create the New World properties
	// I'm adapting methods from IODTNVRAM for this purpose
	char *ofdata = ofPartition->data;
    UInt32 cnt = 0;
	
    while (true) {
		UInt32 propOffset, propType, propLength;
		char *propName, *propData;

		if (!getOWVariableInfo (cnt++, &propName, &propType, &propOffset)) break;
		
		if (ofdata >= ofPartition->data + kNWOFPartitionSizeInBytes) {
			IOLog ("OpenOldWorldNVRAM: Open Firmware data from Old World too large for New World Partition\n");
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: Open Firmware data from Old World too large for New World Partition\n");
			#endif
			break;
		}
      	  
		switch (propType) {
			case kOFVariableTypeBoolean:
				sprintf (ofdata, "%s=%s", propName, fOWHeader->owFlags & propOffset ? "true" : "false");
				break;
	
			case kOFVariableTypeNumber:
				if (fOWHeader->owNumbers[propOffset] == 0xFFFFFFFF) {
					sprintf (ofdata, "%s=-1", propName);
				} else if (fOWHeader->owNumbers[propOffset] < 1000) {
					sprintf (ofdata, "%s=%ld", propName, fOWHeader->owNumbers[propOffset]);
				} else {
					sprintf (ofdata, "%s=0x%lx", propName, fOWHeader->owNumbers[propOffset]);
				}
				break;
	
			case kOFVariableTypeString:
				propData = (char *) fOldWorldBuffer + fOWHeader->owStrings[propOffset].offset;
				propLength = fOWHeader->owStrings[propOffset].length;
				
				if (!strcmp (propName, "boot-command")) {
					if (propLength >= strlen ("0 bootr ") && !strncmp (propData, "0 bootr ", strlen ("0 bootr "))) {
						// If the boot-command starts with 0 bootr, then we fill in the boot-args as well
						sprintf (ofdata, "boot-command=0 bootr");
						#if DEBUG_NVRAM
							kprintf ("Set %s\n", ofdata);
						#endif
						ofdata += strlen (ofdata) + 1;
						
						propName = "boot-args";
						propData += strlen ("0 bootr ");
						propLength -= strlen ("0 bootr ");
					} else {
						// If the boot-command does not start with 0 bootr, then we initialize boot-args to ""
						strcpy (ofdata, "boot-args=");
						#if DEBUG_NVRAM
							kprintf ("Set %s\n", ofdata);
						#endif
						ofdata += strlen (ofdata) + 1;
					}
				}

				sprintf (ofdata, "%s=", propName);
				strncat (ofdata, propData, propLength); 
				break;
		}
		
		if (*ofdata) {
			#if DEBUG_NVRAM
				kprintf ("Set %s\n", ofdata);
			#endif
			ofdata += strlen (ofdata) + 1;
		}
	}
	
	// Then we fill in the partition for the XPRAM / Name Registry
	NVRAMPartition *xpramPartition = (NVRAMPartition *) (((UInt8 *) ofPartition) + ofPartition->lengthIn16ByteUnits * 16);
	xpramPartition->signature = 0xA0;
	xpramPartition->checksum = 0;
	xpramPartition->lengthIn16ByteUnits = ((kXPRAMPartitionSize + kNameRegistryPartitionSize) / 16) + 1;
	strncpy (xpramPartition->partitionName, kIODTNVRAMXPRAMPartitionName, sizeof (xpramPartition->partitionName));
	
	// We can just copy the XPRAM from Old World, as the format is the same (or, at least, Mac OS X thinks it is)
	// We'll pick up the name registry stuff later
	bcopy (fOldWorldBuffer + kOWXPRAMPartitionOffset, xpramPartition->data, kXPRAMPartitionSize);

	// Now we cycle through any "New World" partitions that we've stuffed into the first 4K of the Old World space
	// We could conceivably use compression / decompression to deal with size issues, given that OF isn't going 
	// to be interpreting this stuff anyway.
	UInt8 *nwPartition = ((UInt8 *) xpramPartition) + xpramPartition->lengthIn16ByteUnits * 16;
	NVRAMPartition *owPartition = (NVRAMPartition *) fOldWorldBuffer;
	
	while ((UInt32) owPartition < (UInt32) fOldWorldBuffer + kOWOSPartitionSize) {		
		UInt32 partitionSizeInBytes = owPartition->lengthIn16ByteUnits * 16;
		if (partitionSizeInBytes == 0) break;

		if ((UInt32) owPartition + partitionSizeInBytes > (UInt32) fOldWorldBuffer + kOWOSPartitionSize) {
			IOLog ("OpenOldWorldNVRAM: Saved New World NVRAM partition at %lu is too long\n", (UInt32) owPartition - (UInt32) fOldWorldBuffer);
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: Saved New World NVRAM partition at %lu is too long\n", (UInt32) owPartition - (UInt32) fOldWorldBuffer);
			#endif
			break;
		}
		
		UInt8 checksum = owPartition->checksum;
		owPartition->checksum = 0;
		if (checksum != calculatePartitionChecksum ((UInt8*) owPartition)) {
			IOLog ("OpenOldWorldNVRAM: New World NVRAM partition checksum at %lu not correct\n", (UInt32) owPartition - (UInt32) fOldWorldBuffer);
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: New World NVRAM partition checksum at %lu not correct\n", (UInt32) owPartition - (UInt32) fOldWorldBuffer);
			#endif
			break;
		}
		owPartition->checksum = checksum;
		
		if (!strncmp (owPartition->partitionName, kIODTNVRAMOFPartitionName, sizeof (owPartition->partitionName))) {
			// This would be the partition where we've saved the "extra" OF variable that don't apply to Old World.
			// So we simply copy it to the end of what we've already parsed.
			// It's possible that we could just ignore these (i.e. not save and restore them).
			if ((UInt32) ofdata + partitionSizeInBytes - 16 > (UInt32) ofPartition + ofPartition->lengthIn16ByteUnits * 16) {
				IOLog ("OpenOldWorldNVRAM: Saved OF data is too large for OF partition\n");
				#if DEBUG_NVRAM
					kprintf ("OpenOldWorldNVRAM: Saved OF data is too large for OF partition\n");
				#endif
			} else {
				bcopy (owPartition->data, ofdata, partitionSizeInBytes - 16);
			}
		} else if (!strncmp (owPartition->partitionName, kIODTNVRAMXPRAMPartitionName, sizeof (owPartition->partitionName))) {
			// This would be the partition where we have saved the Name Registry data. Ideally, we would convert back and forth
			// but I'm being lazy for the moment and just restoring the "New World" version, such as it is.
			if (partitionSizeInBytes - 16 != kNameRegistryPartitionSize) {
				IOLog ("OpenOldWorldNVRAM: Saved Name Registry data is the wrong size\n");
				#if DEBUG_NVRAM
					IOLog ("OpenOldWorldNVRAM: Saved Name Registry data is the wrong size\n");
				#endif
			} else {
				bcopy (owPartition->data, xpramPartition->data + kXPRAMPartitionSize, kNameRegistryPartitionSize);
			}
		} else {
			// If it's not one of the two special partitions, then just copy it over if there is space.			
			if ((UInt32) nwPartition + partitionSizeInBytes > (UInt32) fNewWorldBuffer + kNVRAMImageSize) {
				IOLog ("OpenOldWorldNVRAM: Saved New World NVRAM partition %.12s will not fit\n", owPartition->partitionName);
				#if DEBUG_NVRAM
					kprintf ("OpenOldWorldNVRAM: Saved New World NVRAM partition at %lu will not fit\n", (UInt32) owPartition - (UInt32) fOldWorldBuffer);
				#endif
			} else {							
				bcopy (owPartition, nwPartition, partitionSizeInBytes);		
				nwPartition += partitionSizeInBytes;
			}
		}
		
		owPartition = (NVRAMPartition *) (((UInt8*) owPartition) + partitionSizeInBytes);
	}
	
	// Now, we need to build the "free" partition with whatever's left.
	if ((UInt32) nwPartition + 32 < (UInt32) fNewWorldBuffer + kNVRAMImageSize) {
		NVRAMPartition *freePartition = (NVRAMPartition *) nwPartition;
		freePartition->signature = kFreePartitionSignature;
		strncpy (freePartition->partitionName, kIODTNVRAMFreePartitionName, sizeof (freePartition->partitionName));
		freePartition->lengthIn16ByteUnits = ((UInt32) fNewWorldBuffer + kNVRAMImageSize - (UInt32) nwPartition) / 16;
		freePartition->checksum = 0;
		freePartition->checksum = calculatePartitionChecksum ((UInt8 *) freePartition);
	}
	
	// Caculate the other checksums	
	ofPartition->checksum = 0;
	ofPartition->checksum = calculatePartitionChecksum ((UInt8 *) ofPartition);
	
	xpramPartition->checksum = 0;
	xpramPartition->checksum = calculatePartitionChecksum ((UInt8 *) xpramPartition);
	
#if DEBUG_NVRAM
	debugDumpBuffer ("fNewWorldBuffer after conversion", fNewWorldBuffer);
#endif
}

void
OpenOldWorldNVRAM::copyNewWorldToOldWorld ()
{
	// The idea here is that Mac OS X has written to the New World buffer. We now need to convert this
	// to the Old World NVRAM structures before actually writing the NVRAM from the Old World buffer.
	NVRAMPartition *nwPartition = (NVRAMPartition *) fNewWorldBuffer;
	NVRAMPartition *owPartition = (NVRAMPartition *) fOldWorldBuffer;

	fOWHeader = (OWVariablesHeader *) (fOldWorldBuffer + kOWOFPartitionOffset);
	fOSPartitionTop = (char *) fOldWorldBuffer + kOWOSPartitionSize;
	
	#if DEBUG_NVRAM
		// debugDumpBuffer ("fNewWorldBuffer before conversion", fNewWorldBuffer);
	#endif

	// We cycle through the NVRAM partitions and deal with them as needed.	
	while ((UInt32) nwPartition < (UInt32) fNewWorldBuffer + kNVRAMImageSize) {
		UInt32 partitionSizeInBytes = nwPartition->lengthIn16ByteUnits * 16;
		if (partitionSizeInBytes == 0) break;
		
		#if DEBUG_NVRAM
			kprintf ("NW Partition at: %lu size: %lu\n", (UInt32) nwPartition - (UInt32) fNewWorldBuffer, partitionSizeInBytes);
		#endif

		if ((UInt32) nwPartition + partitionSizeInBytes > (UInt32) fNewWorldBuffer + kNVRAMImageSize) {
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: New World NVRAM partition is too long\n");
			#endif
			break;
		}
		
		UInt8 checksum = nwPartition->checksum;
		nwPartition->checksum = 0;
		if (checksum != calculatePartitionChecksum ((UInt8*) nwPartition)) {
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: New World NVRAM partition checksum not correct\n");
			#endif
			break;
		}
		nwPartition->checksum = checksum;
		
		if (!strncmp (nwPartition->partitionName, kIODTNVRAMOFPartitionName, sizeof (nwPartition->partitionName))) {
			#if DEBUG_NVRAM
				kprintf ("OpenOldWorldNVRAM: found OF Partition\n");
			#endif
			
			// Here's where we parse the OF variables and write them out
			// We create a partition to hold OF variables that don't exist on Old World
			owPartition->signature = kOFPartitionSignature;
			strncpy (owPartition->partitionName, kIODTNVRAMOFPartitionName, sizeof (owPartition->partitionName));
			
			// Initialize some OW header values
			fOWHeader->owHere = (UInt32) fOWHeader + sizeof (fOWHeader) - (UInt32) fOldWorldBuffer;
			fOWHeader->owTop = kOWOFPartitionOffset + kOWOFPartitionSize;
			fOWHeader->owNext = 0;

			// The "New World" only variables get written out in New World format at fOSPartitionHere
			// The "Old World" overflow variables get written out at fOSPartitionTop (backwards)
			fOSPartitionHere = owPartition->data;
			
			char *bootArgs = NULL;
			char *bootCommand = NULL;

			char *propName = nwPartition->data;
						
			while ((UInt32) propName < (UInt32) nwPartition + partitionSizeInBytes) {
				UInt32 propType, propOffset;

				char *propData = propName;				
				while (*propData != 0) {
					if (*propData == '=') break;
					propData++;
				}
				
				if (*propData == 0) break;
				if ((UInt32) propData > (UInt32) nwPartition + partitionSizeInBytes) break;
				if ((UInt32) propData + strlen (propData) > (UInt32) nwPartition + partitionSizeInBytes) break;
				*propData = 0;
				propData++;
				
				#if DEBUG_NVRAM
					kprintf ("propName: %s propData: %s\n", propName, propData);
				#endif

				if (!strcmp (propName, "boot-args")) {
					// Save boot-args for later, we'll combine with boot-command
					bootArgs = (char *) IOMalloc (strlen (propData) + 1);
					strcpy (bootArgs, propData);
					
				} else if (!strcmp (propName, "boot-command")) {
					// Save boot-command for later, we'll combine with boot-args
					bootCommand = (char *) IOMalloc (strlen (propData) + 1);
					strcpy (bootCommand, propData);
					
				} else if (getOWVariableInfo (kUsePropName, &propName, &propType, &propOffset)) {
					// If it's one of the standard OW variables, then deal with it
					switch (propType) {
						case kOFVariableTypeBoolean:
							if (!strcmp (propData, "true")) {
								fOWHeader->owFlags |= propOffset;
							} else {
								fOWHeader->owFlags &= ~propOffset;
							}
							break;
						
						case kOFVariableTypeNumber:
							fOWHeader->owNumbers[propOffset] = strtol (propData, NULL, 0);
							break;
						
						case kOFVariableTypeString:
							storeOWString (propName, propData, propOffset);
							break;
					}
					
				} else {
					// If it's not one of the standard OW variables, then just copy it so we can restore it later
					if ((UInt32) fOSPartitionHere + strlen (propName) + strlen (propData) + 2 > (UInt32) fOSPartitionTop) {
						kprintf ("OpenOldWorldNVRAM: Not enough space to store New World OF variables\n");
					} else {
						sprintf (fOSPartitionHere, "%s=%s", propName, propData);
						fOSPartitionHere += strlen (fOSPartitionHere) + 1;
					}
				}
				
				propName = propData + strlen (propData) + 1;
			}
			
			#if DEBUG_NVRAM
				if (bootCommand) kprintf ("bootCommand: %s\n", bootCommand);
				if (bootArgs) kprintf ("bootArgs: %s\n", bootArgs);
			#endif
			
			// Do the special processing for boot-command and boot-args
			if (bootCommand) {
				if (!strcmp (bootCommand, "0 bootr") || !strcmp (bootCommand, "mac-boot")) {
					if (bootArgs && bootArgs[0]) {
						char *realBootCommand = (char *) IOMalloc (strlen ("0 bootr ") + strlen (bootArgs) + 1);
						sprintf (realBootCommand, "0 bootr %s", bootArgs);
						storeOWString ("boot-command", realBootCommand, 9);
						IOFree (realBootCommand, strlen (realBootCommand) + 1);
					} else {
						storeOWString ("boot-command", "0 bootr", 9);
					}
				} else {
					storeOWString ("boot-command", bootCommand, 9);
				}
			}
			
			if (bootCommand) IOFree (bootCommand, strlen (bootCommand) + 1);
			if (bootArgs) IOFree (bootArgs, strlen (bootArgs) + 1);
			
			owPartition->lengthIn16ByteUnits = ((UInt32) fOSPartitionHere + 15 - (UInt32) owPartition) / 16;
			owPartition->checksum = 0;
			owPartition->checksum = calculatePartitionChecksum ((UInt8*) owPartition);
			owPartition = (NVRAMPartition *) (((UInt8*) owPartition) + owPartition->lengthIn16ByteUnits * 16);
			
			// Update OW checksum
			fOWHeader->owChecksum = 0;
			fOWHeader->owChecksum = ~generateOWChecksum ((UInt8*) fOWHeader);
			
		} else if (!strncmp (nwPartition->partitionName, kIODTNVRAMXPRAMPartitionName, sizeof (nwPartition->partitionName))) {
			if (partitionSizeInBytes - 16 != kNameRegistryPartitionSize + kXPRAMPartitionSize) {
				#ifdef DEBUG_NVRAM
					kprintf ("OpenOldWorldNVRAM: XPRAM/NameRegistry Partition is wrong size\n");
				#endif
			} else {
				// We can just copy the XPRAM data over.
				bcopy (nwPartition->data, fOldWorldBuffer + kOWXPRAMPartitionOffset, kXPRAMPartitionSize);
				// Ideally, we would convert the Name Registry data to the Old World format, but I'm being lazy for
				// the moment and just writing it out so we can restore it later.
				if ((UInt32) owPartition + kNameRegistryPartitionSize + 16 > (UInt32) fOSPartitionTop) {
					#ifdef DEBUG_NVRAM
						kprintf ("OpenOldWorldNVRAM: Not enough room to save Name Registry data\n");
					#endif
				} else {
					owPartition->signature = kXPRAMPartitionSignature;
					strncpy (owPartition->partitionName, kIODTNVRAMXPRAMPartitionName, sizeof (owPartition->partitionName));
					owPartition->lengthIn16ByteUnits = (kNameRegistryPartitionSize / 16) + 1;
					bcopy (nwPartition->data + kXPRAMPartitionSize, owPartition->data, kNameRegistryPartitionSize);
					owPartition->checksum = 0;
					owPartition->checksum = calculatePartitionChecksum ((UInt8*) owPartition);
					owPartition = (NVRAMPartition*) (((UInt8*) owPartition) + owPartition->lengthIn16ByteUnits * 16);
				}
			}
			
		} else if (!strncmp (nwPartition->partitionName, kIODTNVRAMFreePartitionName, sizeof (nwPartition->partitionName))) {
			// We can just ignore the free partition, I think
			
		} else {
			// If it's not one of the special partitions, then just copy it over if there is space.		
			if ((UInt32) owPartition + partitionSizeInBytes > (UInt32) fOSPartitionTop) {
				#ifdef DEBUG_NVRAM
					kprintf ("OpenOldWorldNVRAM: Not enough room to save New World partition\n");
				#endif
			} else {
				bcopy (nwPartition, owPartition, partitionSizeInBytes);
				owPartition = (NVRAMPartition *) (((UInt8*) owPartition) + partitionSizeInBytes);
			}
		}
		
		nwPartition = (NVRAMPartition *) (((UInt8*) nwPartition) + partitionSizeInBytes);
	}
	
	// Zero out the rest of the first 4K
	if ((UInt32) owPartition < (UInt32) fOSPartitionTop) {
		bzero (owPartition, (UInt32) fOSPartitionTop - (UInt32) owPartition);
	}

#if DEBUG_NVRAM
	debugDumpBuffer ("fOldWorldBuffer after conversion", fOldWorldBuffer, (UInt32) fOWHeader - (UInt32) fOldWorldBuffer);
#endif
}

UInt8 
OpenOldWorldNVRAM::calculatePartitionChecksum (UInt8 *partitionHeader)
{
	UInt8 cnt, isum, csum = 0;

	for (cnt = 0; cnt < 0x10; cnt++) {
		isum = csum + partitionHeader[cnt];
		if (isum < csum) isum++;
		csum = isum;
	}

	return csum;
}

UInt16
OpenOldWorldNVRAM::generateOWChecksum (UInt8 *buffer)
{
	UInt32 cnt, checksum = 0;
	UInt16 *tmpBuffer = (UInt16 *) buffer;

	for (cnt = 0; cnt < kOWOFPartitionSize / 2; cnt++) {
		checksum += tmpBuffer[cnt];
	}

	return checksum % 0x0000FFFF;
}

bool 
OpenOldWorldNVRAM::validateOWChecksum (UInt8 *buffer)
{
	UInt32 cnt, checksum, sum = 0;
	UInt16 *tmpBuffer = (UInt16 *) buffer;

	for (cnt = 0; cnt < kOWOFPartitionSize / 2; cnt++) {
		sum += tmpBuffer[cnt];
	}
	
	checksum = (sum >> 16) + (sum & 0x0000FFFF);
	if (checksum == 0x10000) checksum--;
	checksum = (checksum ^ 0x0000FFFF) & 0x0000FFFF;

	return checksum == 0;
}

bool 
OpenOldWorldNVRAM::getOWVariableInfo (UInt32 variableNumber, char **propName, UInt32 *propType, UInt32 *propOffset)
{
	OFVariable *ofVar;
  
	ofVar = gOldWorldOFVariables;
	while (1) {
		if (ofVar->variableName == 0) return false;
		if (variableNumber == kUsePropName) {
			if (!strcmp (*propName, ofVar->variableName)) break;
		} else {
			if (ofVar->variableOffset == (SInt32) variableNumber) break;
		}
		ofVar++;
	}
	
	if (variableNumber != kUsePropName) *propName = ofVar->variableName;
	*propType = ofVar->variableType;
  
	switch (*propType) {
		case kOFVariableTypeBoolean :
			*propOffset = 1 << (31 - ofVar->variableOffset);
			break;

		case kOFVariableTypeNumber :
			*propOffset = ofVar->variableOffset - kOWVariableOffsetNumber;
			break;

		case kOFVariableTypeString :
			*propOffset = ofVar->variableOffset - kOWVariableOffsetString;
			break;
	}

	return true;
}

void
OpenOldWorldNVRAM::storeOWString (char *propName, char *propData, UInt32 propOffset) 
{
	#if DEBUG_NVRAM
		kprintf ("Store propName: %s propOffset: %lu propData: %s\n", propName, propOffset, propData);
	#endif
	
	if (fOWHeader->owTop - strlen (propData) >= fOWHeader->owHere) {
		fOWHeader->owTop -= strlen (propData);
		fOWHeader->owStrings[propOffset].offset = fOWHeader->owTop;
		fOWHeader->owStrings[propOffset].length = strlen (propData);
		bcopy (propData, fOldWorldBuffer + fOWHeader->owTop, strlen (propData));
	} else if (fOSPartitionTop - strlen (propData) >= fOSPartitionHere) {
		fOSPartitionTop -= strlen (propData);
		fOWHeader->owStrings[propOffset].offset = (UInt32) fOSPartitionTop - (UInt32) fOldWorldBuffer;
		fOWHeader->owStrings[propOffset].length = strlen (propData);
		bcopy (propData, fOSPartitionTop, strlen (propData));
	} else {
		#if DEBUG_NVRAM
			kprintf ("OpenOldWorldNVRAM: Not enough space to store OF variable: %s\n", propName);
		#endif
	}
}

#if DEBUG_NVRAM

static void
debugDumpBuffer (char *name, UInt8* buffer, UInt32 offset) 
{
	char str[17];
	str[16] = 0;
	
	kprintf ("---------------\n%s\n---------------\n\n", name);
	
	for (UInt32 *c = (UInt32 *) (buffer + offset); (UInt32) c < (UInt32) buffer + kNVRAMImageSize; c += 4) {
		char *d = (char *) c;
		for (int x = 0; x < 16; x++) {
			str[x] = ((d[x] >=32) && (d[x] < 127)) ? d[x] : ((d[x] == 0) ? '|' : '_');
		}
		kprintf ("%8lu : %08X %08X %08X %08X : %s\n", (UInt32) c - (UInt32) buffer, c[0], c[1], c[2], c[3], str);
	}
	
	kprintf ("\n------------------\n");
}

#endif