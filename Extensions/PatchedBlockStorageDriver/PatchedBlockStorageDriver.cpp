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

#include "PatchedBlockStorageDriver.h"

#include <IOKit/IOLib.h>

#define super IOBlockStorageDriver

OSDefineMetaClassAndStructors(PatchedBlockStorageDriver, IOBlockStorageDriver);

IOReturn	
PatchedBlockStorageDriver::checkForMedia (void)
{
    IOReturn result;
    bool currentState;
    bool changed;
    
    IOLockLock (_mediaStateLock);    

    result = getProvider()->reportMediaState (&currentState, &changed);
	changed = currentState ? !_mediaPresent : _mediaPresent;
	
    if (result != kIOReturnSuccess) {		/* the poll operation failed */
        IOLog("%s[IOBlockStorageDriver]::checkForMedia; err '%s' from reportMediaState\n", getName(), stringFromReturn (result));
    } else if (changed) {	/* the poll succeeded, media state has changed */
        result = mediaStateHasChanged (currentState ? kIOMediaStateOnline : kIOMediaStateOffline);
    }

    IOLockUnlock (_mediaStateLock);
    return (result);
}

#undef super

#define super IOCDBlockStorageDriver

OSDefineMetaClassAndStructors(PatchedCDBlockStorageDriver, IOCDBlockStorageDriver);

IOReturn	
PatchedCDBlockStorageDriver::checkForMedia (void)
{
    IOReturn result;
    bool currentState;
    bool changed;
    
    IOLockLock (_mediaStateLock);    

    result = getProvider()->reportMediaState (&currentState, &changed);
	if (!changed) {
		changed = currentState ? !_mediaPresent : _mediaPresent;
		if (changed) setProperty ("Bug fix applied", "");
	}
	
    if (result != kIOReturnSuccess) {		/* the poll operation failed */
        IOLog("%s[IOBlockStorageDriver]::checkForMedia; err '%s' from reportMediaState\n", getName(), stringFromReturn (result));
    } else if (changed) {	/* the poll succeeded, media state has changed */
        result = mediaStateHasChanged (currentState ? kIOMediaStateOnline : kIOMediaStateOffline);
    }

    IOLockUnlock (_mediaStateLock);
    return (result);
}

