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


// ===================
// NVRAM
// ===================

#include "NVRAM.h"
#include <Sound.h>
#include <strings.h>
#include <stdio.h>
#include "XPFNameRegistry.h"
#include "XPFLog.h"

#include "ToolboxNVRAM.h"

// Resource manager stuff

struct OFtc {
	char name[36];
	short id;
};


NVRAMVariables *NVRAMVariables::gVariables = NULL;

const unsigned kOFPartitionOffset = 0x1800;
const unsigned kOFPartitionSize = 0x800;

const unsigned kOSPartitionOffset = 0x0;
const unsigned kOSPartitionSize = 0xC00;

unsigned char kNVRAM7300RegProperty [] = {0x00, 0x01, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x10,
										  0x00, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x00};

#if qDebug

ostream& operator << (ostream &os, NVRAMVariables &var)
{
	os << hex << "magic: " << var.buffer.magic << "\r";
	os << "version: " << var.buffer.version << "\r";
	os << "pages: " << var.buffer.pages << "\r";
	os << "checksum: " << var.buffer.checksum << "\r";
	os << "here: " << var.buffer.here << "\r";
	os << "top: " << var.buffer.top << "\r";
	os << "next: " << var.buffer.next << "\r";
	os << "\r";
	
	os << "little-endian?: " << var.getLittleEndian () << "\r";
	os << "real-mode?: " << var.getRealMode () << "\r";
	os << "auto-boot?: " << var.getAutoBoot () << "\r";
	os << "diag-switch?: " << var.getDiagSwitch () << "\r";
	os << "fcode-debug?: " << var.getFCodeDebug () << "\r";
	os << "oem-logo?: " << var.getUseOEMLogo () << "\r";
	os << "oem-banner?: " << var.getUseOEMBanner () << "\r";
	os << "use-nvramc?: " << var.getUseNVRAMRC () << "\r";
	os << "\r";
	
	os << "real-base: " << hex << var.getRealBase () << "\r";
	os << "real-size: " << var.getRealSize () << "\r";
	os << "virt-base: " << var.getVirtBase () << "\r";
	os << "virt-size: " << var.getVirtSize () << "\r";
	os << "load-base: " << var.getLoadBase () << "\r";
	os << "pci-probe-list: " << var.getPCIProbeList () << "\r";
	os << "screen-columns: " << var.getScreenColumns () << "\r";
	os << "screen-rows: " << var.getScreenRows () << "\r";
	os << "selftest-megs: " << var.getSelftestMegs () << "\r";
	os << "\r";
	
	os << "boot-device: " << hex << " " << var.buffer.bootDevice.offset << " -- " << var.buffer.bootDevice.length << "\r";
	os << "boot-file: " << " " << var.buffer.bootFile.offset << " -- " << var.buffer.bootFile.length << "\r";
	os << "diag-device: " << " " << var.buffer.diagDevice.offset << " -- " << var.buffer.diagDevice.length << "\r";
	os << "diag-file: " << " " << var.buffer.diagFile.offset << " -- " << var.buffer.diagFile.length << "\r";
	os << "input-device: " << " " << var.buffer.inputDevice.offset << " -- " << var.buffer.inputDevice.length << "\r";
	os << "output-device: " << " " << var.buffer.outputDevice.offset << " -- " << var.buffer.outputDevice.length << "\r";
	os << "oem-banner: " << " " << var.buffer.oemBanner.offset << " -- " << var.buffer.oemBanner.length << "\r";
	os << "oem-logo: " << " " << var.buffer.oemLogo.offset << " -- " << var.buffer.oemLogo.length << "\r";
	os << "nvramc: " << " " << var.buffer.nvramrc.offset << " -- " << var.buffer.nvramrc.length << "\r";
	os << "boot-commnand: " << " " << var.buffer.bootCommand.offset << " -- " << var.buffer.bootCommand.length << "\r";
	os << "\r";

	os << "boot-device: " << var.getBootDevice () << "\r";
	os << "boot-file: " << var.getBootFile () << "\r";
	os << "diag-device: " << var.getDiagDevice () << "\r";
	os << "diag-file: " << var.getDiagFile () << "\r";
	os << "input-device: " << var.getInputDevice () << "\r";
	os << "output-device: " << var.getOutputDevice () << "\r";
	os << "oem-banner: " << var.getOEMBanner () << "\r";
	os << "oem-logo: " << var.getOEMLogo () << "\r";
	os << "nvramc: " << var.getNVRAMRC () << "\r";
	os << "boot-commnand: " << var.getBootCommand () << "\r";

	return os;
}

#endif

#define kNVRAMDeviceType "nvram"
#define kRegProperty "reg"

#if qDebug

void 
NVRAMVariables::testNameRegistry ()
{
	RegEntryID fOptionsEntry;
	ThrowIfOSErr_AC (RegistryEntryIDInit (&fOptionsEntry));	
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree:options", &fOptionsEntry));
		
	ThrowIfOSErr_AC (RegistryPropertySet (&fOptionsEntry, "boot-device", "bob", sizeof ("bob")));
	
	RegPropertyModifiers mods;
	ThrowIfOSErr_AC (RegistryPropertyGetMod (&fOptionsEntry, "boot-device", &mods));
	ThrowIfOSErr_AC (RegistryPropertySetMod (&fOptionsEntry, "boot-device", mods | kRegPropertyValueIsSavedToNVRAM));
		
/*	
	RegPropertyIter cookie;
	Boolean done = false;

	ThrowIfOSErr_AC (RegistryPropertyIterateCreate (&fOptionsEntry, &cookie));
	while (!done) {
		RegPropertyNameBuf property;
		ThrowIfOSErr_AC (RegistryPropertyIterate (&cookie, property, &done));
		if (!done) {
			char *value;
			RegPropertyValueSize size;
			ThrowIfOSErr_AC (RegistryPropertyGetSize (&fOptionsEntry, property, &size));
			value = NewPtr (size);
			try {
				ThrowIfOSErr_AC (RegistryPropertyGet (&fOptionsEntry, property, value, &size));
				printf ("%d -- %s", size, property);
				for (int x = 0; x < size; x++) {
					printf (" %d", value[x]);
				}
				printf ("\n");
			}
			catch (...) {
				DisposePtr (value);
				throw;
			}
			DisposePtr (value);
		}
	}
	RegistryPropertyIterateDispose(&cookie);
*/
}

#endif

NVRAMVariables*
NVRAMVariables::chooseSubClass ()
{
	return new ToolboxNVRAM;
	
#if 0
	RegEntryIter cookie;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    RegEntryID nvramRegEntry;
    RegistryEntryIDInit (&nvramRegEntry);
    OSErr err = RegistryEntryIterateCreate (&cookie);
    if (err != noErr) return NULL;
    try {
        ThrowIfOSErr_AC (RegistryEntrySearch(&cookie, iterOp, &nvramRegEntry, &done, 
        	"device_type", kNVRAMDeviceType, sizeof(kNVRAMDeviceType)));
        if (!done) {
			RegPropertyValueSize propSize;
			err = RegistryPropertyGetSize (&nvramRegEntry, kRegProperty, &propSize);
			if (err) return new ToolboxNVRAM;
			unsigned char *reg = (unsigned char *) NewPtr (propSize);
			try {
				ThrowIfOSErr_AC (RegistryPropertyGet (&nvramRegEntry, kRegProperty, reg, &propSize));
				if (!memcmp (reg, kNVRAM7300RegProperty, sizeof (kNVRAM7300RegProperty))) {
//					return new PowerSurgeNVRAM;
				}
			}
			catch (...) {
				if (reg) DisposePtr ((Ptr) reg);
				throw;
			}
			if (reg) DisposePtr ((Ptr) reg);
        }
   	}
   	catch (OSErr error) {
		err = error;
   	}
   	
    RegistryEntryIterateDispose (&cookie);
    RegistryEntryIDDispose (&nvramRegEntry);
    
    return new NVRAMVariables;
#endif
}

NVRAMVariables*
NVRAMVariables::GetVariables ()
{
	// The idea here is to either return the already constructed singleton,
	// or to figure out which subclass to use and return it.
	if (!gVariables) gVariables = chooseSubClass ();
	return gVariables;
}

NVRAMVariables::NVRAMVariables ()
{
	if ((Ptr) RegistryEntryIDInit == (Ptr) kUnresolvedCFragSymbolAddress) return;

	Erase_AC (&buffer);
	
	buffer.magic = 0x1275;
	buffer.version = 0x05;
	buffer.pages = 0x08;
	buffer.here = kOFPartitionOffset + (unsigned) &buffer.strings[0] - (unsigned) &buffer;
	buffer.next = 0;
	
	setAutoBoot (true);
	setUseNVRAMRC (true);
	setRealBase (0xFFFFFFFF);
	setRealSize (0x00100000);
	setVirtBase (0xFFFFFFFF);
	setVirtSize (0x00100000);
	setLoadBase (0x00600000);
	setPCIProbeList (0xFFFFFFFF);
	setScreenColumns (0x64);
	setScreenRows (0x28);
	
	cacheStrings ();
	
	fCanWriteNVRAM = false;
	fCanWriteNVRAMPatches = false;
	fCompatible = NULL;
	
	loadNVRAMPatch ();
}

NVRAMVariables::~NVRAMVariables ()
{
	if (cacheBootDevice) DisposePtr (cacheBootDevice);
	if (cacheBootFile) DisposePtr (cacheBootFile);
	if (cacheDiagDevice) DisposePtr (cacheDiagDevice);
	if (cacheDiagFile) DisposePtr (cacheDiagFile);
	if (cacheInputDevice) DisposePtr (cacheInputDevice);
	if (cacheOutputDevice) DisposePtr (cacheOutputDevice);
	if (cacheOEMBanner) DisposePtr (cacheOEMBanner);
	if (cacheOEMLogo) DisposePtr (cacheOEMLogo);
	if (cacheNVRAMRC) DisposePtr (cacheNVRAMRC);
	if (cacheBootCommand) DisposePtr (cacheBootCommand);
	
	if (fCompatible) DisposePtr (fCompatible);
}

void
NVRAMVariables::read ()
{
	// read the NVRAM
		
	char *bufferPtr = (char *) &buffer;
		
	char line[17], buff[49];
	buff[48] = 0;
	line[16] = 0;
	
	for (int cnt = 0; cnt < kOFPartitionSize; cnt++) {
		bufferPtr[cnt] = readByte (kOFPartitionOffset + cnt);
		
#if qDebug
		line [cnt % 16] = ((bufferPtr[cnt] > 29) && (bufferPtr [cnt] < 128)) ? bufferPtr [cnt] : '_';
		sprintf (buff + ((cnt % 16) * 3), "%.2X ", bufferPtr [cnt]);
		if ((cnt % 16) == 15) {
			printf ("%X %.48s %.16s\r", cnt - 15 + kOFPartitionOffset, buff, line);
		}
#endif
	}
	
	for (int cnt = 0; cnt < kOSPartitionSize; cnt++) {
		osBuffer[cnt] = readByte (kOSPartitionOffset + cnt);

#if qDebug
		line [cnt % 16] = ((osBuffer[cnt] > 29) && (osBuffer [cnt] < 128)) ? osBuffer [cnt] : '_';
		sprintf (buff + ((cnt % 16) * 3), "%.2X ", osBuffer [cnt]);
		if ((cnt % 16) == 15) {
			printf ("%X %.48s %.16s\r", cnt - 15 + kOSPartitionOffset, buff, line);
		}
#endif
	}
	
	cacheStrings ();
	
/*
	char aChar;
	
	char line[17], buff[49];
	buff[48] = 0;
	line[16] = 0;
	
	for (int cnt = 0; cnt < 0x290; cnt++) {
		*nvramPort = (0x966 + cnt) >> 5;
	    asm (eieio);
	    aChar = (nvramData[((0x966 + cnt) & 0x1F) << 4]);
		asm (eieio);
		
		line [cnt % 16] = ((aChar > 29) && (aChar < 128)) ? aChar : '_';
		sprintf (buff + ((cnt % 16) * 3), "%.2X ", aChar);
		if ((cnt % 16) == 15) {
			printf ("%X %.48s %.16s\r", cnt - 15 + 966, buff, line);
		}
	}
	printf ("%.2X %.48s %.16s\r", 0, buff, line);
*/
}

void
NVRAMVariables::cacheStrings ()
{
	cacheString (buffer.bootDevice, &cacheBootDevice);
	cacheString (buffer.bootFile, &cacheBootFile);
	cacheString (buffer.diagDevice, &cacheDiagDevice);
	cacheString (buffer.diagFile, &cacheDiagFile);
	cacheString (buffer.inputDevice, &cacheInputDevice);
	cacheString (buffer.outputDevice, &cacheOutputDevice);
	cacheString (buffer.oemBanner, &cacheOEMBanner);
	cacheString (buffer.oemLogo, &cacheOEMLogo);
	cacheString (buffer.nvramrc, &cacheNVRAMRC);
	cacheString (buffer.bootCommand, &cacheBootCommand);
}

void
NVRAMVariables::cacheString (NVRAMString string, char **cache)
{
	if (*cache) DisposePtr (*cache);
	char *bufferPtr;
	if ((string.offset > kOFPartitionOffset) && (string.length > 0) &&
			((string.offset + string.length) <= (kOFPartitionOffset + kOFPartitionSize))) {
		bufferPtr = (char *) &buffer;
		bufferPtr += string.offset - kOFPartitionOffset;
		*cache = NewPtr (string.length + 1);
		if (*cache) {
			BlockMoveData (bufferPtr, *cache, string.length);
			(*cache)[string.length] = 0;
		} else {
			*cache = NewPtr (1);
			(*cache)[0] = 0;
		}
	} else if ((string.offset > kOSPartitionOffset) && (string.length > 0) &&
			((string.offset + string.length) <= (kOSPartitionOffset + kOSPartitionSize))) {
		bufferPtr = &osBuffer[0];
		bufferPtr += string.offset - kOSPartitionOffset;
		*cache = NewPtr (string.length + 1);
		if (*cache) {
			BlockMoveData (bufferPtr, *cache, string.length);
			(*cache)[string.length] = 0;
		} else {
			*cache = NewPtr (1);
			(*cache)[0] = 0;
		}
	
	} else {
		*cache = NewPtr (1);
		(*cache)[0] = 0;
	}
}

void
NVRAMVariables::packStrings ()
{
	buffer.top = kOFPartitionOffset + kOFPartitionSize;
	BlockSet_AC (&buffer.strings[0], kOFStringCapacity, 0);
	osBufferOffset = 0;

	if (cacheBootDevice) packString (buffer.bootDevice, cacheBootDevice);
	if (cacheBootFile) packString (buffer.bootFile, cacheBootFile);
	if (cacheDiagDevice) packString (buffer.diagDevice, cacheDiagDevice);
	if (cacheDiagFile) packString (buffer.diagFile, cacheDiagFile);
	if (cacheInputDevice) packString (buffer.inputDevice, cacheInputDevice);
	if (cacheOutputDevice) packString (buffer.outputDevice, cacheOutputDevice);
	if (cacheOEMBanner) packString (buffer.oemBanner, cacheOEMBanner);
	if (cacheOEMLogo) packString (buffer.oemLogo, cacheOEMLogo);
	if (cacheNVRAMRC) packString (buffer.nvramrc, cacheNVRAMRC);
	if (cacheBootCommand) packString (buffer.bootCommand, cacheBootCommand);
}

void 
NVRAMVariables::packStringInOSBuffer (NVRAMString &string, char *cache)
{
	int len = strlen (cache);
	if (len + osBufferOffset < kOSPartitionSize) {
		string.offset = kOSPartitionOffset + osBufferOffset;
		string.length = len;
		if (len > 0) BlockMoveData (cache, osBuffer + osBufferOffset - kOSPartitionOffset, len);
	}
	osBufferOffset += len;
}

void 
NVRAMVariables::packString (NVRAMString &string, char *cache)
{
	int len = strlen (cache);
	buffer.top -= len;
	if (buffer.top > buffer.here) {
		string.offset = buffer.top;
		string.length = len;
		if (len > 0) BlockMoveData (cache, ((char *) &buffer) + buffer.top - kOFPartitionOffset, len);
	}
}

int
NVRAMVariables::write ()
{
//	if (!hasChanged) return noErr;
	hasChanged = false;
		
	packStrings ();

	if ((buffer.top <= buffer.here) || (osBufferOffset >= kOSPartitionSize)) {
		#if qLogging
			gLogFile << "NVRAM size limits exceeded" << endl_AC;
		#endif
		return -1;
	}
 	buffer.checksum = 0;
	unsigned checksum = 0;
 	UInt16 *tmpBuffer = (UInt16 *) &buffer;
 	for (int cnt = 0; cnt < kOFPartitionSize / 2; cnt++) checksum += tmpBuffer[cnt];
	checksum %= 0x0000FFFF;
	buffer.checksum = ~checksum;
	
	char *bufferPtr = (char *) &buffer;

	try {
		for (int cnt = 0; cnt < kOFPartitionSize; cnt++) {
			writeByte (kOFPartitionOffset + cnt, bufferPtr[cnt]);
		}

		for (int cnt = 0; cnt < osBufferOffset; cnt++) {
			writeByte (kOSPartitionOffset + cnt, osBuffer[cnt]);
		}
	}
	
	catch (...) {
		return paramErr;
	}

	return noErr;
}

UInt8 
NVRAMVariables::readByte (unsigned offset)
{
#pragma unused (offset)
	// need to subclass this for particular machines
	throw paramErr;
}
		
void 
NVRAMVariables::writeByte (unsigned offset, UInt8 byte)
{
#pragma unused (offset, byte)
	// need to subclass this for particular machines
	throw paramErr;
}

void
NVRAMVariables::setString (char **string, const char *val)
{
	if (strcmp (*string, val)) {
		hasChanged = true;
		if (*string) DisposePtr (*string);
		unsigned size = strlen (val) + 1;
		*string = NewPtr (size);
		BlockMoveData (val, *string, size);
	}
}

void
NVRAMVariables::appendToNVRAMRC (const char *val) {
	unsigned length = strlen (val);
	if (length > 0) {
		hasChanged = true;
		char *newNVRAMRC = NewPtr (length + strlen (cacheNVRAMRC) + 1);
		strcpy (newNVRAMRC, cacheNVRAMRC);
		strcat (newNVRAMRC, val);
		char *oldNVRAMRC = cacheNVRAMRC;
		cacheNVRAMRC = newNVRAMRC;
		DisposePtr (oldNVRAMRC); 
	}	
}





void
NVRAMVariables::loadNVRAMPatch ()
{
	// Find the nvram settings for us
	fCanWriteNVRAMPatches = false;
	
	if ((Ptr) RegistryEntryIDInit == (Ptr) kUnresolvedCFragSymbolAddress) return;
	
	RegEntryID deviceTree;
	RegistryEntryIDInit (&deviceTree);
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree", &deviceTree));
	RegPropertyValueSize propSize;
	RegPropertyName *propName = "compatible";
	ThrowIfOSErr_AC (RegistryPropertyGetSize (&deviceTree, propName, &propSize));
	fCompatible = NewPtr (propSize + 1);
	ThrowIfNULL_AC (fCompatible);
	ThrowIfOSErr_AC (RegistryPropertyGet (&deviceTree, propName, fCompatible, &propSize));  
	fCompatible[propSize] = '\0';
	RegistryEntryIDDispose (&deviceTree);

	#if qLogging
		gLogFile << "Compatible: " << fCompatible << endl_AC;
	#endif
	
	Handle tc = GetResource ('OFtc', 128);
	ThrowIfNULL_AC (tc);
	HLock (tc);
	OFtc *type = (OFtc*) *tc;
	int numberOfEntries = GetResourceSizeOnDisk (tc) / sizeof (OFtc);
	int x;
	for (x = 0; x < numberOfEntries; x++) if (!strcmp (type[x].name, fCompatible)) break;
	if (x == numberOfEntries) {
		#if qLogging
			gLogFile << "Couldn't find entry for NVRAM patches" << endl_AC;
		#endif
	} else {
		// Now we need to get the actual resource we want
		Handle settings = GetResource ('OFpt', type[x].id);
		ThrowIfNULL_AC (settings);
		fCanWriteNVRAMPatches = true;
		HLock (settings);
		
		// now we zero-terminate
		unsigned handleSize = GetHandleSize (settings);
		Ptr zeroTerminatedSettings = NewPtr (handleSize + 1);
		BlockMoveData (*settings, zeroTerminatedSettings, handleSize);
		zeroTerminatedSettings[handleSize] = 0;
		
		// and fix line endings
		for (int x = handleSize; x >= 0; x--) {
			if (zeroTerminatedSettings[x] == 10) zeroTerminatedSettings[x] = 13;
		}

		HUnlock (settings);
		ReleaseResource (settings);
		processNVRAMPatch (zeroTerminatedSettings);
		DisposePtr (zeroTerminatedSettings);
		#if qLogging
			gLogFile << "Found entry for NVRAM patches" << endl_AC;
		#endif
	}
	HUnlock (tc);	
	ReleaseResource (tc);
}


void
NVRAMVariables::processNVRAMPatchLine (char *line)
{
	if (line[0] == '#') return;
	if (!memcmp ("nvramrc ", line, 8)) {
		setNVRAMRC (line + 8);
	}
}

void
NVRAMVariables::processNVRAMPatch (char *settings)
{
	char *line = NewPtr (3000);
	ThrowIfNULL_AC (line);
	char *cursor = line;
	bool finished = false;
	while (!finished) {
		switch (*settings) {
			case 0x00:
				finished = true; 
				// break; intentionally omitted
			case 0x0D:
			case 0x0A:
				*cursor = 0;
				processNVRAMPatchLine (line);
				cursor = line;
				break;
			case '\\':
				 settings++;
				 if (*settings == 0) settings--;
				 // break; intentionally omitted
			default:
				*cursor++ = *settings;
				break;
		}
		settings++;
	}
	DisposePtr (line);
}


