/*

Copyright (c) 2001 - 2003
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


#include "FirewireDevice.h"
#include "XPFPartition.h"
#include "MoreFilesExtras.h"
#include "SCSIBus.h"
#include "MountedVolume.h"

#include "XPFLog.h"
#include "XPFErrors.h"
#include "OFAliases.h"
#include "XPostFacto.h"

#include "MoreDisks.h"

void
FirewireDevice::Initialize ()
{
	// Now the idea is to find all the Firewire devices we can
	
	#if qLogging
		gLogFile << "Finding Firewire devices" << endl_AC;
	#endif

	RegEntryIter cookie;
    RegEntryID entry;
    Boolean done = false;
    RegEntryIterationOp iterOp = kRegIterDescendants;
    OSStatus err = RegistryEntryIterateCreate (&cookie);

	// We iterate through, and collect all entries with an fw-mao property 
    while (true) {
        err = RegistryEntryIterate (&cookie, iterOp, &entry, &done);
        if (!done && (err == noErr)) {
  			RegPropertyValueSize propSize;
  			err = RegistryPropertyGetSize (&entry, "fw-mao", &propSize);
  			if (err == noErr) {
  				// Figure out the driverRefNum
  				err = RegistryPropertyGetSize (&entry, "driver-ref", &propSize);
  				if ((err == noErr) && (propSize == 2)) {
  					SInt16 driverRefNum;
  					ThrowIfOSErr_AC (RegistryPropertyGet (&entry, "driver-ref", &driverRefNum, &propSize));
					XPFBootableDevice *oldBus = DeviceWithDriverRefNum (driverRefNum);
					if (!oldBus) {
						gDeviceList.InsertLast (new FirewireDevice (&entry, driverRefNum));	
					}
				}
	      		RegistryEntryIDDispose (&entry);
	      	}
        } else {
        	break;
        }
        iterOp = kRegIterContinue;
    }
    RegistryEntryIterateDispose (&cookie);
}

void
FirewireDevice::checkOpenFirmwareName ()
{
}

FirewireDevice::FirewireDevice (RegEntryID *regEntry, SInt16 driverRefNum)
	: XPFBootableDevice (driverRefNum)
{
	fNeedsHelper = true;

	OFAliases::AliasFor (regEntry, fOpenFirmwareName, fShortOpenFirmwareName);

	#if qLogging
		gLogFile << "OpenFirmwareName: " << fOpenFirmwareName << endl_AC;
		gLogFile << "ShortOpenFirmwareName: " << fShortOpenFirmwareName << endl_AC;
	#endif;
	
	Changed (cSetOpenFirmwareName, this);
}

bool
FirewireDevice::isFirewireDevice ()
{
	return true;
}

void 
FirewireDevice::extractPartitionInfo ()
{
	// don't need to do this
}

FirewireDevice::~FirewireDevice ()
{

}

void
FirewireDevice::readCapacity ()
{
	// don't need to do this
}


OSErr
FirewireDevice::writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer)
{
	#pragma unused (start, count, buffer)
	// not supported
	return paramErr;
}

OSErr
FirewireDevice::readBlocks (unsigned int start, unsigned int count, UInt8 **buffer)
{
	#pragma unused (start, count, buffer)
	// not supported
	return paramErr;
}
