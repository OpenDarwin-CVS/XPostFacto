/*

Copyright (c) 2001, 2002, 2005
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


// ========================================
// Name Registry NVRAM - For New World Macs
// ========================================

#include "NameRegistryNVRAM.h"
#include "XPFLog.h"

void
NameRegistryNVRAM::readFromNVRAM ()
{
	RegEntryID options;
	ThrowIfOSErr_AC (RegistryEntryIDInit (&options));	
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree:options", &options));

	RegPropertyIter cookie;
	Boolean done = false;

	ThrowIfOSErr_AC (RegistryPropertyIterateCreate (&options, &cookie));
	while (!done) {
		RegPropertyNameBuf key;
		ThrowIfOSErr_AC (RegistryPropertyIterate (&cookie, key, &done));
		// only process values we're interested in
		if (!done && getValue (key)) {
			char *value;
			RegPropertyValueSize size;
			ThrowIfOSErr_AC (RegistryPropertyGetSize (&options, key, &size));
			value = NewPtr (size + 1);
			ThrowIfOSErr_AC (RegistryPropertyGet (&options, key, value, &size));
			value[size] = 0;
			
			setStringValue (key, value);

			DisposePtr (value);
		}
	}

	RegistryPropertyIterateDispose(&cookie);
}

int
NameRegistryNVRAM::writeToNVRAM ()
{
	char *stringValue;
	
	NVRAMValue *current;
	RegEntryID options;
	
	ThrowIfOSErr_AC (RegistryEntryIDInit (&options));	
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree:options", &options));

	for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {
		stringValue = current->getStringValue ();
		RegistryPropertySet (&options, current->getName(), stringValue, strlen (stringValue) + 1);
		RegistryPropertySetMod (&options, current->getName(), kRegPropertyValueIsSavedToNVRAM);
	}
	
	return noErr;
}