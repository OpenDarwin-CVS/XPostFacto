/*

 Copyright (c) 2003
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

#include "GossamerDeviceTreeUpdater.h"

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>

#undef super
#define super IOService

OSDefineMetaClassAndStructors(GossamerDeviceTreeUpdater, IOService);

bool
GossamerDeviceTreeUpdater::start (IOService *provider)
{
	if (!super::start (provider)) return false;
	IOCreateThread (&updateDeviceTree, this);
	return true;
}

void
GossamerDeviceTreeUpdater::updateDeviceTree (void *argument)
{
	IOService *self = (IOService *) argument;
	IOService *provider = self->getProvider ();
	provider->waitQuiet ();
	
	IORegistryEntry *options = provider->childFromPath (("options"), gIODTPlane);
	if (!options) {
		IOLog ("GossamerDeviceTreeUpdater: Could not find /options\n");
		return;
	}
	
	bool update = false;
	OSString *bootfile = OSDynamicCast (OSString, options->getProperty ("boot-file"));
	if (bootfile) {
		const char *c = bootfile->getCStringNoCopy ();
		while (*c) {
			if ((c[0] == '-') && (c[1] == 'i')) {
				update = true;
				break;
			}
			c++;
		}
	} else {
		IOLog ("GossamerDeviceTreeUpdater: Could not find boot-file\n");
		return;
	}
	
	if (update) {
		char buffer [36];
		buffer[0] = 0;
			
		if (IODTMatchNubWithKeys (provider, "'AAPL,Gossamer'"))
			strcpy (buffer, "OPEN,Gossamer");
		else if (IODTMatchNubWithKeys (provider, "'AAPL,PowerMac G3'"))
			strcpy (buffer, "OPEN,PowerMac G3");
		else if (IODTMatchNubWithKeys (provider, "'AAPL,PowerBook1998'"))
			strcpy (buffer, "OPEN,PowerBook1998");
			
		if (buffer[0]) {
			provider->setName (buffer);
			strcpy (buffer + strlen (buffer) + 1, "MacRISC");
			OSData *data = OSData::withBytes (buffer, strlen (buffer) + strlen ("MacRISC") + 2);
			provider->setProperty ("compatible", data);
			data->release ();
		}
	}
}