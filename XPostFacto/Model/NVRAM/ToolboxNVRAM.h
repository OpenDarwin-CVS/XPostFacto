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

#ifndef __TOOLBOXNVRAM_H__
#define __TOOLBOXNVRAM_H__

#include "NVRAM.h"
#include "MixedMode.h"

#define kOFPartitionOffset 	0x1800
#define kOFPartitionSize	0x0800

#define kOSPartitionOffset 	0x0
#define kOSPartitionSize	0x0C00

class ToolboxNVRAM : public XPFNVRAMSettings {

	struct NVRAMString {
		UInt16 offset;
		UInt16 length;
	};

	struct NVRAMBuffer {
		UInt16   magic;   	// 0x1275
		UInt8    version;	// 0x05
		UInt8    pages;		// 0x08
		UInt16   checksum;
		UInt16   here;
		UInt16   top;		
		UInt16   next;		// 0x0000
		
		UInt32   flags;
		
		long numericValues[9];
		
		NVRAMString stringValues[10];

		char strings[kOFStringCapacity];
	};

	public:
	
		virtual void readFromNVRAM ();
		virtual int writeToNVRAM ();
		
	protected:
	
		virtual UInt8 readByte (unsigned offset);
		virtual void writeByte (unsigned offset, UInt8 byte);

		void packStrings ();
		void packString (NVRAMString &string, char *cache);

	private:
					
		NVRAMBuffer fBuffer;		
};

#endif
