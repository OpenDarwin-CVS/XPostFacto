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

#ifndef __OPENCONTROLFRAMEBUFFER_H__
#define __OPENCONTROLFRAMEBUFFER_H__

#include <IOKit/graphics/IOFramebuffer.h>
#include <IOKit/IOPlatformExpert.h>

class OpenControlFramebuffer : public IOFramebuffer
{

	OSDeclareDefaultStructors (OpenControlFramebuffer);

public:

    virtual IOService * probe(	IOService * 	provider,
				SInt32 *	score );

//    virtual bool start( IOService * provider );

    virtual const char * getPixelFormats( void );

    virtual IOItemCount getDisplayModeCount( void );

    virtual IOReturn getDisplayModes( IODisplayModeID * allDisplayModes );

    virtual IOReturn getInformationForDisplayMode( IODisplayModeID displayMode,
                    IODisplayModeInformation * info );

    virtual UInt64  getPixelFormatsForDisplayMode( IODisplayModeID displayMode,
                    IOIndex depth );

    virtual IOReturn getPixelInformation (
	IODisplayModeID displayMode, IOIndex depth,
	IOPixelAperture aperture, IOPixelInformation * pixelInfo );

    virtual IOReturn getCurrentDisplayMode( IODisplayModeID * displayMode,
                            IOIndex * depth );

    virtual IODeviceMemory * getApertureRange( IOPixelAperture aperture );

    virtual bool isConsoleDevice( void );

    virtual IOReturn setCLUTWithEntries( IOColorEntry * colors, UInt32 index,
                UInt32 numEntries, IOOptionBits options );

    virtual IOReturn setGammaTable( UInt32 channelCount, UInt32 dataCount,
                    UInt32 dataWidth, void * data );
	
};

#endif