/*

Copyright (c) 2003
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

#include "XPFIODevice.h"
#include "XPFLog.h"

XPFIODeviceList XPFIODevice::gInputDeviceList;
XPFIODeviceList XPFIODevice::gOutputDeviceList;

OSErr
XPFIODevice::getRegistryProperty (REG_ENTRY_TYPE entry, char *key, char *value)
{
	value[0] = 0;

#ifdef __MACH__

	CFStringRef cfKey = CFStringCreateWithCString (NULL, key, kCFStringEncodingMacRoman);
	CFTypeRef cfValue = IORegistryEntryCreateCFProperty (entry, cfKey, NULL, 0);
 
	if (!cfValue) return -1;
	if (CFGetTypeID (cfValue) == CFDataGetTypeID ()) {
		memcpy (value, CFDataGetBytePtr ((CFDataRef) cfValue), CFDataGetLength ((CFDataRef) cfValue));
		value [CFDataGetLength ((CFDataRef) cfValue)] = 0;
		CFRelease (cfValue);
	}
	return noErr;

#else

	RegPropertyValueSize propSize;
	OSErr err = RegistryPropertyGetSize (entry, key, &propSize);
	if (err == noErr) {
		RegistryPropertyGet (entry, key, value, &propSize);
		value[propSize] = '\0';
	}
	return err;
	
#endif
}

XPFIODevice*
XPFIODevice::InputDeviceWithLabel (char *label)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetInputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getLabel () == label) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

XPFIODevice*
XPFIODevice::OutputDeviceWithLabel (char *label)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetOutputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getLabel () == label) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

XPFIODevice *
XPFIODevice::InputDeviceWithShortOpenFirmwareName (char *ofName)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetInputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getOpenFirmwareName (true) == ofName) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

XPFIODevice *
XPFIODevice::OutputDeviceWithShortOpenFirmwareName (char *ofName)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetOutputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getOpenFirmwareName (true) == ofName) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

XPFIODevice *
XPFIODevice::InputDeviceWithOpenFirmwareName (char *ofName)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetInputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getOpenFirmwareName (false) == ofName) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

XPFIODevice *
XPFIODevice::OutputDeviceWithOpenFirmwareName (char *ofName)
{
	XPFIODevice *retVal = NULL;
	for (XPFIODeviceIterator iter (GetOutputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->getOpenFirmwareName (false) == ofName) {
			retVal = iter.Current ();
			break;
		}		
	}	
	return retVal;
}

void
XPFIODevice::EvaluateDevice (REG_ENTRY_TYPE entry)
{
	char deviceType[256];
	getRegistryProperty (entry, "device_type", deviceType);
			
	if 	(	!strcmp (deviceType, "serial"	) || 
			!strcmp (deviceType, "display"	) || 
			!strcmp (deviceType, "keyboard"	)) 
	{
		char alias[256];
		char shortAlias[256];
		char label[256];
		char displayType[256];

		OFAliases::AliasFor (entry, alias, shortAlias);
		if (!strcmp (alias, "/offscreen-display")) return;
		
		OSErr err = getRegistryProperty (entry, "AAPL,connector", label);
		if (err != noErr) {
			err = getRegistryProperty (entry, "name", label);
			if (err != noErr) {
				strcpy (label, alias);
			}
		}
		
		if (!strcmp (label, "infrared")) return;
	
		err = getRegistryProperty (entry, "display-type", displayType);
		if (err == noErr) {
			if (!strcmp (displayType, "NONE")) return;
			strcat (label, " (");
			strcat (label, displayType);
			strcat (label, ")");
		}
		
		char buffer[16];
		bool active = (getRegistryProperty (entry, "driver-ref", buffer) == noErr);
		
		XPFIODevice *device = new XPFIODevice (label, alias, shortAlias, deviceType, active);
		
		if (!strcmp (alias, "kbd") || !strcmp (deviceType, "keyboard")) {
			gInputDeviceList.InsertLast (device);
		} else if (!strcmp (deviceType, "serial")) {
			// Need a separate object for the two lists, because I'm not doing any
			// reference counting, so one object would get deleted twice otherwise.
			gInputDeviceList.InsertLast (device);
			gOutputDeviceList.InsertLast (new XPFIODevice (label, alias, shortAlias, deviceType, active));
		} else if (!strcmp (deviceType, "display")) {
			gOutputDeviceList.InsertLast (device);
		}
	}
}

XPFIODevice::XPFIODevice (char *label, char *alias, char *shortAlias, char* deviceType, bool active) 
{
	fOpenFirmwareName.CopyFrom (alias);
	fShortOpenFirmwareName.CopyFrom (shortAlias);
	fLabel.CopyFrom (label);
	fDeviceType.CopyFrom (deviceType);
	fActive = active;
}

XPFIODevice*
XPFIODevice::GetDefaultOutputDevice ()
{
	Initialize ();
	for (XPFIODeviceIterator iter (GetOutputDeviceList ()); iter.Current(); iter.Next()) {
		if (iter->fDeviceType != "display") continue;
		if (!iter->fActive) continue;
		return iter.Current ();
	}	
	return NULL;
}

void
XPFIODevice::Initialize ()
{
	static bool doneOnce = false;
	if (doneOnce) return;
	doneOnce = true;
	
	#if qLogging
		gLogFile << "Initializing Input and Output Devices" << endl_AC;
	#endif

#if __MACH__

	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_iterator_t iterator = NULL;
	IORegistryCreateIterator (iokitPort, kIODeviceTreePlane, kIORegistryIterateRecursively, &iterator);
	if (!iterator) return;
	io_object_t entry;
	while ((entry = IOIteratorNext (iterator)) != NULL) {
		EvaluateDevice (entry);
		IOObjectRelease (entry);
	}
	IOObjectRelease (iterator);

#else

	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	try {
	    while (true) {
	        err = RegistryEntryIterate (&cookie, iterOp, &entry, &done);
	        if (!done && (err == noErr)) {
				EvaluateDevice (&entry);
		        RegistryEntryIDDispose (&entry);
	        } else {
	        	break;
	        }
	        iterOp = kRegIterContinue;
	    }
	} 
	catch (...) {
	}
	RegistryEntryIterateDispose (&cookie);
	
#endif

}

