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

#include <libkern/c++/OSContainers.h>
#include <IOKit/system.h>
#include <IOKit/IOLib.h>
#include <IOKit/graphics/IODisplay.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>
#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/assert.h>

#include <machine/ansi.h>
#ifndef offsetof
#include <stddef.h>
#endif

class OpenG3AIODisplay : public IODisplayParameterHandler {

	OSDeclareDefaultStructors (OpenG3AIODisplay);

private:
	OSDictionary *fDisplayParams;
	IOService *fHeathrow;
	const OSSymbol *fHeathrowSafeWriteRegUInt8;

public:
	virtual IOService* probe (IOService *provider, SInt32 *score);
	virtual bool start (IOService *provider);

	virtual bool setDisplay (IODisplay *display);
	virtual bool doIntegerSet (OSDictionary *params, const OSSymbol *paramName, UInt32 value);
	virtual bool doDataSet (const OSSymbol *paramName, OSData *value);
	virtual bool doUpdate (void);
};