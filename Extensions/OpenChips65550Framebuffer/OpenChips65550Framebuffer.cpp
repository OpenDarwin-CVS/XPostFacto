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

#include "OpenChips65550Framebuffer.h"
#include "OpenChips65550Registers.h"

#include <IOKit/ndrvsupport/IOMacOSVideo.h>
#include <libkern/OSByteOrder.h>

#define kIOFBGammaWidthKey			"IOFBGammaWidth"
#define kIOFBGammaCountKey			"IOFBGammaCount"
#define kIOPMIsPowerManagedKey		"IOPMIsPowerManaged"

#define kDepth8Bit		0
#define kDepth16Bit		1
#define kDepth32Bit		2

#define kDisplayMode	41

enum {
    kOCFSleepState		= 0,
    kOCFDozeState		= 1,
    kOCFWakeState		= 2
};

#define super IOFramebuffer

OSDefineMetaClassAndStructors (OpenChips65550Framebuffer, IOFramebuffer);

void
OpenChips65550Framebuffer::WriteIndexedRegister (unsigned reg, unsigned index, UInt8 value)
{
	if (reg == ATTRIBUTE_INDEX) fDevice->ioRead8 (INPUT_STATUS_REGISTER_1);
	fDevice->ioWrite8 (reg, index);
	if (reg != ATTRIBUTE_INDEX) reg++;
	fDevice->ioWrite8 (reg, value);
}

UInt8
OpenChips65550Framebuffer::ReadIndexedRegister (unsigned reg, unsigned index)
{
	if (reg == ATTRIBUTE_INDEX) fDevice->ioRead8 (INPUT_STATUS_REGISTER_1);
	fDevice->ioWrite8 (reg, index);
	return fDevice->ioRead8 (reg + 1);
}

bool
OpenChips65550Framebuffer::start (IOService *provider)
{
	fCurrentDisplayMode = 0;
	fCurrentDepth = 0;
		
	fGammaValid = false;
	fCLUTValid = false;

	fDevice = OSDynamicCast (IOPCIDevice, provider);
	if (!fDevice) return false;
	fDevice->setIOEnable (true);
		
	return super::start (provider);
}

void
OpenChips65550Framebuffer::initializeRegisters ()
{
	WriteIndexedRegister (EXTENSION_INDEX, MEMORY_CLOCK_DIVISOR_SELECT, 0x00);
	WriteIndexedRegister (EXTENSION_INDEX, MEMORY_CLOCK_VCO_M_DIVISOR, 0x0C);
	WriteIndexedRegister (EXTENSION_INDEX, MEMORY_CLOCK_VCO_N_DIVISOR, 0x06);
	WriteIndexedRegister (EXTENSION_INDEX, MEMORY_CLOCK_DIVISOR_SELECT, 0x91);
	
	WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_0, 0x82);
	WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_1, 0x12);
	WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_2, 0x08);

	WriteIndexedRegister (GRAPHICS_CONTROLLER_INDEX, MISCELLANEOUS, 0x0D);
	
	WriteIndexedRegister (FLAT_PANEL_INDEX, POWER_DOWN_CONTROL_1, 0x21);
	WriteIndexedRegister (FLAT_PANEL_INDEX, PROGRAMMABLE_OUTPUT_DRIVE, 0x74);
	WriteIndexedRegister (FLAT_PANEL_INDEX, HORIZONTAL_TOTAL_LSB, 0x7F);
}

IOReturn 
OpenChips65550Framebuffer::enableController (void)
{
	setProperty (kIOFBGammaWidthKey, 8, 32);
    setProperty (kIOFBGammaCountKey, 256, 32);
	
	initializeRegisters ();
	
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
OpenChips65550Framebuffer::setStartupDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
	// Should be writing this to NVRAM, but that turns out to be more complicated than
	// one might imagine. Ultimately, I'm going to have to redo some of the NVRAM classes.

	return kIOReturnUnsupported;
}

IOReturn 
OpenChips65550Framebuffer::getStartupDisplayMode (IODisplayModeID *displayMode, IOIndex *depth)
{
	*displayMode = kDisplayMode;
	*depth = kDepth8Bit;	// until I get the gamma working in 16 bit
	
	return kIOReturnSuccess;
}

const char * 
OpenChips65550Framebuffer::getPixelFormats ()
{
    static const char *pixelFormats = IO8BitIndexedPixels "\0" IO16BitDirectPixels "\0" IO32BitDirectPixels "\0\0";
    return pixelFormats;
}

IOItemCount 
OpenChips65550Framebuffer::getDisplayModeCount ()
{
	return 1;
}

IOReturn 
OpenChips65550Framebuffer::getDisplayModes (IODisplayModeID *allDisplayModes)
{
	*allDisplayModes = kDisplayMode;
    return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::getInformationForDisplayMode (IODisplayModeID displayMode, IODisplayModeInformation *info)
{
	if (!info) return kIOReturnBadArgument;

    bzero (info, sizeof (*info));

    info->maxDepthIndex	= kDepth16Bit;
    info->nominalWidth = 800;
    info->nominalHeight	= 600;
    info->refreshRate = 0x0;

    return kIOReturnSuccess;
}
 
UInt32 
OpenChips65550Framebuffer::getApertureSize (IODisplayModeID displayMode, IOIndex depth)
{
	IOPixelInformation info;
	getPixelInformation (displayMode, depth, kIOFBSystemAperture, &info);
	return (info.bytesPerRow * info.activeHeight) + 128;
}

UInt64 
OpenChips65550Framebuffer::getPixelFormatsForDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
    return 0;
}

IOReturn 
OpenChips65550Framebuffer::getPixelInformation (IODisplayModeID displayMode, IOIndex depth,
		IOPixelAperture aperture, IOPixelInformation *info)
{
    bzero (info, sizeof (*info));
	
	info->activeWidth = 800;
	info->activeHeight = 600;

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

    info->bytesPerRow = (info->activeWidth + 32) * info->bitsPerPixel / 8;

    return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::getCurrentDisplayMode (IODisplayModeID *displayMode, IOIndex *depth)
{
	if (!fCurrentDisplayMode) return kIOReturnError;

    if (displayMode) *displayMode = fCurrentDisplayMode;
    if (depth) *depth = fCurrentDepth;

    return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::getTimingInfoForDisplayMode (IODisplayModeID displayMode, IOTimingInformation *info)
{
	if (!info) return kIOReturnBadArgument;
	
	info->appleTimingID = timingApple_FixedRateLCD;
	info->flags = 0;
	
	return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::getAttribute (IOSelect attribute, UInt32 *value)
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
OpenChips65550Framebuffer::setAttribute (IOSelect attribute, UInt32 value)
{
	if (attribute == kIOPowerAttribute) {
		handleEvent ((value >= kOCFWakeState) ? kIOFBNotifyWillPowerOn : kIOFBNotifyWillPowerOff);
		
		if (fPowerState < kOCFWakeState) {
			if (value >= kOCFWakeState) {
				// We'll just restore the gamma for the moment
				implementGammaAndCLUT ();
			}
		} else {
			if (value < kOCFWakeState) {
				// we'll just gamma to black for the moment
				fDevice->ioWrite8 (PALETTE_WRITE_INDEX_REGISTER, 0);
				for (int x = 0; x < (256 * 3); x++) {
					fDevice->ioWrite8 (PALLETE_DATA_REGISTER, 0);
				}
			}
		}

		handleEvent ((value >= kOCFWakeState) ? kIOFBNotifyDidPowerOn : kIOFBNotifyDidPowerOff);
		
		fPowerState = value;		
		return kIOReturnSuccess;
	}
	
	return super::setAttribute (attribute, value);
}

IOReturn 
OpenChips65550Framebuffer::setAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 value)
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
OpenChips65550Framebuffer::getAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 *value)
{
	switch (attribute) {
		case kConnectionEnable:
			if (!value) return kIOReturnBadArgument;
			*value = 1;
			break;
			
		case kConnectionSupportsAppleSense:
			return kIOReturnSuccess;
			break;
					
		default:
			return kIOReturnUnsupported;
			break;
	}
	
	return kIOReturnSuccess;
}

IODeviceMemory* 
OpenChips65550Framebuffer::getVRAMRange (void) {
	IOService *provider = getProvider ();
	if (!provider) return NULL;
	IODeviceMemory *mem = provider->getDeviceMemoryWithIndex (0);
	if (!mem) return NULL;
	mem->retain ();
	return mem;
}

IODeviceMemory* 
OpenChips65550Framebuffer::getApertureRange (IOPixelAperture aper)
{
	if (!fCurrentDisplayMode) return NULL;

	IODeviceMemory *deviceMemory = getVRAMRange ();                      
	if (!deviceMemory) return NULL;
	
	IODeviceMemory *apertureRange = IODeviceMemory::withSubRange (deviceMemory, 0, getApertureSize (fCurrentDisplayMode, fCurrentDepth));

	deviceMemory->release (); // since getNVRAMRange () does a retain ()
	return apertureRange;
}

bool 
OpenChips65550Framebuffer::isConsoleDevice (void)
{
    return getProvider ()->getProperty ("AAPL,boot-display") != NULL;
}

void
OpenChips65550Framebuffer::implementGammaAndCLUT ()
{
	switch (fCurrentDepth) {
		case kDepth8Bit:
			if (!fCLUTValid || !fGammaValid) return;
			fDevice->ioWrite8 (PALETTE_WRITE_INDEX_REGISTER, 0);
			for (int x = 0; x < 256; x++) {
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.red[fCLUTEntries[x].red]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.green[fCLUTEntries[x].green]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.blue[fCLUTEntries[x].blue]);
			}
			break;
			
		case kDepth16Bit:
			if (!fGammaValid) return;
			fDevice->ioWrite8 (PALETTE_WRITE_INDEX_REGISTER, 0);
			for (int x = 0; x < 32; x++) {
				unsigned offset = x * 8;
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.red[offset]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.green[offset]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.blue[offset]);
				for (int y = 0; y < (7 * 3); y++) fDevice->ioWrite8 (PALLETE_DATA_REGISTER, 0);
			}
			break;
			
		case kDepth32Bit:
			if (!fGammaValid) return;
			fDevice->ioWrite8 (PALETTE_WRITE_INDEX_REGISTER, 0);
			for (int x = 0; x < 256; x++) {
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.red[x]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.green[x]);
				fDevice->ioWrite8 (PALLETE_DATA_REGISTER, fGammaTable.blue[x]);
			}
			break;
	}
}

IOReturn 
OpenChips65550Framebuffer::setGammaTable (UInt32 channelCount, UInt32 dataCount, UInt32 dataWidth, void *data)
{
	if (dataCount != 256) {
		IOLog ("OpenChips65550Framebuffer::setGammaTable dataCount: %lu dataWidth: %lu\n", dataCount, dataWidth);
		return kIOReturnUnsupported;
	};
	
	if (dataWidth == 8) {
		UInt8 *gammaData8 = (UInt8 *) data;		
		if (channelCount == 3) {
			bcopy (gammaData8, &fGammaTable, 256 * 3);
		} else if (channelCount == 1) {
			for (int x = 0; x < 256; x++) {
				fGammaTable.red[x] = gammaData8[x];
				fGammaTable.green[x] = gammaData8[x];
				fGammaTable.blue[x] = gammaData8[x];
			}
		} else {
			return kIOReturnUnsupported;
		}
	} else if (dataWidth == 16) {
		UInt16 *gammaData16 = (UInt16 *) data;		
		if (channelCount == 3) {
			for (int x = 0; x < 256; x++) {
				fGammaTable.red[x] = gammaData16[x] >> 8;
				fGammaTable.green[x] = gammaData16[x + 256] >> 8;
				fGammaTable.blue[x] = gammaData16[x + 512] >> 8;
			}
		} else if (channelCount == 1) {
			for (int x = 0; x < 256; x++) {
				fGammaTable.red[x] = gammaData16[x] >> 8;
				fGammaTable.green[x] = gammaData16[x] >> 8;
				fGammaTable.blue[x] = gammaData16[x] >> 8;
			}
		} else {
			return kIOReturnUnsupported;
		}	
	} else {
		return kIOReturnUnsupported;
	}   
	
	fGammaValid = true;
	
	implementGammaAndCLUT ();
	
    return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::setCLUTWithEntries (IOColorEntry *colors, UInt32 index, UInt32 numEntries,
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
OpenChips65550Framebuffer::setDisplayMode (IODisplayModeID displayMode, IOIndex depth)
{
	if ((displayMode == fCurrentDisplayMode) && (depth == fCurrentDepth)) return kIOReturnSuccess;
	
	fCurrentDisplayMode = displayMode;
	fCurrentDepth = depth;
	
	IOPixelInformation info;
	getPixelInformation (displayMode, depth, kIOFBSystemAperture, &info);
	UInt32 offset = info.bytesPerRow / 8;
	WriteIndexedRegister (CRT_CONTROLLER_INDEX, EXTENDED_OFFSET, (offset & 0xFF00) >> 8);
	WriteIndexedRegister (CRT_CONTROLLER_INDEX, OFFSET, offset & 0xFF);
	
	switch (depth) {
		case kDepth8Bit:
				WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_1, 0x12);
				WriteIndexedRegister (EXTENSION_INDEX, BITBLT_ENGINE_CONFIGURATION, 0x0);
			break;
			
		case kDepth16Bit:
				WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_1, 0x14);
				WriteIndexedRegister (EXTENSION_INDEX, BITBLT_ENGINE_CONFIGURATION, 0x10);
			break;

		case kDepth32Bit:
				WriteIndexedRegister (EXTENSION_INDEX, PIXEL_PIPELINE_CONFIGURATION_1, 0x17);
				WriteIndexedRegister (EXTENSION_INDEX, BITBLT_ENGINE_CONFIGURATION, 0x20);
			break;
	}
	
	return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::getAppleSense (IOIndex connectIndex, UInt32 *senseType, UInt32 *primary, UInt32 *extended, UInt32 *displayType) 
{
	*senseType = 0;
	*primary = 7;
	*extended = 0;
	*displayType = kPanelConnect;

	return kIOReturnSuccess;
}

IOReturn 
OpenChips65550Framebuffer::connectFlags (IOIndex connectIndex, IODisplayModeID displayMode, IOOptionBits *flags)
{
	if (displayMode == kDisplayMode) {
		*flags = kDisplayModeValidFlag | kDisplayModeSafeFlag | kDisplayModeDefaultFlag;
	} else {
		*flags = kDisplayModeValidFlag;
	}
	return kIOReturnSuccess;
}

bool 
OpenChips65550Framebuffer::hasDDCConnect (IOIndex connectIndex)
{
	return false;
}

