/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as 
defined in and that are subject to the Apple Public Source License Version 2.0 
(the 'License').  You may not use this file except in compliance with the License.  
Please obtain a copy of the License at http://www.opensource.apple.com/apsl/ and 
read it before using this file. 

The Original Code and all software distributed under the License are distributed 
on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND 
APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT 
OR NON-INFRINGEMENT. Please see the License for the specific language governing 
rights and limitations under the License.

*/

#ifndef __PATCHEDBLOCKSTORAGEDRIVER_H__
#define __PATCHEDBLOCKSTORAGEDRIVER_H__

#include <libkern/OSTypes.h>
#include "IOBlockStorageDriver.h"	// our copy, with a protected _mediaStateLock
#include <IOKit/storage/IOCDBlockStorageDriver.h>

class PatchedBlockStorageDriver : public IOBlockStorageDriver
{

	OSDeclareDefaultStructors(PatchedBlockStorageDriver);

protected:

    virtual IOReturn checkForMedia (void);
	
};

class PatchedCDBlockStorageDriver : public IOCDBlockStorageDriver
{

	OSDeclareDefaultStructors(PatchedCDBlockStorageDriver);

protected:

    virtual IOReturn checkForMedia (void);
	
};

#endif