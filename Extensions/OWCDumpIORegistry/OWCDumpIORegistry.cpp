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

#include "OWCDumpIORegistry.h"
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>

#undef super
#define super IOService

OSDefineMetaClassAndStructors(OWCDumpIORegistry, IOService);

#define DUMP_DELAY_SECONDS 60

bool
OWCDumpIORegistry::start (IOService *provider)
{
	if (!super::start (provider)) return false;
				
	kprintf ("OWCDumpIORegistry will dump IORegistry in %d seconds\n", DUMP_DELAY_SECONDS);
	
	IOCreateThread (&dumpIORegistry, NULL);
	
	return true;
}

void
OWCDumpIORegistry::dumpIORegistry (void *argument)
{
	IOSleep (DUMP_DELAY_SECONDS * 1000);
		
	IORegistryIterator *iter = IORegistryIterator::iterateOver (gIOServicePlane, kIORegistryIterateRecursively);
	if (iter == NULL) return;
	
	int maxBufferSize = 2048;
	char *buffer = (char *) IOMalloc (maxBufferSize);
	if (buffer == NULL) return;
	
	OSSerialize *s = OSSerialize::withCapacity (maxBufferSize);
	if (s == NULL) return;
	
	IORegistryEntry *object = iter->getCurrentEntry ();
	while (object) {
		s->clearText ();
		
		int busyState = 0;
		IOService *ios = OSDynamicCast (IOService, object);
		if (ios) busyState = ios->getBusyState ();
					
		int pathSize = maxBufferSize;
		object->getPath (buffer, &pathSize, gIOServicePlane);
		kprintf ("\n--> %s <%s> (%d, %d)\n", buffer, object->getMetaClass ()->getClassName (), object->getRetainCount (), busyState);
		
		if (object->serializeProperties (s)) {
			kprintf ("%s\n", s->text ());
		} else {
			kprintf ("serializeProperties failed\n");
		}
		
		object = iter->getNextObject ();
	} 
	
	IOFree (buffer, maxBufferSize);
	s->release ();
	iter->release ();
}