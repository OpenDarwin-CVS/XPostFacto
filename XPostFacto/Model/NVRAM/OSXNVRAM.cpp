/*

Copyright (c) 2002
Other World Computing
All rights reserved

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer as the first lines of
   each file.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

3. Redistributions in binary form must retain the link to Other World
   Computing's web site in the application's "About Box."

This software is provided by Other World Computing ``as is'' and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed. In no event
shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or
profits; or business interruption) however caused and on any theory of
liability, whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

*/

// ===================
// OS X NVRAM
// ===================

#include "OSXNVRAM.h"

#include "XPFAuthorization.h"
#include "XPFLog.h"
    
OSXNVRAM::OSXNVRAM ()
{
	readFromNVRAM ();
}
    
void 
OSXNVRAM::processDictionary (const void *key, const void *value, void *context)
{
	((OSXNVRAM *) context)->setKeyForValue (key, value);
}

void
OSXNVRAM::setKeyForValue (const void *key, const void *value)
{
	const char *name = CFStringGetCStringPtr ((CFStringRef) key, kCFStringEncodingMacRoman);
	if (!strcmp (name, "boot-args")) return;
	
	CFTypeID type = CFGetTypeID (value);
		
	if (type == CFBooleanGetTypeID ()) {
		setBooleanValue (name, CFBooleanGetValue ((CFBooleanRef) value));
	} else if (type == CFStringGetTypeID ()) {
		setStringValue (name, CFStringGetCStringPtr ((CFStringRef) value, kCFStringEncodingMacRoman));
	} else if (type == CFNumberGetTypeID ()) {
		long number;
		CFNumberGetValue ((CFNumberRef) value, kCFNumberSInt32Type, &number);
		setNumericValue (name, number);	
	}
}
    
void
OSXNVRAM::readFromNVRAM ()
{	
	mach_port_t iokitPort;
	kern_return_t status = IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_registry_entry_t options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	ThrowIfNULL_AC ((void *) options);

	CFMutableDictionaryRef properties;
	IORegistryEntryCreateCFProperties (options, &properties, kCFAllocatorDefault, kNilOptions);

 	CFDictionaryApplyFunction (properties, &OSXNVRAM::processDictionary, this);

	CFRelease (properties);
	IOObjectRelease (options);
		
	fHasChanged = false;
}

int
OSXNVRAM::writeToNVRAM ()
{
	if (fHasChanged) {		
		fHasChanged = false;
		
		// Check to see whether there is enough space
		unsigned stringLength = 0;
		
		NVRAMValue *current;
		for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {
			if (current->getValueType () == kStringValue) {
				stringLength += strlen (current->getStringValue ());
			}
		}
		
		if (stringLength >= kOFStringCapacity) {
			#if qLogging
				gLogFile << "NVRAM size limits exceeded" << endl_AC;
			#endif
			return -1;
		}
			
		char *arguments[32];
		BlockZero_AC (arguments, sizeof (char *) * 32);
		
		unsigned argNumber = 0;
		for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {
			arguments[argNumber] = new char [strlen (current->getName ()) + strlen (current->getStringValue ()) + 2];
			sprintf (arguments[argNumber], "%s=%s", current->getName (), current->getStringValue ());
			argNumber++;
		}
		
		FILE *pipe;
		XPFAuthorization::ExecuteWithPrivileges ("/usr/sbin/nvram", arguments, &pipe);		
		char buffer[256];
		while (fread (buffer, 256, 1, pipe)) {
			gLogFile << buffer;
		}	
		fclose (pipe);
		
		for (argNumber = 0; argNumber < 32; argNumber++) {
			if (arguments[argNumber]) {
				delete [] arguments[argNumber];
			} else {
				break;
			}
		}
	}
	return 0;
}
