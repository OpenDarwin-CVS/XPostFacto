/*

Copyright (c) 2001, 2002
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

#include "OFAliases.h"
#include "XPFLog.h"

#include <stdio.h>

#ifndef __MACH__
	#include <PCI.h>
#endif

CAutoPtr_AC <OFAliases> OFAliases::sOFAliases = NULL;
bool OFAliases::sHasBeenInitialized = false;

AliasEntry::AliasEntry (const char *key, const char *value)
{
	fKey = NewPtr (strlen (key) + 1);
	strcpy (fKey, key);
	
	fValue = NewPtr (strlen (value) + 1);
	strcpy (fValue, value);
	
	char *start, *end;
	start = end = fValue + 1;
	do {
		end++;
		if ((*end == 0) || (*end == '/')) {
			char *comp = NewPtr (end - start + 1);
			char save = *end;
			*end = 0;
			strcpy (comp, start);
			fComponents.InsertLast (comp);
			*end = save;
			start = end + 1;
		}
	} while (*end != 0);
}

AliasEntry::~AliasEntry ()
{
	DisposePtr (fKey);
	DisposePtr (fValue);
	for (int x = 1; x <= fComponents.GetSize (); x++) {
		DisposePtr (fComponents.At (x));
	} 
}

char *
AliasEntry::abbreviate (TemplateList_AC <char> *wholeName) 
{
	Ptr result = NewPtr (256);
	*result = 0;
	
	if (fComponents.GetSize () > wholeName->GetSize ()) return result;
	
	bool match = true;
	int x;
	for (x = 1; x <= fComponents.GetSize (); x++) {
		char *me = fComponents.At (x);
		char *you = wholeName->At (x);
		if (strcmp (me, you)) {
			match = false;
			char *atSign = strstr (you, "@");
			if (atSign) {
				*atSign = 0;
				if (!strcmp (me, you)) match = true;
				*atSign = '@';
			}
		}
		if (!match) break;
	}
	
	if (match) {
		strcpy (result, fKey);
		for (x = fComponents.GetSize () + 1; x <= wholeName->GetSize (); x++) {
			strcat (result, "/");
			strcat (result, wholeName->At (x));
		}
	}

	return result;
}

void
OFAliases::Initialize ()
{
	if (sHasBeenInitialized) return;
	sHasBeenInitialized = true;
	sOFAliases = new OFAliases;
}

#ifdef __MACH__

void 
OFAliases::processDictionary (const void *key, const void *value, void *context)
{
	OFAliases *self = (OFAliases *) context;
	
	const char *name = CFStringGetCStringPtr ((CFStringRef) key, kCFStringEncodingMacRoman);
	if (!strcmp (name, "AAPL,phandle")) return;
	if (!strcmp (name, "name")) return;
	
	CFTypeID type = CFGetTypeID (value);
		
	if (type == CFDataGetTypeID ()) {
		char str[256];
		strcpy (str, (char *) CFDataGetBytePtr ((CFDataRef) value));
		str [CFDataGetLength ((CFDataRef) value)] = 0;
		
		self->fEntries.InsertLast (new AliasEntry (name, str));
	}
}

#endif

OFAliases::OFAliases ()
{
#ifdef __MACH__

	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_registry_entry_t aliasEntry = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/aliases");
	if (aliasEntry) {
		CFMutableDictionaryRef properties;
		IORegistryEntryCreateCFProperties (aliasEntry, &properties, NULL, 0);
 		if (properties) {
 			CFDictionaryApplyFunction (properties, &OFAliases::processDictionary, this);
 			CFRelease (properties);
 		}
 		IOObjectRelease (aliasEntry);	
 	}
	
#else

	RegEntryID aliasEntry;
	ThrowIfOSErr_AC (RegistryEntryIDInit (&aliasEntry));	
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree:aliases", &aliasEntry));
	
	// OK, the idea is that we need to construct an array of the aliases, in a manner that will be useful
	// for later.
	
	RegPropertyIter cookie;
	Boolean done = false;
	
	ThrowIfOSErr_AC (RegistryPropertyIterateCreate (&aliasEntry, &cookie));
	while (!done) {
		RegPropertyNameBuf property;
		ThrowIfOSErr_AC (RegistryPropertyIterate (&cookie, property, &done));
		if (!done && strcmp (property, "name")) {
			char *value;
			RegPropertyValueSize size;
			ThrowIfOSErr_AC (RegistryPropertyGetSize (&aliasEntry, property, &size));
			value = NewPtr (size + 1);
			ThrowIfOSErr_AC (RegistryPropertyGet (&aliasEntry, property, value, &size));
			value[size] = '\0';
			
			fEntries.InsertLast (new AliasEntry (property, value));
				
			DisposePtr (value);
		}
	}
	RegistryPropertyIterateDispose(&cookie);	
	RegistryEntryIDDispose (&aliasEntry);
#endif
}

OFAliases::~OFAliases ()
{
	for (int x = 1; x <= fEntries.GetSize (); x++) {
		delete (fEntries.At (x));
	} 
}

void
OFAliases::AliasFor (const REG_ENTRY_TYPE regEntry, char *outAlias, char *shortAlias) 
{
	Initialize ();
	return sOFAliases->aliasFor (regEntry, outAlias, shortAlias);
}

void
OFAliases::aliasFor (const REG_ENTRY_TYPE regEntry, char *outAlias, char *shortAlias) 
{			
#ifdef __MACH__

	char ofPath[256];
	kern_return_t retVal = IORegistryEntryGetPath (regEntry, kIODeviceTreePlane, ofPath);
	if (retVal != 0) {
		outAlias[0] = 0;
		shortAlias[0] = 0;
		return;
	}
	
	char *ofName = ofPath;
	if (!strncmp (ofName, "IODeviceTree:", strlen ("IODeviceTree:"))) {
		ofName += strlen ("IODeviceTree:");
	}
	
	strcpy (outAlias, ofName);					
	strcpy (shortAlias, ofName);
	
	char *at;
	char *start = shortAlias;
	while ((at = strstr (start, "@"))) {
		start = at;
		while ((*start != '/') && (start >= shortAlias)) start--;
		BlockMoveData (at, start + 1, strlen (at) + 1);
		start += 2;
	}

#else

	RegEntryID deviceTreeEntry;
	ThrowIfOSErr_AC (RegistryEntryIDInit (&deviceTreeEntry));
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree", &deviceTreeEntry));
	
	RegEntryID iterEntry, parentEntry;
	ThrowIfOSErr_AC (RegistryEntryIDInit (&iterEntry));
	ThrowIfOSErr_AC (RegistryEntryIDInit (&parentEntry));
	ThrowIfOSErr_AC (RegistryEntryIDCopy (regEntry, &iterEntry));
			
	RegPropertyValueSize propSize;
	Ptr prop;
	unsigned regValue;
	OSErr err;
	char nameComponent[256];
	char workString[256];
	char *temp;
	
	TemplateList_AC <char> wholeName;
 	
	Boolean finished = false;
	while (!finished) {
 		ThrowIfOSErr_AC (RegistryCStrEntryToName (&iterEntry, &parentEntry, nameComponent, &finished));
		if (finished) break;
		workString[0] = 0;
		regValue = 0;
		
		if (RegistryEntryIDCompare (&parentEntry, &deviceTreeEntry)) finished = true;
			
		// See if there is a reg property, which we can use for an address		
		err = RegistryPropertyGetSize (&iterEntry, "reg", &propSize);
		if (err == noErr) {
			propSize = sizeof (regValue);
			ThrowIfOSErr_AC (RegistryPropertyGet (&iterEntry, "reg", &regValue, &propSize));
		}
		
		// We want to special-case where the parent is a pci bus or vci bus
		// So that we can include the device number and function number	
		err = RegistryPropertyGetSize (&parentEntry, "device_type", &propSize);
		if (err == noErr) {
			prop = NewPtr (propSize + 1);
			ThrowIfOSErr_AC (RegistryPropertyGet (&parentEntry, "device_type", prop, &propSize));
			prop[propSize] = '\0';
			if (!strcmp (prop, "pci") || !strcmp (prop, "vci")) {
				// since the parent is a pci or vci bus, we get device and function number
				if (regValue) {
					int deviceNumber = GetPCIDeviceNumber ((PCIAssignedAddress *) &regValue);
					int functionNumber = GetPCIFunctionNumber ((PCIAssignedAddress *) &regValue);
					if (functionNumber) {
						snprintf (workString, 255, "%s@%X,%X", nameComponent, deviceNumber, functionNumber);
					} else {
						snprintf (workString, 255, "%s@%X", nameComponent, deviceNumber);
					}
				} else {
					strcpy (workString, nameComponent);
				}
			}
		}
			
		// Special case FW devices
		if (workString[0] == 0) {
			err = RegistryPropertyGetSize (&iterEntry, "fw-mao", &propSize);
			if (err == noErr) {
				unsigned fwmao, fwlun;
				ThrowIfOSErr_AC (RegistryPropertyGet (&iterEntry, "fw-mao", &fwmao, &propSize));
				err = RegistryPropertyGetSize (&iterEntry, "fw-lun", &propSize);
				if (err == noErr) {
					ThrowIfOSErr_AC (RegistryPropertyGet (&iterEntry, "fw-lun", &fwlun, &propSize));
					sprintf (workString, "@%X/@%X", fwmao, fwlun);	
				}
			}			
		}
			
		// Special case for FW bus
		if (workString[0] == 0) {
			err = RegistryPropertyGetSize (&iterEntry, "fw-guid", &propSize);
			if (err == noErr) {
				unsigned fwguid[2];
				ThrowIfOSErr_AC (RegistryPropertyGet (&iterEntry, "fw-guid", fwguid, &propSize));
				sprintf (workString, "@%X%.8X", fwguid[0], fwguid[1]);
			}			
		}
					
		// Last chance!
		if (workString[0] == 0) {
			if (regValue) {
				sprintf (workString, "%s@%X", nameComponent, regValue);	
			} else {
				strcpy (workString, nameComponent);
			}
		}
		
		RegistryEntryIDDispose (&iterEntry);
		ThrowIfOSErr_AC (RegistryEntryIDCopy (&parentEntry, &iterEntry));
		RegistryEntryIDDispose (&parentEntry);
	
		temp = NewPtr (strlen (workString) + 1);
		strcpy (temp, workString);
		wholeName.InsertFirst (temp);
	}

	strcpy (outAlias, "");
	for (int x = 1; x <= wholeName.GetSize (); x++) {
		strcat (outAlias, "/");
		strcat (outAlias, wholeName.At (x));
	}
	
	gLogFile << outAlias << endl_AC;
	
	// Now, we ask each of the aliases in turn what the name would be if we used their alias
	// If it is shorter, then we substitute it.
	
	for (int x = 1; x <= fEntries.GetSize (); x++) {
		char *result = fEntries.At (x)->abbreviate (&wholeName);
		if ((result[0] != 0) && (strlen (result) < strlen (outAlias))) {
			strcpy (outAlias, result);
		}
		DisposePtr (result);
	}
	
	for (int x = 1; x <= wholeName.GetSize (); x++) {
		DisposePtr (wholeName.At (x));
	}
	
	// Now, we construct the short alias by using the "@" where available
	
	strcpy (shortAlias, outAlias);
	
	char *at;
	char *start = shortAlias;
	while ((at = strstr (start, "@"))) {
		start = at;
		while ((*start != '/') && (start >= shortAlias)) start--;
		BlockMoveData (at, start + 1, strlen (at) + 1);
		start += 2;
	}
	
	RegistryEntryIDDispose (&deviceTreeEntry);
	
#endif
}
