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

/*

	SCSIDevice
	==========

	The purpose of this class is to model a SCSI device (i.e. a physical drive).
	It constructs a list of partitions.

*/

#ifndef __SCSIDEVICE_H__
#define __SCSIDEVICE_H__

#include "XPFBootableDevice.h"
#include "SCSIBus.h"
#include <SCSI.h>

class SCSIDevice : public XPFBootableDevice
{
	public:
		
		static void Initialize ();
		
		virtual ~SCSIDevice ();
				
		OSErr readBlocks (unsigned int start, unsigned int count, UInt8 **buffer);
		OSErr writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer);
		
		bool isReallyATADevice () {return fSCSIBus && fSCSIBus->getIsActuallyATABus ();} 
		
	private:	

		SCSIDevice (DeviceIdent scsiDevice, SInt16 driverRefNum);
	
		void readCapacity ();

		DeviceIdent	fDeviceIdent;
		SCSIExecIOPB *fPB;
		long fPBLength;
		
		SCSIBus *fSCSIBus;

};

#endif