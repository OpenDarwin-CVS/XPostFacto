/*

Copyright (c) 2002, 2005
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

/* 

Portions Copyright (c) 1999-2001 Apple Computer, Inc. All Rights Reserved.

This file contains Original Code and/or Modifications of Original Code 
as defined in and that are subject to the Apple Public Source License 
Version 1.2 (the 'License'). You may not use this file except in compliance 
with the License. Please obtain a copy of the License at 
http://www.apple.com/publicsource and read it before using this file.

The Original Code and all software distributed under the License are 
distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
the License for the specific language governing rights and limitations 
under the License.

*/

#include "XPFPlatform.h"
#include "XPFLog.h"
#include "NVRAM.h"
#include "XPFErrors.h"
#include <ctype.h>
#include <stdio.h>

#ifdef __MACH__
     #include <sys/types.h>
     #include <sys/sysctl.h>
#endif

XPFPlatform* XPFPlatform::gPlatform = NULL;

// Resource manager stuff

struct OFtc {
	char name[36];
	short id;
};

XPFPlatform*
XPFPlatform::GetPlatform ()
{
	if (!gPlatform) gPlatform = new XPFPlatform;
	return gPlatform;
}

XPFPlatform::XPFPlatform ()
{
	// Figure out whether we know how to patch this machine.
	fCompatible = NULL;
	getCompatibleFromDeviceTree (&fCompatible);
	gLogFile << "Compatible: " << fCompatible << endl_AC;

	fIsNewWorld = false;
	fEmulatingNewWorld = false;

#ifdef __MACH__	
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_service_t patchedAppleNVRAM = NULL; 
	io_iterator_t iter = NULL;
	
	IOServiceGetMatchingServices (iokitPort, IOServiceMatching ("PatchedAppleNVRAM"), &iter);
	if (iter) {
		patchedAppleNVRAM = IOIteratorNext (iter);
		IOObjectRelease (iter);
	}
	
	if (patchedAppleNVRAM) {
		CFTypeRef emulatingNewWorld = IORegistryEntryCreateCFProperty (patchedAppleNVRAM, CFSTR("EmulatingNewWorld"), NULL, 0);
		if (emulatingNewWorld) {
			fIsNewWorld = false;
			fEmulatingNewWorld = true;
			CFRelease (emulatingNewWorld);
		}
		IOObjectRelease (patchedAppleNVRAM);
	}

	if (!fEmulatingNewWorld) {
		UInt32 epoch;
		size_t epochlen = sizeof (epoch);
		int err = sysctlbyname ("hw.epoch", &epoch, &epochlen, NULL, NULL);
		if (err == noErr) fIsNewWorld = (epoch != 0);
	}
#else
	long machineType;
	Gestalt (gestaltMachineType, &machineType);
	fIsNewWorld = (machineType == gestaltPowerMacNewWorld);
#endif
	
	fNVRAMPatch = NULL;
	if (!fIsNewWorld) loadNVRAMPatch (fCompatible);

	gPlatform = this;
}

XPFPlatform::~XPFPlatform ()
{
	if (fCompatible) DisposePtr (fCompatible);
	if (fNVRAMPatch) DisposePtr (fNVRAMPatch);
}

void
XPFPlatform::patchNVRAM ()
{
	if (fIsNewWorld) return;

	XPFNVRAMSettings *settings = XPFNVRAMSettings::GetSettings ();
	
	settings->setBooleanValue ("use-nvramrc?", true);
	
	settings->setNumericValue ("real-base", -1);
	settings->setNumericValue ("real-size", 0x100000);
	settings->setNumericValue ("virt-base", -1);
	settings->setNumericValue ("virt-size", 0x100000);
	settings->setNumericValue ("load-base", 0x600000);
	settings->setNumericValue ("pci-probe-list", -1);
	settings->setNumericValue ("screen-#columns", 0x64);
	settings->setNumericValue ("screen-#rows", 0x28);
	
	settings->setStringValue ("nvramrc", fNVRAMPatch);
}

void
XPFPlatform::loadNVRAMPatch (char *compatible)
{
	if (fIsNewWorld) return;
	
	if (!strcmp (compatible, "AAPL,PowerBook1998")) {
		#ifdef __MACH__
			// No way to distinguish, so we'll just leave the current patch as is
			XPFNVRAMSettings *settings = XPFNVRAMSettings::GetSettings ();
			processOFVariable ("nvramrc", settings->getStringValue ("nvramrc"));
			return;
		#else
			long machineType;
			Gestalt (gestaltMachineType, &machineType);
			char typeString [32];
			sprintf (typeString, ",%u", machineType);
			strcat (compatible, typeString);
		#endif
	}
	Handle tc = GetResource ('OFtc', 128);
	ThrowIfNULL_AC (tc);
	HLock (tc);
	OFtc *type = (OFtc*) *tc;
	int numberOfEntries = GetResourceSizeOnDisk (tc) / sizeof (OFtc);
	int x;
	for (x = 0; x < numberOfEntries; x++) if (!strcmp (type[x].name, compatible)) break;
	if (x == numberOfEntries) {
		#if qLogging
			gLogFile << "Couldn't find entry for NVRAM patches" << endl_AC;
		#endif
	} else {
		// Now we need to get the actual resource we want
		Handle settings = GetResource ('OFpt', type[x].id);
		ThrowIfNULL_AC (settings);
		DetachResource (settings);
		
		CHandleStream_AC *settingsStream = new CHandleStream_AC (settings);
		parsePatchStream (settingsStream);
		delete settingsStream;
		
		DisposeHandle (settings);
	}
	HUnlock (tc);	
	ReleaseResource (tc);
}

void
XPFPlatform::processOFVariable (char *name, char *value) {
	if (!strcmp (name, "nvramrc")) {
		fNVRAMPatch = NewPtr (strlen (value) + 1);
		strcpy (fNVRAMPatch, value);
		char *c = fNVRAMPatch;
		while (*c) {
			if (*c == 10) *c = 13;
			c++;
		}	
	}
}

// States for ParseFile.
enum {
	kFirstColumn = 0,
	kScanComment,
	kFindName,
	kCollectName,
	kFindValue,
	kCollectValue,
	kContinueValue,
	kSetenv,

	kMaxStringSize = 0x800,
	kMaxNameSize = 0x100
};

// This is adapted from Darwin's nvram.c

void
XPFPlatform::parsePatchStream (CStream_AC *patches)
{
	long state, tc, ni = 0, vi = 0;
	char name[kMaxNameSize];
	char value[kMaxStringSize];
    
	state = kFirstColumn;
	while (true) {
		try {
			tc = patches->ReadCharacter ();
		}
		catch (...) {
			break;
		}
		if (vi >= kMaxStringSize) ThrowException_AC (kNVRAMPatchParseError, 0);
	    switch (state) {
			case kFirstColumn :
				ni = 0;
				vi = 0;
				if (tc == '#') {
					state = kScanComment;
			 	} else if ((tc == '\n') || (tc == '\r')) {
					// state stays kFirstColumn.
			  	} else if (isspace(tc)) {
					state = kFindName;
			  	} else {
					state = kCollectName;
					name[ni++] = tc;
			  	}
			  	break;
      
			case kScanComment :
				if ((tc == '\n') || (tc == '\r')) {
					state = kFirstColumn;
				} else {
					// state stays kScanComment.
				}
				break;

			case kFindName :
				if ((tc == '\n') || (tc == '\r')) {
					state = kFirstColumn;
				} else if (isspace(tc)) {
					// state stays kFindName.
				} else {
					state = kCollectName;
					name[ni++] = tc;
				}
				break;

			case kCollectName :
				if ((tc == '\n') || (tc == '\r')) {
					name[ni] = 0;
					ThrowException_AC (kNVRAMPatchParseError, 0);
					state = kFirstColumn;
				} else if (isspace(tc)) {
					state = kFindValue;
				} else {
					name[ni++] = tc;
					// state stays kCollectName.
				}
				break;

			case kFindValue :
			case kContinueValue :
				if ((tc == '\n') || (tc == '\r')) {
					state = kSetenv;
				} else if (isspace(tc)) {
					// state stays kFindValue or kContinueValue.
				} else {
					state = kCollectValue;
					value[vi++] = tc;
				}
				break;

			case kCollectValue :
				if ((tc == '\n') || (tc == '\r')) {
					if (value[vi-1] == '\\') {
						value[vi-1] = '\r';
						state = kContinueValue;
					} else {
						state = kSetenv;
					}
				} else {
					// state stays kCollectValue.
					value[vi++] = tc;
				}
				break;
		}

		if (state == kSetenv) {
			name[ni] = 0;
			value[vi] = 0;
			processOFVariable (name, value);
			state = kFirstColumn;
		}
	}

	if (state != kFirstColumn) {
		ThrowException_AC (kNVRAMPatchParseError, 0);
	}
}

void
XPFPlatform::getCompatibleFromDeviceTree (char **compatible)
{
#ifdef __MACH__

	mach_port_t iokitPort;
	kern_return_t status = IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_registry_entry_t deviceTree = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/");
	ThrowIfNULL_AC ((void *) deviceTree);

	CFDataRef compatibleData = (CFDataRef) IORegistryEntryCreateCFProperty (deviceTree, CFSTR ("compatible"), kCFAllocatorDefault, kNilOptions);
	ThrowIfNULL_AC (compatibleData);
	if (CFGetTypeID (compatibleData) != CFDataGetTypeID ()) {
		gLogFile << "Compatible wrong type" << endl_AC;
		return;
	}
	
	const UInt8 *bytes = CFDataGetBytePtr (compatibleData);
	CFIndex length = CFDataGetLength (compatibleData);
	*compatible = NewPtr (length + 1);
	memcpy (*compatible, bytes, length);
	(*compatible)[length] = '\0';
	
	CFRelease (compatibleData);
	IOObjectRelease (deviceTree);

#else

	RegEntryID deviceTree;
	RegistryEntryIDInit (&deviceTree);
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree", &deviceTree));
	RegPropertyValueSize propSize;
	RegPropertyName *propName = "compatible";
	ThrowIfOSErr_AC (RegistryPropertyGetSize (&deviceTree, propName, &propSize));
	*compatible = NewPtr (propSize + 1);
	ThrowIfNULL_AC (*compatible);
	ThrowIfOSErr_AC (RegistryPropertyGet (&deviceTree, propName, *compatible, &propSize));  
	(*compatible)[propSize] = '\0';
	RegistryEntryIDDispose (&deviceTree);

#endif

	// Clean up the compatible entry in case we've altered it. I don't actually think this
	// is necessary (since this would only happen during an install, and we're unlikely
	// to be running then), but may as well take care of it anyway.
	if (!strncmp (*compatible, "OPEN", 4)) memcpy (*compatible, "AAPL", 4);
	if (!strncmp (*compatible, "XPF,", 4)) memmove (*compatible, *compatible + 4, strlen (*compatible + 4) + 1);
}

