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

/* 

XPFIODevice

Maintains a list of input and output devices (i.e. displays, serial ports, etc.)

*/

#ifndef __XPFIODEVICE_H__
#define __XPFIODEVICE_H__

#include "OFAliases.h"

class XPFIODevice;

typedef TemplateAutoList_AC <XPFIODevice> XPFIODeviceList;
typedef TemplateAutoList_AC <XPFIODevice>::Iterator XPFIODeviceIterator;

class XPFIODevice : public MDependable_AC
{
	public:
	
		static void Initialize ();
		static const XPFIODeviceList* GetInputDeviceList () {Initialize (); return &gInputDeviceList;}
		static const XPFIODeviceList* GetOutputDeviceList () {Initialize (); return &gOutputDeviceList;}
		
		static XPFIODevice *InputDeviceWithLabel (char *label);
		static XPFIODevice *InputDeviceWithShortOpenFirmwareName (char *ofName);
		static XPFIODevice *InputDeviceWithOpenFirmwareName (char *ofName);
		static XPFIODevice *OutputDeviceWithLabel (char *label);
		static XPFIODevice *OutputDeviceWithShortOpenFirmwareName (char *ofName);
		static XPFIODevice *OutputDeviceWithOpenFirmwareName (char *ofName);
		
		const CStr255_AC& getOpenFirmwareName (bool useShortName) {return useShortName ? fShortOpenFirmwareName : fOpenFirmwareName;}
		const CStr255_AC& getLabel () {return fLabel;}
					
	private:
	
		XPFIODevice (char *label, char *openFirmwareName, char *shortOpenFirmwareName);

		CStr255_AC fOpenFirmwareName;
		CStr255_AC fShortOpenFirmwareName;
		CStr255_AC fLabel;
				
		static XPFIODeviceList gInputDeviceList;
		static XPFIODeviceList gOutputDeviceList;
		
		static OSErr getRegistryProperty (REG_ENTRY_TYPE entry, char *key, char *value);
		static void EvaluateDevice (REG_ENTRY_TYPE entry);

};

#endif
