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


// =============
//  NVRAM
// =============

#ifndef __XPFNVRAMSETTINGS_H__
#define __XPFNVRAMSETTINGS_H__
	
#define kOFStringCapacity 1956

#include "NVRAMValue.h"
	
class XPFNVRAMSettings {

	public:

		virtual void readFromNVRAM () = 0;
		virtual int writeToNVRAM () = 0;
						
		static XPFNVRAMSettings* GetSettings ();

		NVRAMValue *getValue (const char *key);
		NVRAMValue *getOrCreateValue (const char *key);
		
		NVRAMValueType getValueType (const char *key);
		unsigned getOffset (const char *key);
		
		virtual char *getStringValue (const char *key);
		virtual bool getBooleanValue (const char *key);
		virtual long getNumericValue (const char *key);
		
		virtual bool setStringValue (const char *key, const char *value);
		virtual bool setBooleanValue (const char *key, const bool value);
		virtual bool setNumericValue (const char *key, const long value);

	protected:
	
		XPFNVRAMSettings ();
		virtual ~XPFNVRAMSettings ();	

		static XPFNVRAMSettings *gSettings;			
		
		bool fHasChanged;
		
		TemplateAutoList_AC <NVRAMValue> fNVRAMValues;			
};

#endif
