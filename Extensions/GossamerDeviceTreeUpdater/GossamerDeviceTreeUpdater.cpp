/*

 Copyright (c) 2003, 2005
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
#include <IOKit/system.h>

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
	GossamerDeviceTreeUpdater *self = (GossamerDeviceTreeUpdater *) argument;
	IOService *provider = self->getProvider ();
	
	// We wait until all the driver matching has finished. The reason is that some of the driver
	// configuration relies on the original "compatible" value.
	provider->waitQuiet ();
	
	IORegistryEntry *options = provider->childFromPath (("options"), gIODTPlane);
	if (!options) {
		IOLog ("GossamerDeviceTreeUpdater: Could not find /options\n");
		return;
	}
	
	// We only need to adjust things when installing, so we check for the boot-file
	// setting which XPF uses to indicate an install.
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
		// We add "XPF," to the front of whatever name we matched
		OSString *nameMatched = OSDynamicCast (OSString, self->getProperty ("IONameMatched"));
		if (nameMatched) {
			char buffer [64];
			sprintf (buffer, "XPF,%s", nameMatched->getCStringNoCopy ());
			provider->setName (buffer);

			self->adjustProperty ("model");
			self->adjustProperty ("compatible");
		}
	}
}

#define kPrefix		"XPF,"

void
GossamerDeviceTreeUpdater::adjustProperty (char *key)
{
	OSData *data = OSDynamicCast (OSData, getProperty (key));
	if (data) {
		OSData *newData = OSData::withCapacity (data->getLength () + strlen (kPrefix));
		if (newData) {
			newData->appendBytes (kPrefix, strlen (kPrefix));
			newData->appendBytes (data);
			setProperty (key, newData);
			newData->release ();
		}
	}
}