/*

 Copyright (c) 2005
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

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as defined 
in and that are subject to the Apple Public Source License Version 2.0 (the 'License').  
You may not use this file except in compliance with the License.  Please obtain a copy 
of the License at http://www.opensource.apple.com/apsl/ and read it before using this file. 

The Original Code and all software distributed under the License are distributed on an 
'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND APPLE HEREBY 
DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see the License 
for the specific language governing rights and limitations under the License.

*/

#ifndef __OPENPMUNVRAMCONTROLLER_H__
#define __OPENPMUNVRAMCONTROLLER_H__

#include <IOKit/nvram/IONVRAMController.h>

class OpenPMUNVRAMController : public IONVRAMController
{

    OSDeclareDefaultStructors(OpenPMUNVRAMController)

private:

	void sendPMUCommand (UInt32 command, IOByteCount sendLength, UInt8 *sendBuffer, IOByteCount *readLength, UInt8 *readBuffer);

	// The PMUDdriver could be either ApplePMU or OpenPMU
    IOService	*fPMUdriver;

	UInt8		*fOldWorldBuffer;
	UInt8		*fNewWorldBuffer;

public:

    bool init (OSDictionary *properties, IOService *driver);
	bool start (IOService *provider);
	virtual	void free ();
	
    IOReturn read (IOByteCount offset, UInt8 *buffer, IOByteCount length);
    IOReturn write (IOByteCount offset, UInt8 *buffer, IOByteCount length);
	
};

#endif
