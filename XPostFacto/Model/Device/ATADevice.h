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

/*

	ATADevice
	==========

	The purpose of this class is to model a ATA device (i.e. a physical drive).
	It constructs a list of partitions.

*/

#ifndef __ATADEVICE_H__
#define __ATADEVICE_H__

#include "XPFBootableDevice.h"
#include "ATABus.h"

class ATADevice : public XPFBootableDevice
{
	public:
		static void Initialize ();
		static bool ATAHardwarePresent ();
		
		OSErr readBlocks (unsigned int start, unsigned int count, UInt8 **buffer);
		OSErr writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer);
		
		bool isReallyATADevice () {return true;}
		
		virtual CVoidList_AC* getBusList () {return ATABus::GetBusList ();}

	protected:
	
		void checkOpenFirmwareName ();

	private:	

		ATADevice (UInt32 ataDevice, SInt16 driverRefNum);
		OSErr readATAPIBlocks (unsigned int start, unsigned int count, UInt8 *buffer);
		OSErr readATABlocks (unsigned int start, unsigned int count, UInt8 *buffer);
	
		bool atapiTestUnitReady ();
		void readCapacity ();

		UInt32	fDeviceIdent;
		UInt8 fATADeviceType;
		UInt8 fATASocketType;
		bool fUseLBA;
		
		unsigned long fBlockSize;
		unsigned long fBlockCount;
		unsigned long fHeads;
		unsigned long fSectors; 
		
};

#endif
