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

#include "GossamerNDRVSupport.h"
#include <IOKit/IOLib.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define heathrowID					(0x34)
#define heathrowTermEna				(1 << 27)   // was 1 << 3, but must reverse bytes
#define heathrowTermDir				(1 << 24)   // was 1 << 0, but must reverse bytes

#define heathrowFeatureControl		(0x38)
#define heathrowMBRES				(1 << 0)	// was 1 << 24, but must reverse bytes

#define heathrowBrightnessControl   (0x32)
#define	defaultBrightness			144
#define heathrowContrastControl		(0x33)
#define	defaultContrast				183

#define gossamerSystemReg1			(0xff000004)
#define	gossamerAllInOne			(1 << 4)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOService* GossamerNDRVSupport::fHeathrow;	
const OSSymbol* GossamerNDRVSupport::fHeathrowSafeWriteRegUInt8 = OSSymbol::withCString ("heathrow_safeWriteRegUInt8");
const OSSymbol* GossamerNDRVSupport::fHeathrowSafeWriteRegUInt32 = OSSymbol::withCString ("heathrow_safeWriteRegUInt32");
	
#define super IOService

OSDefineMetaClassAndStructors (GossamerNDRVSupport, IOService);

GossamerNDRVSupport* GossamerNDRVSupport::fInstance = NULL;

bool
GossamerNDRVSupport::start (IOService *provider)
{
	if (fInstance) return false;

	if (!super::start (provider)) return false;
	
	unsigned x, y;
	
	fATISetMBRES.entry = NULL;
	fATISetMonitorTermination.entry = NULL;
	fATIIsAllInOne.entry = NULL;

	mach_timespec_t timeout = {5, 0};
	fHeathrow = waitForService (serviceMatching ("Heathrow"), &timeout);
	if (!fHeathrow) {
		IOLog ("GossamerNDRVSupport::start could not find Heathrow\n");
		return false;
	}
	
	for (x = 0; x < IONumNDRVLibraries; x++) {
		if (!strcmp (IONDRVLibraries[x].name, "ATIUtils")) break;
	}
	
	if (x == IONumNDRVLibraries) {
		IOLog ("GossamerNDRVSupport::start could not find ATIUtils LibraryEntry\n");
		return false;
	}
	
	for (y = 0; y < IONDRVLibraries[x].numSyms; y++) {
		FunctionEntry *function = &IONDRVLibraries[x].functions[y];
		char *functionName = function->name;
		if (!strcmp (functionName, "ATISetMBRES")) {
			fATISetMBRES.entry = function;
			fATISetMBRES.address = function->address;
			function->address = (void *) GossamerNDRVSupport::ATISetMBRES;
		} else if (!strcmp (functionName, "ATISetMonitorTermination")) {
			fATISetMonitorTermination.entry = function;
			fATISetMonitorTermination.address = function->address;
			function->address = (void *) GossamerNDRVSupport::ATISetMonitorTermination;		
		} else if (!strcmp (functionName, "ATIIsAllInOne")) {
			fATIIsAllInOne.entry = function;
			fATIIsAllInOne.address = function->address;
			function->address = (void *) GossamerNDRVSupport::ATIIsAllInOne;		
		}
	}
	
	fInstance = this;

	return true;
}

void 
GossamerNDRVSupport::stop (IOService *provider)
{
	if (fATISetMBRES.entry) fATISetMBRES.entry->address = fATISetMBRES.address;
	if (fATISetMonitorTermination.entry) fATISetMonitorTermination.entry->address = fATISetMonitorTermination.address;
	if (fATIIsAllInOne.entry) fATIIsAllInOne.entry->address = fATIIsAllInOne.address;
	
	fInstance = NULL;

	super::stop (provider);
}

void 
GossamerNDRVSupport::ATISetMBRES (UInt32 state)
{
	fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt32, false, (void *) heathrowFeatureControl, 
			(void *) heathrowMBRES, (void *) (state ? heathrowMBRES : 0), (void *) 0);
}

void 
GossamerNDRVSupport::ATISetMonitorTermination (Boolean enable)
{
	fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt32, false, (void *) heathrowID, 
			(void *) (heathrowTermEna | heathrowTermDir), 
			(void *) (enable ? heathrowTermEna | heathrowTermDir : heathrowTermEna), (void *) 0);
}

Boolean 
GossamerNDRVSupport::ATIIsAllInOne (void)
{
	static bool	didBrightness = false;
	static Boolean rtn = (0 == (ml_phys_read_half (gossamerSystemReg1) & gossamerAllInOne));
	if (rtn && !didBrightness) {
		fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt8, false, (void *) heathrowBrightnessControl, 
				(void *) 0xFF, (void *) defaultBrightness, (void *) 0);
		fHeathrow->callPlatformFunction (fHeathrowSafeWriteRegUInt8, false, (void *) heathrowContrastControl, 
				(void *) 0xFF, (void *) defaultContrast, (void *) 0);
		didBrightness = true;
	}
	return (rtn);
}
