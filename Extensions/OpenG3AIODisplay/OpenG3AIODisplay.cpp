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

#include "OpenG3AIODisplay.h"

static void OpenG3AIODisplayRestoreXPRAM (IODisplay *display, OSDictionary *params);
static void OpenG3AIODisplaySaveXPRAM (OSDictionary *params);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super IODisplayParameterHandler

OSDefineMetaClassAndStructors (OpenG3AIODisplay, IODisplayParameterHandler);

#define heathrowBrightnessControl (0x33)
#define heathrowContrastControl (0x32)

IOService * 
OpenG3AIODisplay::probe (IOService *provider, SInt32 *score)
{
    if (0 == getPlatform()->getProperty ("AllInOne"))
        return (0);
    else
        return (super::probe (provider, score));
}

bool 
OpenG3AIODisplay::start (IOService *provider)
{
    if (!super::start (provider)) return (false);

    fDisplayParams = OSDynamicCast (OSDictionary, getProperty (gIODisplayParametersKey));
    if (!fDisplayParams) return (false);
	
	mach_timespec_t timeout = {5, 0};
	fHeathrow = waitForService (serviceMatching ("Heathrow"), &timeout);
	if (!fHeathrow) return false;
	
	fHeathrowSafeWriteRegUInt8 = OSSymbol::withCString ("heathrow_safeWriteRegUInt8");
	
    return (true);
}

bool 
OpenG3AIODisplay::setDisplay (IODisplay *display)
{
    OSNumber *num;

    display->setProperty (gIODisplayParametersKey, fDisplayParams);

    num = OSNumber::withNumber (kAppleOnboardGUID, 64);
    display->setProperty (gIODisplayGUIDKey, num);
    num->release ();

    OpenG3AIODisplayRestoreXPRAM (display, fDisplayParams);

    return (true);
}

bool 
OpenG3AIODisplay::doDataSet (const OSSymbol *paramName, OSData *value)
{
    return (false);
}

bool 
OpenG3AIODisplay::doIntegerSet (OSDictionary *params, const OSSymbol *paramName, UInt32 value)
{
    if (paramName == gIODisplayBrightnessKey) {
		fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt8, false, (void *) heathrowBrightnessControl,
				(void *) 0xFF, (void *) value, (void *) 0); 
    } else if (paramName == gIODisplayContrastKey) {
		fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt8, false, (void *) heathrowContrastControl,
				(void *) 0xFF, (void *) value, (void *) 0); 
    } else if (paramName == gIODisplayParametersCommitKey)
        OpenG3AIODisplaySaveXPRAM (fDisplayParams);
    else return (false);

    return (true);
}

bool 
OpenG3AIODisplay::doUpdate (void)
{
    return (false);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void OpenG3AIODisplayRestoreXPRAM (IODisplay *display, OSDictionary *params)
{
    UInt8 bytes[2];
    SInt32 value, min, max;
    OSDictionary *param;
    IODTPlatformExpert *dtPlaform = OSDynamicCast (IODTPlatformExpert, IOService::getPlatform ());

    if (!dtPlaform || (kIOReturnSuccess != dtPlaform->readXPRAM (0x50, (UInt8 *) bytes, 2)))
        bytes[0] = bytes[1] = 0;

    if ((param = IODisplay::getIntegerRange (params, gIODisplayBrightnessKey, 0, &min, &max))) {
        value = (bytes[0] + (((UInt8) max) + ((UInt8) min)) / 2) & 0xff;
        display->setForKey (param, gIODisplayBrightnessKey, value, min, max);
    }
	
    if ((param = IODisplay::getIntegerRange (params, gIODisplayContrastKey, 0, &min, &max))) {
        value = (bytes[1] + ((UInt8) max)) & 0xff;
        display->setForKey (param, gIODisplayContrastKey, value, min, max);
    }
}

static void OpenG3AIODisplaySaveXPRAM (OSDictionary * params)
{
    UInt8 bytes[2];
    SInt32 value, min, max;
    OSDictionary *param;
    IODTPlatformExpert *dtPlaform = OSDynamicCast (IODTPlatformExpert, IOService::getPlatform ());

    if (dtPlaform && (kIOReturnSuccess == dtPlaform->readXPRAM (0x50, (UInt8 *) bytes, 2))) {
        if ((param = IODisplay::getIntegerRange (params, gIODisplayBrightnessKey, &value, &min, &max)))
            bytes[0] = value - (min + max) / 2;

        if ((param = IODisplay::getIntegerRange (params, gIODisplayContrastKey, &value, &min, &max)))
            bytes[1] = value - max;
			
        dtPlaform->writeXPRAM (0x50, (UInt8 *) bytes, 2);
    }
}