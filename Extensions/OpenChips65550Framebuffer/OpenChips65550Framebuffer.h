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

#ifndef __OPENCHIPS65550FRAMEBUFFER_H__
#define __OPENCHIPS65550FRAMEBUFFER_H__

#include <IOKit/graphics/IOFramebuffer.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/pci/IOPCIDevice.h>

struct OCFGammaTable {
	UInt8 red[256];
	UInt8 green[256];
	UInt8 blue[256];
};	

struct OCFClutEntry {
	UInt8 red;
	UInt8 green;
	UInt8 blue;
};

class OpenChips65550Framebuffer : public IOFramebuffer {

	OSDeclareDefaultStructors (OpenChips65550Framebuffer);

public:

	virtual bool start (IOService *provider);
    virtual IOReturn enableController (void);

    virtual IOReturn setAttribute (IOSelect attribute, UInt32 value);
    virtual IOReturn getAttribute (IOSelect attribute, UInt32 *value);
    virtual IOReturn setAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 value);
    virtual IOReturn getAttributeForConnection (IOIndex connectIndex, IOSelect attribute, UInt32 *value);

    virtual const char* getPixelFormats (void);
    virtual IOItemCount getDisplayModeCount (void);
    virtual IOReturn getDisplayModes (IODisplayModeID *allDisplayModes);
    virtual IOReturn getInformationForDisplayMode (IODisplayModeID displayMode, IODisplayModeInformation *info);
    virtual UInt64 getPixelFormatsForDisplayMode (IODisplayModeID displayMode, IOIndex depth);

    virtual IOReturn getTimingInfoForDisplayMode(
		IODisplayModeID displayMode, IOTimingInformation * info );

    virtual IOReturn getPixelInformation (	
		IODisplayModeID displayMode, 
		IOIndex depth,
		IOPixelAperture aperture, 
		IOPixelInformation * pixelInfo);

    virtual IOReturn getCurrentDisplayMode (IODisplayModeID *displayMode, IOIndex *depth);

    virtual IODeviceMemory * getApertureRange( IOPixelAperture aperture );
    virtual IODeviceMemory * getVRAMRange( void );

    virtual bool isConsoleDevice (void);

    virtual IOReturn setCLUTWithEntries (IOColorEntry *colors, UInt32 index, UInt32 numEntries, IOOptionBits options);

    virtual IOReturn setGammaTable (UInt32 channelCount, UInt32 dataCount,
                    UInt32 dataWidth, void *data);
	
    virtual IOReturn setDisplayMode( IODisplayModeID displayMode,
                            IOIndex depth );

	virtual IOReturn getAppleSense (IOIndex connectIndex, UInt32 *senseType, UInt32 *primary, UInt32 *extended, UInt32 *displayType);				
	virtual IOReturn connectFlags (IOIndex connectIndex, IODisplayModeID displayMode, IOOptionBits *flags);
	virtual bool hasDDCConnect (IOIndex connectIndex);
	
	virtual IOReturn setStartupDisplayMode (IODisplayModeID displayMode, IOIndex depth);
    virtual IOReturn getStartupDisplayMode (IODisplayModeID *displayMode, IOIndex *depth);

private:

	void implementGammaAndCLUT ();

	UInt32 getApertureSize (IODisplayModeID displayMode, IOIndex depth);

	void WriteIndexedRegister (unsigned reg, unsigned index, UInt8 value);
	UInt8 ReadIndexedRegister (unsigned reg, unsigned index);
	void initializeRegisters ();
																		
private:

	IOPCIDevice *fDevice;
	
	IODisplayModeID fCurrentDisplayMode;
	IOIndex fCurrentDepth;
	
	OCFGammaTable fGammaTable;
	OCFClutEntry fCLUTEntries [256];

	bool fGammaValid;
	bool fCLUTValid;
			
	UInt32 fPowerState;

};

#endif
