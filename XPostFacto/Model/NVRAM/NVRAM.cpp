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

// ===================
// NVRAM
// ===================

#include "NVRAM.h"

#include "XPFLog.h"
#include "XPFErrors.h"

#if __MACH__
	#include "OSXNVRAM.h"
#else
	#include "ToolboxNVRAM.h"
#endif

XPFNVRAMSettings *XPFNVRAMSettings::gSettings = NULL;

XPFNVRAMSettings*
XPFNVRAMSettings::GetSettings ()
{
	// The idea here is to either return the already constructed singleton,
	// or to figure out which subclass to use and return it.
	if (!gSettings) {
		#ifdef __MACH__
			gSettings = new OSXNVRAM;
		#else
			gSettings = new ToolboxNVRAM;
		#endif
	}
	return gSettings;
}

XPFNVRAMSettings::XPFNVRAMSettings ()
{	
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("little-endian?", 0));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("real-mode?", 1));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("auto-boot?", 2));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("diag-switch?", 3));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("fcode-debug?", 4));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("oem-banner?", 5));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("oem-logo?", 6));
	fNVRAMValues.InsertLast (new NVRAMBooleanValue ("use-nvramrc?", 7));

	fNVRAMValues.InsertLast (new NVRAMNumericValue ("real-base", 0));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("real-size", 1));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("virt-base", 2));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("virt-size", 3));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("load-base", 4));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("pci-probe-list", 5));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("screen-#columns", 6));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("screen-#rows", 7));
	fNVRAMValues.InsertLast (new NVRAMNumericValue ("selftest-#megs", 8));

	fNVRAMValues.InsertLast (new NVRAMStringValue ("boot-device", 0));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("boot-file", 1));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("diag-device", 2));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("diag-file", 3));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("input-device", 4));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("output-device", 5));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("oem-banner", 6));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("oem-logo", 7));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("nvramrc", 8));
	fNVRAMValues.InsertLast (new NVRAMStringValue ("boot-command", 9));
						
	fHasChanged = false;
}

XPFNVRAMSettings::~XPFNVRAMSettings ()
{

}

NVRAMValue*
XPFNVRAMSettings::getValue (const char *key)
{
	NVRAMValue *current;
	for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {
		if (!strcmp (current->getName (), key)) return current;
	}
	return NULL;
}

NVRAMValue*
XPFNVRAMSettings::getOrCreateValue (const char *key)
{
	NVRAMValue *value = getValue (key);
	if (value == NULL) {
		value = new NVRAMStringValue (key, 99);
		fNVRAMValues.InsertLast (value);
	}
	return value;
}
		
NVRAMValueType 
XPFNVRAMSettings::getValueType (const char *key)
{
	return getValue (key)->getValueType ();
}

unsigned
XPFNVRAMSettings::getOffset (const char *key)
{
	return getValue (key)->getOffset ();
}
		
char *
XPFNVRAMSettings::getStringValue (const char *key)
{
	return getValue (key)->getStringValue ();
}

bool 
XPFNVRAMSettings::getBooleanValue (const char *key)
{
	return getValue (key)->getBooleanValue ();
}

long 
XPFNVRAMSettings::getNumericValue (const char *key)
{
	return getValue (key)->getNumericValue ();
}
		
bool 
XPFNVRAMSettings::setStringValue (const char *key, const char *value)
{
	NVRAMValue *nv = getValue (key);
	if (nv == NULL) {
		nv = new NVRAMStringValue (key, 99);
		fNVRAMValues.InsertLast (nv);
	}
	bool result = nv->setStringValue (value);
	if (result) fHasChanged = true;
	return result;
}

bool 
XPFNVRAMSettings::setBooleanValue (const char *key, const bool value)
{
	NVRAMValue *nv = getValue (key);
	if (nv == NULL) {
		nv = new NVRAMBooleanValue (key, 99);
		fNVRAMValues.InsertLast (nv);
	}
	bool result = nv->setBooleanValue (value);
	if (result) fHasChanged = true;
	return result;
}

bool 
XPFNVRAMSettings::setNumericValue (const char *key, const long value)
{
	NVRAMValue *nv = getValue (key);
	if (nv == NULL) {
		nv = new NVRAMNumericValue (key, 99);
		fNVRAMValues.InsertLast (nv);
	}
	bool result = nv->setNumericValue (value);
	if (result) fHasChanged = true;
	return result;
}
