/*

 Copyright (c) 2004
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 
*/

#include "OpenControlFramebuffer.h"

#include <IOKit/ndrvsupport/IOMacOSVideo.h>
#include <libkern/OSByteOrder.h>

#define kIOFBGammaWidthKey			"IOFBGammaWidth"
#define kIOFBGammaCountKey			"IOFBGammaCount"
#define kIOPMIsPowerManagedKey		"IOPMIsPowerManaged"

#define COLOR_MAP_ADDRESS 	0xF301B000
#define COLOR_MAP_LENGTH	0x1000

#define CONTROL_FRAMEBUFFER_OFFSET 0x10

#define kDepth8Bit		0
#define kDepth16Bit		1
#define kDepth32Bit		2

enum {
    kOCFSleepState		= 0,
    kOCFDozeState		= 1,
    kOCFWakeState		= 2
};

#define super IOFramebuffer

OSDefineMetaClassAndStructors (OpenControlFramebuffer, IOFramebuffer);

struct ControlDisplayMode {
	IODisplayModeID displayMode;
    UInt32 nominalWidth;
    UInt32 nominalHeight;
    IOFixed1616 refreshRate;
	IOAppleTimingID appleTimingID;
	char clockVals[3];		// to be sent to PE_write_IIC
	UInt32 regVals[19];		// note that regVal[0] is not set, and regVal[8] will need to be calculated
};

const ControlDisplayMode gControlDisplayMode [] = {
	{0, 0, 0, 0, 0, {0, 0, 0}, {0}},	// 0 -- make things 1-based
	
	{1, 512, 384, 0x3BF080, timingAppleNTSC_ST, {0, 0, 0}, {0}},
	{2, 512, 384, 0x3C0000, timingApple_512x384_60hz, {0, 0, 0}, {0}},
	{3, 640, 480, 0x320000, timingApplePAL_ST, {0, 0, 0}, {0}},
	{4, 640, 480, 0x3BF080, timingAppleNTSC_FF, {0, 0, 0}, {0}},
	
	{5, 640, 480, 0x3BF080, timingVESA_640x480_60hz, {18, 29, 2}, 
		{0, 1038, 1028, 68, 35, 2, 1048, 1050, 63, 398, 383, 63, 47, 399, 24, 200, 352, 0, 59}},
	{6, 640, 480, 0x42AA80, timingApple_640x480_67hz, {25, 48, 2}, 
		{0, 1045, 1042, 82, 43, 4, 1048, 1050, 70, 430, 391, 71, 31, 431, 16, 216, 400, 0, 59}},
	
	{7, 640, 870, 0x4B0000, timingApple_640x870_75hz, {0, 0, 0}, {0}},
	{8, 768, 576, 0x320000, timingApplePAL_FF, {0, 0, 0}, {0}},

	{9, 800, 600, 0x380000, timingVESA_800x600_56hz, {10, 23, 2}, 
		{0, 1247, 1246, 46, 24, 2, 1248, 1250, 84, 510, 491, 91, 35, 511, 18, 256, 476, 0, 59}},
	{10, 800, 600, 0x3C0000, timingVESA_800x600_60hz, {25, 64, 2}, 
		{0, 1253, 1252, 52, 29, 6, 1254, 1256, 92, 526, 499, 99, 63, 527, 32, 264, 464, 0, 59}},
	{11, 800, 600, 0x480000, timingVESA_800x600_72hz, {5, 16, 2}, 
		{0, 1293, 1256, 56, 33, 10, 1330, 1332, 82, 518, 483, 83, 59, 519, 30, 260, 460, 0, 59}},
	{12, 800, 600, 0x4B0000, timingVESA_800x600_75hz, {6, 19, 2}, 
		{0, 1247, 1246, 46, 25, 4, 1248, 1250, 104, 526, 511, 111, 39, 527, 20, 264, 488, 0, 59}},

	{13, 832, 624, 0x4B0000, timingApple_832x624_75hz, {19, 70, 2}, 
		{0, 1331, 1330, 82, 43, 4, 1332, 1334, 136, 574, 551, 135, 31, 575, 16, 288, 544, 0, 59}},

	{14, 1024, 768, 0x3C0000, timingVESA_1024x768_60hz, {25, 52, 3}, 
		{0, 1607, 1604, 68, 39, 10, 1610, 1612, 132, 670, 651, 139, 67, 671, 34, 336, 604, 0, 59}},
	{15, 1024, 768, 0x480000, timingVESA_1024x768_70hz, {5, 12, 3}, 
		{0, 1607, 1604, 68, 39, 10, 1610, 1612, 124, 662, 643, 131, 67, 663, 34, 332, 596, 0, 59}},
	{16, 1024, 768, 0x4B0000, timingVESA_1024x768_75hz, {25, 63, 3}, 
		{0, 1597, 1596, 60, 32, 4, 1598, 1600, 128, 654, 647, 135, 47, 655, 24, 328, 608, 0, 59}},
		
	{17, 1024, 768, 0x4B0000, timingApple_1024x768_75hz, {25, 63, 3}, 
		{0, 1597, 1596, 60, 32, 4, 1598, 1600, 120, 654, 639, 127, 47, 655, 24, 328, 608, 0, 59}},	
		
	{18, 1152, 870, 0x4B0000, timingApple_1152x870_75hz, {5, 16, 3}, 
		{0, 1825, 1822, 82, 43, 4, 1828, 1830, 126, 726, 703, 127, 63, 727, 32, 364, 664, 0, 59}},

	{19, 1280, 960, 0x4B0000, timingVESA_1280x960_75hz, {1, 4, 3}, 
		{0, 1997, 1996, 76, 40, 4, 1998, 2000, 176, 838, 823, 183, 71, 839, 36, 420, 768, 0, 59}},

	{20, 1280, 1024, 0x4B0000, timingVESA_1280x1024_75hz, {22, 95, 3}, 
		{0, 2129, 2128, 80, 42, 4, 2130, 2132, 164, 842, 811, 171, 55, 843, 28, 422, 788, 0, 59}}
};

const UInt32 gNumControlDisplayModes = sizeof (gControlDisplayMode) / sizeof (ControlDisplayMode);

static unsigned
OCFUnsignedPower (unsigned x, unsigned y)
{
	unsigned result = 1;
	while (y--) result *= x;
	return result;
}

bool 
OpenControlFramebuffer::start (IOService *provider)
{
	fRegisterMap = NULL;	
	fRegister = NULL;
	fColorDeviceMemory = NULL;
	fColorMap = NULL;
	fColorRegister = NULL;
	
	fCurrentDisplayMode = 0;
	fCurrentDepth = 0;
	
	fVRAMBank1 = false;
	fVRAMBank2 = false;
	fVRAMSize = 0;
	fApertureOffset = 0x800000;
	
	fGammaValid = false;
	fCLUTValid = false;
		
	return super::start (provider);
}

void 
OpenControlFramebuffer::free ()
{
	if (fColorMap) fColorMap->release ();
	if (fColorDeviceMemory) fColorDeviceMemory->release ();
	if (fRegisterMap) fRegisterMap->release ();
	
	super::free ();
}

IOReturn 
OpenControlFramebuffer::enableController (void)
{
	setProperty (kIOFBGammaWidthKey, 8, 32);
    setProperty (kIOFBGammaCountKey, 256, 32);

	fRegisterMap = getProvider()->mapDeviceMemoryWithIndex (1);	
	if (fRegisterMap) fRegister = (OCFRegister *) fRegisterMap->getVirtualAddress ();
	if (!fRegister) {
		IOLog ("OpenControlFramebuffer: unable to map memory for registers\n");
		return kIOReturnError;
	}
	
	fColorDeviceMemory = IODeviceMemory::withRange (COLOR_MAP_ADDRESS, COLOR_MAP_LENGTH); 
	if (fColorDeviceMemory) fColorMap = fColorDeviceMemory->map ();
	if (fColorMap) fColorRegister = (OCFColorRegister *) fColorMap->getVirtualAddress ();
	if (!fColorRegister) {
		IOLog ("OpenControlFramebuffer: unable to map memory for color registers\n");
		return kIOReturnError;
	}
	
	// need CUDA around to service PE_write_IIC
	mach_timespec_t timeout = {5, 0};
	IOService *cuda = waitForService (serviceMatching ("AppleCuda"), &timeout);
	if (!cuda) {
		IOLog ("OpenControlFramebuffer: AppleCuda not available\n");
		return kIOReturnError;
	}
	
	UInt32 displayType;
	getAppleSense (0, 0, 0, 0, &displayType);
	if (displayType == kNoConnect) {
		IOLog ("OpenControlFramebuffer: no display connected\n");
		return kIOReturnError;
	}
		
	IOReturn kr = locateVRAMBanks ();
	if (kr != kIOReturnSuccess) return kr;

	IODisplayModeID initialDisplayMode;
	IOIndex initialDepth;
	getStartupDisplayMode (&initialDisplayMode, &initialDepth);
	setDisplayMode (initialDisplayMode, initialDepth);
	
	fPowerState = kOCFWakeState;
	
	IOPMPowerState powerStates [] = {
		{kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{kIOPMPowerStateVersion1, 0, 0, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 },
		{kIOPMPowerStateVersion1, IOPMDeviceUsable, IOPMPowerOn, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    registerPowerDriver (this, powerStates, 3);
    temporaryPowerClampOn ();
    changePowerStateTo (kOCFDozeState);
	getProvider()->setProperty (kIOPMIsPowerManagedKey, true);
	
	return kIOReturnSuccess;
}

IOReturn
OpenControlFramebuffer::locateVRAMBanks ()
{
	IOMemoryDescriptor *vramDescriptor = getVRAMRange ();
	if (!vramDescriptor) {
		IOLog ("OpenControlFramebuffer: could not get VRAM range\n");
		return kIOReturnError;
	}

	IOMemoryMap *vramMap = vramDescriptor->map (kIOMapInhibitCache);
	if (!vramMap) {
		IOLog ("OpenControlFramebuffer: could not map VRAM range\n");
		return kIOReturnError;
	}
	
	char *bank1 = (char *) vramMap->getVirtualAddress () + fApertureOffset;
	char *bank2 = bank1 + 0x600000;
	
	fRegister[22].reg = OSSwapHostToLittleInt32 (0x39); eieio ();
	strcpy (bank1, "chaos"); eieio ();
	fVRAMBank1 = !strcmp (bank1, "chaos");
	
	fRegister[22].reg = OSSwapHostToLittleInt32 (0x31); eieio ();
	strcpy (bank2, "control"); eieio ();
	fVRAMBank2 = !strcmp (bank2, "control");

	IOLog ("OpenControlFramebuffer::locateNVRAM Bank1 %s, Bank2 %s\n", fVRAMBank1 ? "present" : "absent", fVRAMBank2 ? "present" : "absent");

	vramMap->release ();
	vramDescriptor->release ();
	
	fVRAMSize = (fVRAMBank1 ? 0x200000 : 0) + (fVRAMBank2 ? 0x200000 : 0);
	if (!fVRAMSize) {
		IOLog ("OpenControlFramebuffer: No VRAM detected\n");
		return kIOReturnError;
	}
	
	if (fVRAMBank2 && !fVRAMBank1) fApertureOffset += 0x600000;
	
	return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::setStartupDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
	// Should be writing this to NVRAM, but that turns out to be more complicated than
	// one might imagine. Ultimately, I'm going to have to redo some of the NVRAM classes.

	return kIOReturnUnsupported;
}

IOReturn 
OpenControlFramebuffer::getStartupDisplayMode (IODisplayModeID *displayMode, IOIndex *depth)
{
	// Should be reading this from NVRAM, but that turns out to be more complicated than one
	// might imagine. Ultimately, I'm going to have to redo some of the NVRAM classes.

	IODisplayModeID theDisplayMode = 0;
	IOOptionBits flags;
	
	for (unsigned x = 1; x < gNumControlDisplayModes; x++) {
		if (!gControlDisplayMode[x].clockVals[0]) continue;
		interpretAppleSense (x, &flags, 0);
		if (flags & kDisplayModeDefaultFlag) {
			theDisplayMode = x;
			break;
		}
	}

	if (theDisplayMode == 0) theDisplayMode = 17; // a reasonably sane default, hopefully

	IODisplayModeInformation info;
	getInformationForDisplayMode (theDisplayMode, &info);
	if (info.maxDepthIndex == kDepth32Bit) info.maxDepthIndex = kDepth16Bit;

	if (displayMode) *displayMode = theDisplayMode;
	if (depth) *depth = info.maxDepthIndex;
	
	return kIOReturnSuccess;
}

const char * 
OpenControlFramebuffer::getPixelFormats ()
{
    static const char *pixelFormats = IO8BitIndexedPixels "\0" IO16BitDirectPixels "\0" IO32BitDirectPixels "\0\0";
    return pixelFormats;
}

IOItemCount 
OpenControlFramebuffer::getDisplayModeCount ()
{
	IOItemCount count = 0;
	for (unsigned x = 1; x < gNumControlDisplayModes; x++) {
		if (gControlDisplayMode[x].clockVals[0]) count++;
	}
	return count;
}

IOReturn 
OpenControlFramebuffer::getDisplayModes (IODisplayModeID *allDisplayModes)
{
	for (unsigned x = 1; x < gNumControlDisplayModes; x++) {
		if (gControlDisplayMode[x].clockVals[0]) {
			*allDisplayModes = x;
			allDisplayModes++;
		}
	}
    return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::getInformationForDisplayMode (IODisplayModeID displayMode, IODisplayModeInformation *info)
{
	if (!info) return kIOReturnBadArgument;

    bzero (info, sizeof (*info));

    info->maxDepthIndex	= kDepth8Bit;
	if (getApertureSize (displayMode, kDepth16Bit) + CONTROL_FRAMEBUFFER_OFFSET < fVRAMSize) info->maxDepthIndex = kDepth16Bit;
	if (getApertureSize (displayMode, kDepth32Bit) + CONTROL_FRAMEBUFFER_OFFSET < fVRAMSize) info->maxDepthIndex = kDepth32Bit;
	
    info->nominalWidth = gControlDisplayMode[displayMode].nominalWidth;
    info->nominalHeight	= gControlDisplayMode[displayMode].nominalHeight;
    info->refreshRate = gControlDisplayMode[displayMode].refreshRate;

    return kIOReturnSuccess;
}
 
UInt32 
OpenControlFramebuffer::getApertureSize (IODisplayModeID displayMode, IOIndex depth)
{
	IOPixelInformation info;
	getPixelInformation (displayMode, depth, kIOFBSystemAperture, &info);
	return (info.bytesPerRow * info.activeHeight) + 128;
}

UInt64 
OpenControlFramebuffer::getPixelFormatsForDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
    return 0;
}

IOReturn 
OpenControlFramebuffer::getPixelInformation (IODisplayModeID displayMode, IOIndex depth,
		IOPixelAperture aperture, IOPixelInformation *info)
{
    bzero (info, sizeof (*info));
	
	info->activeWidth = gControlDisplayMode[displayMode].nominalWidth;
	info->activeHeight = gControlDisplayMode[displayMode].nominalHeight;

    switch (depth) {
        case kDepth8Bit:
			strcpy (info->pixelFormat, IO8BitIndexedPixels);
            info->pixelType = kIOCLUTPixels;
            info->componentMasks[0]	= 0xFF;
            info->bitsPerPixel = 8;
            info->componentCount = 1;
            info->bitsPerComponent = 8;
            break;

        case kDepth16Bit:
            strcpy (info->pixelFormat, IO16BitDirectPixels);
            info->pixelType = kIORGBDirectPixels;
            info->componentMasks[0] = 0x7C00;
            info->componentMasks[1] = 0x03E0;
            info->componentMasks[2] = 0x001F;
            info->bitsPerPixel = 16;
            info->componentCount = 3;
            info->bitsPerComponent = 5;
            break;

        case kDepth32Bit:
            strcpy (info->pixelFormat, IO32BitDirectPixels);
            info->pixelType = kIORGBDirectPixels;
            info->componentMasks[0] = 0x00FF0000;
            info->componentMasks[1] = 0x0000FF00;
            info->componentMasks[2] = 0x000000FF;
            info->bitsPerPixel = 32;
            info->componentCount = 3;
            info->bitsPerComponent = 8;
            break;
    }

    info->bytesPerRow = (info->activeWidth * info->bitsPerPixel / 8) + 32;

    return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::getCurrentDisplayMode (IODisplayModeID *displayMode, IOIndex *depth)
{
	if (!fCurrentDisplayMode) return kIOReturnError;

    if (displayMode) *displayMode = fCurrentDisplayMode;
    if (depth) *depth = fCurrentDepth;

    return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::getTimingInfoForDisplayMode (IODisplayModeID displayMode, IOTimingInformation *info)
{
	if (!info) return kIOReturnBadArgument;
	
	info->appleTimingID = gControlDisplayMode[displayMode].appleTimingID;
	info->flags = 0;
	
	return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::getAttribute (IOSelect attribute, UInt32 *value)
{
	if (!value) return kIOReturnBadArgument;
	
	switch (attribute) {
		case kIOHardwareCursorAttribute:
			*value = 0;
			break;
									
		default:
			return kIOReturnUnsupported;
			break;
	}
	
	return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::setAttribute (IOSelect attribute, UInt32 value)
{
	if (attribute == kIOPowerAttribute) {
		UInt32 reg18;

		handleEvent ((value >= kOCFWakeState) ? kIOFBNotifyWillPowerOn : kIOFBNotifyWillPowerOff);
		
		if (fPowerState < kOCFWakeState) {
			if (value >= kOCFWakeState) {
				reg18 = OSSwapLittleToHostInt32 (fRegister[18].reg); eieio ();
				reg18 &= ~0x400;
				reg18 |= 0x33;
				fRegister[18].reg = OSSwapHostToLittleInt32 (reg18); eieio ();
			}
		} else {
			if (value < kOCFWakeState) {
				reg18 = OSSwapLittleToHostInt32 (fRegister[18].reg); eieio ();
				reg18 |= 0x400;
				reg18 &= ~0x33;
				fRegister[18].reg = OSSwapHostToLittleInt32 (reg18); eieio ();
			}
		}

		handleEvent ((value >= kOCFWakeState) ? kIOFBNotifyDidPowerOn : kIOFBNotifyDidPowerOff);
		
		fPowerState = value;		
		return kIOReturnSuccess;
	}
	
	return super::setAttribute (attribute, value);
}

IOReturn 
OpenControlFramebuffer::setAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 value)
{
	switch (attribute) {
		case kConnectionPower:
			// We just succeed here -- I don't think we have to do anything as such
			return kIOReturnSuccess;
			break;
			
		default:
			return kIOReturnUnsupported;
			break;
	}
	
	return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::getAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 *value)
{
	switch (attribute) {
		case kConnectionEnable:
			if (!value) return kIOReturnBadArgument;
			*value = 1;
			break;
			
		case kConnectionSupportsAppleSense:
			return kIOReturnSuccess;
			break;
			
		case kConnectionFlags:
			if (!value) return kIOReturnBadArgument;
			*value = 0x84;
			break;
		
		default:
			return kIOReturnUnsupported;
			break;
	}
	
	return kIOReturnSuccess;
}

IODeviceMemory* 
OpenControlFramebuffer::getVRAMRange (void) {
	IOService *provider = getProvider ();
	if (!provider) return NULL;
	IODeviceMemory *mem = provider->getDeviceMemoryWithIndex (0);
	if (!mem) return NULL;
	mem->retain ();
	return mem;
}

IODeviceMemory* 
OpenControlFramebuffer::getApertureRange (IOPixelAperture aper)
{
	if (!fCurrentDisplayMode) return NULL;

	IODeviceMemory *deviceMemory = getVRAMRange ();                      
	if (!deviceMemory) return NULL;
	
	IODeviceMemory *apertureRange = IODeviceMemory::withSubRange (deviceMemory, fApertureOffset + CONTROL_FRAMEBUFFER_OFFSET, getApertureSize (fCurrentDisplayMode, fCurrentDepth));

	deviceMemory->release (); // since getNVRAMRange () does a retain ()
	return apertureRange;
}

bool 
OpenControlFramebuffer::isConsoleDevice (void)
{
    return getProvider ()->getProperty ("AAPL,boot-display") != NULL;
}

void
OpenControlFramebuffer::implementGammaAndCLUT ()
{
	switch (fCurrentDepth) {
		case kDepth8Bit:
			if (!fCLUTValid || !fGammaValid) return;
			for (int x = 0; x < 256; x++) {
				fColorRegister[0].reg = x; eieio ();
				fColorRegister[3].reg = fGammaTable.red[fCLUTEntries[x].red]; eieio ();
				fColorRegister[3].reg = fGammaTable.green[fCLUTEntries[x].green]; eieio ();
				fColorRegister[3].reg = fGammaTable.blue[fCLUTEntries[x].blue]; eieio ();
			}
			break;
			
		case kDepth16Bit:
			if (!fGammaValid) return;
			for (int x = 0; x < 32; x++) {
				fColorRegister[0].reg = x; eieio ();
				unsigned offset = x * 8;
				fColorRegister[3].reg = fGammaTable.red[offset]; eieio ();
				fColorRegister[3].reg = fGammaTable.green[offset]; eieio ();
				fColorRegister[3].reg = fGammaTable.blue[offset]; eieio ();
			}
			break;
			
		case kDepth32Bit:
			if (!fGammaValid) return;
			for (int x = 0; x < 256; x++) {
				fColorRegister[0].reg = x; eieio ();
				fColorRegister[3].reg = fGammaTable.red[x]; eieio ();
				fColorRegister[3].reg = fGammaTable.green[x]; eieio ();
				fColorRegister[3].reg = fGammaTable.blue[x]; eieio ();
			}
			break;
	}
}

IOReturn 
OpenControlFramebuffer::setGammaTable (UInt32 channelCount, UInt32 dataCount, UInt32 dataWidth, void *data)
{
	if ((dataCount != 256) || (dataWidth != 8)) {
		IOLog ("OpenControlFramebuffer::setGammaTable dataCount: %lu dataWidth: %lu\n", dataCount, dataWidth);
		return kIOReturnUnsupported;
	};

	UInt8 *gammaData = (UInt8 *) data;
	
	if (channelCount == 3) {
		bcopy (gammaData, &fGammaTable, 256 * 3);
	} else if (channelCount == 1) {
		for (int x = 0; x < 256; x++) {
			fGammaTable.red[x] = gammaData[x];
			fGammaTable.green[x] = gammaData[x];
			fGammaTable.blue[x] = gammaData[x];
		}
	} else {
		return kIOReturnUnsupported;
	}
	
	fGammaValid = true;
	
	implementGammaAndCLUT ();
	
    return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::setCLUTWithEntries (IOColorEntry *colors, UInt32 index, UInt32 numEntries,
    IOOptionBits options)
{
	bool byValue = options & kSetCLUTByValue;

	for (UInt32 x = 0; x < numEntries; x++) {
		UInt32 offset = byValue ? colors[x].index : index + x;
		if (offset > 255) continue;
		fCLUTEntries[offset].red = colors[x].red >> 8;
		fCLUTEntries[offset].green = colors[x].green >> 8;
		fCLUTEntries[offset].blue = colors[x].blue >> 8;
	}
	
	fCLUTValid = true;
	
	implementGammaAndCLUT ();

    return kIOReturnSuccess;	
}

IOReturn 
OpenControlFramebuffer::setDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
	if ((displayMode == fCurrentDisplayMode) && (depth == fCurrentDepth)) return kIOReturnSuccess;

	UInt32 regVal[26];	
	bcopy (gControlDisplayMode[displayMode].regVals, regVal, sizeof (gControlDisplayMode[displayMode].regVals));
	
	bool vram4MB = fVRAMBank1 && fVRAMBank2;		
	
	regVal[8] = regVal[11] - OCFUnsignedPower (3, (2 - depth));
	if (vram4MB) regVal[8] -= (2 * (2 - depth));
	
	if (depth == 0) regVal[18] = 59;

	regVal[19] = 0x0;
	
	IOPixelInformation info;
	getPixelInformation (displayMode, depth, kIOFBSystemAperture, &info);
	regVal[20] = info.bytesPerRow;
	
	regVal[22] = fVRAMBank1 ? (fVRAMBank2 ? 0x51 : 0x31) : 0x39;
	regVal[23] = (vram4MB ? 3 : 2) - depth;
	regVal[24] = 0x1E5;
	regVal[25] = 0;
	
	fRegister[18].reg = OSSwapHostToLittleInt32 (regVal[18] | 0x400); eieio ();
	
	for (int x = 0; x < 3; x++) PE_write_IIC (0x50, x + 1, gControlDisplayMode[displayMode].clockVals[x]);
		
	fColorRegister[0].reg = 0x20; eieio ();
	fColorRegister[2].reg = (depth * 4) | (vram4MB ? 0x20 : 0x10); eieio ();
	
	fColorRegister[0].reg = 0x21; eieio ();
	fColorRegister[2].reg = (fVRAMBank2 && !fVRAMBank1) ? 0 : 1; eieio ();
	fColorRegister[0].reg = 0x10; eieio ();
	fColorRegister[2].reg = 0; eieio ();
	fColorRegister[0].reg = 0x11; eieio ();
	fColorRegister[2].reg = 0; eieio ();
	
	for (int x = 1; x < 26; x++) {
		if ((x == 18) || (x == 21)) continue;
		fRegister[x].reg = OSSwapHostToLittleInt32 (regVal[x]); eieio ();
	}
	
	fRegister[18].reg = OSSwapHostToLittleInt32 (regVal[18]); eieio ();
	
	fCurrentDisplayMode = displayMode;
	fCurrentDepth = depth;
	
	return kIOReturnSuccess;
}

void
OpenControlFramebuffer::interpretAppleSensePrimary (IODisplayModeID displayMode, IOOptionBits *flags, UInt32 *displayType)
{
	UInt32 theDisplayType;
	UInt32 theFlags = 0;

	switch (fAppleSensePrimary) {
		case 0:
			theDisplayType = kColorTwoPageConnect;
			if (displayMode == 18) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 1:
			theDisplayType = kFullPageConnect;
			if (displayMode == 7) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 2:
			theDisplayType = kGenericCRT;  // 12" RGB
			if (displayMode == 2) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 3:
			theDisplayType = kMonoTwoPageConnect;
			if (displayMode == 18) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 4:
			theDisplayType = kNTSCConnect;
			if (displayMode == 1) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if (displayMode == 4) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			break;
			
		case 5:
			theDisplayType = kFullPageConnect;
			if (displayMode == 7) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;

		default:
			theDisplayType = kUnknownConnect;
			break;
	}
	
	if (displayType) *displayType = theDisplayType;
	if (flags) *flags = theFlags;
}

void
OpenControlFramebuffer::interpretAppleSenseExtended (IODisplayModeID displayMode, IOOptionBits *flags, UInt32 *displayType)
{
	UInt32 theDisplayType;
	UInt32 theFlags = 0;

	switch (fAppleSenseExtended) {		
		case 0:
		case 48:
			theDisplayType = kPALConnect;
			if (displayMode == 3) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if (displayMode == 8) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			break;
					
		case 3:
			theDisplayType = kMultiModeCRT1Connect;
			if (displayMode == 6) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if (displayMode == 13) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			if ((displayMode == 17) || (displayMode == 18)) theFlags = kDisplayModeValidFlag;
			break;
			
		case 11:
		case 43:
			theDisplayType = kMultiModeCRT2Connect;
			if (displayMode == 6) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			if (displayMode == 13) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if ((displayMode == 17) || (displayMode == 18)) theFlags = kDisplayModeValidFlag;
			break;
			
		case 20:
			theDisplayType = kNTSCConnect;
			if (displayMode == 1) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if (displayMode == 4) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			break;
			
		case 23:
			theDisplayType = kVGAConnect;
			if (displayMode == 5) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if ((displayMode >= 9) && (displayMode <= 12)) theFlags = kDisplayModeValidFlag;
			if ((displayMode >= 14) && (displayMode <= 16)) theFlags = kDisplayModeValidFlag;
			break;
			
		case 35:
			theDisplayType = kMultiModeCRT3Connect;
			if ((displayMode == 6) || (displayMode == 13)) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag;
			if (displayMode == 17) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			if (displayMode == 18) theFlags = kDisplayModeValidFlag;
			break;
			
		case 45:
			theDisplayType = kColor16Connect;
			if (displayMode == 13) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 58:
			theDisplayType = kColor19Connect;
			if (displayMode == 17) theFlags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
			break;
			
		case 63:
			theDisplayType = kNoConnect;
			break;
			
		default:
			theDisplayType = kUnknownConnect;
			break;
	}
	
	if (displayType) *displayType = theDisplayType;
	if (flags) *flags = theFlags;
}

void
OpenControlFramebuffer::interpretAppleSense (IODisplayModeID displayMode, IOOptionBits *flags, UInt32 *displayType)
{
	UInt32 theDisplayType;
	UInt32 theFlags = 0;

	if (fAppleSensePrimary < 6) {
		interpretAppleSensePrimary (displayMode, &theFlags, &theDisplayType);
	} else {
		interpretAppleSenseExtended (displayMode, &theFlags, &theDisplayType);
	}
	
	if (theDisplayType == kUnknownConnect) {
		switch (displayMode) {
			case 6:
			case 13:
			case 17:
			case 18:
				theFlags = kDisplayModeValidFlag;
				break;
		}
	}
	
	if (displayType) *displayType = theDisplayType;
	if (flags) *flags = theFlags;
}

IOReturn 
OpenControlFramebuffer::getAppleSense (IOIndex connectIndex, UInt32 *senseType, UInt32 *primary, UInt32 *extended, UInt32 *displayType) {

	fRegister[21].reg = OSSwapHostToLittleInt32 (007); eieio (); 
	IODelay (1);
	
	fRegister[21].reg = OSSwapHostToLittleInt32 (077); eieio (); 
	IODelay (5);
	fAppleSensePrimary = (OSSwapLittleToHostInt32 (fRegister[21].reg) & 0700) >> 6; eieio ();
	
	fRegister[21].reg = OSSwapHostToLittleInt32 (033); eieio (); 
	IODelay (5);
	fAppleSenseExtended = (OSSwapLittleToHostInt32 (fRegister[21].reg) & 0300) >> 2; eieio ();
	
	fRegister[21].reg = OSSwapHostToLittleInt32 (055); eieio (); 
	IODelay (5);
	UInt32 sense = OSSwapLittleToHostInt32 (fRegister[21].reg) & 0500; eieio ();
	if (sense & 0400) sense |= 0200;
	sense &= 0300;
	fAppleSenseExtended |= sense >> 4;
	
	fRegister[21].reg = OSSwapHostToLittleInt32 (066); eieio (); 
	IODelay (5);
	fAppleSenseExtended |= (OSSwapLittleToHostInt32 (fRegister[21].reg) & 0600) >> 7; eieio ();

	fRegister[21].reg = OSSwapHostToLittleInt32 (077); eieio ();	
	
	if (senseType) *senseType = 0;
	if (primary) *primary = fAppleSensePrimary;
	if (extended) *extended = fAppleSenseExtended;
	if (displayType) interpretAppleSense (0, 0, displayType);
		
	return kIOReturnSuccess;
}

IOReturn 
OpenControlFramebuffer::connectFlags (IOIndex connectIndex, IODisplayModeID displayMode, IOOptionBits *flags)
{
	if (flags) interpretAppleSense (displayMode, flags, 0);
	return kIOReturnSuccess;
}

bool 
OpenControlFramebuffer::hasDDCConnect (IOIndex connectIndex)
{
	return false;
}


