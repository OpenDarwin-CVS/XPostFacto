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

#define 	heathrowID		(0xf3000034)
#define 	heathrowTermEna		(1 << 3)
#define 	heathrowTermDir		(1 << 0)

#define 	heathrowFeatureControl	(0xf3000038)
#define 	heathrowMBRES		(1 << 24)

#define 	heathrowBrightnessControl (0xf3000032)
#define		defaultBrightness	144
#define 	heathrowContrastControl (0xf3000033)
#define		defaultContrast		183

#define 	gossamerSystemReg	(0xff000004)
#define		gossamerAllInOne	(1 << 20)

extern "C" {

void GossamerATISetMBRES (UInt32 state)
{
	UInt32	value;

	value = ml_phys_read (heathrowFeatureControl);

	if (state == 0) value &= ~heathrowMBRES;
		else if (state == 1) value |= heathrowMBRES;

	ml_phys_write (heathrowFeatureControl, value);
	eieio();
}

void GossamerATISetMonitorTermination (Boolean enable)
{
	UInt32	value;

	value = ml_phys_read (heathrowID);

	value |= heathrowTermEna;
	if (enable) value |= heathrowTermDir;
		else value &= ~heathrowTermDir;

	ml_phys_write (heathrowID, value);
	eieio();
}

Boolean GossamerATIIsAllInOne (void)
{
	Boolean	rtn;
	static bool	didBrightness;

	rtn = (0 == (ml_phys_read (gossamerSystemReg) & 0xFFFF0000 & gossamerAllInOne));
	if (rtn && !didBrightness) {
		ml_phys_write_byte (heathrowBrightnessControl, defaultBrightness);
		eieio();
		ml_phys_write_byte (heathrowContrastControl, defaultContrast);
		eieio();
		didBrightness = true;
	}
	return (rtn);
}

} // extern "C"

#define super IOService

OSDefineMetaClassAndStructors (GossamerNDRVSupport, IOService);

GossamerNDRVSupport* GossamerNDRVSupport::fInstance = NULL;

bool
GossamerNDRVSupport::start (IOService *provider)
{
	unsigned x, y;
	
	fATISetMBRES.entry = NULL;
	fATISetMonitorTermination.entry = NULL;
	fATIIsAllInOne.entry = NULL;
	
	if (fInstance) return false;
	if (!super::start (provider)) return false;
	fInstance = this;

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
			function->address = (void *) GossamerATISetMBRES;
		} else if (!strcmp (functionName, "ATISetMonitorTermination")) {
			fATISetMonitorTermination.entry = function;
			fATISetMonitorTermination.address = function->address;
			function->address = (void *) GossamerATISetMonitorTermination;		
		} else if (!strcmp (functionName, "ATIIsAllInOne")) {
			fATIIsAllInOne.entry = function;
			fATIIsAllInOne.address = function->address;
			function->address = (void *) GossamerATIIsAllInOne;		
		}
	}
	
	return true;
}

void 
GossamerNDRVSupport::stop (IOService *provider)
{
	if (fATISetMBRES.entry) fATISetMBRES.entry->address = fATISetMBRES.address;
	if (fATISetMonitorTermination.entry) fATISetMonitorTermination.entry->address = fATISetMonitorTermination.address;
	if (fATIIsAllInOne.entry) fATIIsAllInOne.entry->address = fATIIsAllInOne.address;

	super::stop (provider);
}

