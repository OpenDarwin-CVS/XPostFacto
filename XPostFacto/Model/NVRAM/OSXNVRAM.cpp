/*

Copyright (c) 2002, 2005
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
#include "XPFPlatform.h"
        
void 
OSXNVRAM::processDictionary (const void *key, const void *value, void *context)
{
	((OSXNVRAM *) context)->setKeyForValue (key, value);
}

void
OSXNVRAM::setKeyForValue (const void *key, const void *value)
{
	char name[256];
	if (!CFStringGetCString ((CFStringRef) key, name, 256, kCFStringEncodingASCII)) return;
	
	// Only set ones that we care about
	if (!getValue (name)) return;
	
	CFTypeID type = CFGetTypeID (value);
		
	if (type == CFBooleanGetTypeID ()) {
		setBooleanValue (name, CFBooleanGetValue ((CFBooleanRef) value));
	} else if (type == CFStringGetTypeID ()) {
		char strVal[4096];
		if (!CFStringGetCString ((CFStringRef) value, strVal, 4096, kCFStringEncodingASCII)) return;
		if (XPFPlatform::GetPlatform()->getIsNewWorld() || XPFPlatform::GetPlatform()->getEmulatingNewWorld()) {
			setStringValue (name, strVal);
		} else {
			// On Old World, we ignore the boot-args, and pick it up from the boot-command
			if (!strcmp (name, "boot-args")) return;
			if (!strcmp (name, "boot-command")) {
				if (!strncmp (strVal, "0 bootr", strlen ("0 bootr"))) {
					setStringValue ("boot-command", "0 bootr");
					setStringValue ("boot-args", strVal + strlen ("0 bootr"));
				} else {
					setStringValue ("boot-command", strVal);
					setStringValue ("boot-args", "");
				}
			} else {
				setStringValue (name, strVal);
			}
		}
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
	NVRAMValue *current;
	bool isOldWorld = !XPFPlatform::GetPlatform()->getIsNewWorld() && !XPFPlatform::GetPlatform()->getEmulatingNewWorld();

	if (fHasChanged) {		
		fHasChanged = false;
		
		// Check to see whether there is enough space (if "Old World" or emulating)
		if (!XPFPlatform::GetPlatform()->getIsNewWorld()) {
			unsigned stringLength = 0;
			
			for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {
				if (current->getValueType () == kStringValue) {
					stringLength += strlen (current->getStringValue ());
				}
			}
			
			if ((stringLength >= kOFStringCapacity)) {
				gLogFile << "NVRAM size limits exceeded" << endl_AC;
				return -1;
			}
		}
				
		XPFSetUID myUID (0);
		
		mach_port_t iokitPort;
		kern_return_t status = IOMasterPort (MACH_PORT_NULL, &iokitPort);
		io_registry_entry_t options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
		ThrowIfNULL_AC ((void *) options);
		char strVal[2048];
			
		for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {

			// On Old World, we skip "boot-args" (we'll add it to boot-command)
			if (isOldWorld && !strcmp (current->getName(), "boot-args")) continue;

			CFStringRef nameRef = CFStringCreateWithCString (kCFAllocatorDefault, current->getName (), kCFStringEncodingASCII); 
			CFTypeRef valueRef;
			
			switch (current->getValueType ()) {
				case kBooleanValue:
					valueRef = current->getBooleanValue () ? kCFBooleanTrue : kCFBooleanFalse;
					break;
					
				case kNumericValue:
					long number = current->getNumericValue ();
					valueRef = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &number); 
					break;
					
				case kStringValue:
					strcpy (strVal, current->getStringValue ());
					// On Old World, we add boot-args to boot-command
					if (isOldWorld && !strcmp (current->getName(), "boot-command")) strcat (strVal, getStringValue ("boot-args"));
					valueRef = CFStringCreateWithCString (kCFAllocatorDefault, strVal, kCFStringEncodingASCII); 
					break;
			}
			
			IORegistryEntrySetCFProperty (options, nameRef, valueRef); 
			
			CFRelease (nameRef);
			CFRelease (valueRef);
		}
		
		IOObjectRelease (options);
	}
		
	return 0;
}
