/*

 Portions Copyright (c) 1999-2001 Apple Computer, Inc. All Rights Reserved.

 This file contains Original Code and/or Modifications of Original Code
 as defined in and that are subject to the Apple Public Source License
 Version 1.2 (the 'License'). You may not use this file except in compliance
 with the License. Please obtain a copy of the License at
 http://www.apple.com/publicsource and read it before using this file.

 The Original Code and all software distributed under the License are
 distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see
 the License for the specific language governing rights and limitations
 under the License.

 */

#ifndef __PATCHEDNDRVFRAMEBUFFER_H__
#define __PATCHEDNDRVFRAMEBUFFER_H__

#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>

class PatchedNDRVFramebuffer : public IONDRVFramebuffer
{

	OSDeclareDefaultStructors(PatchedNDRVFramebuffer);

public:

    virtual IOService *probe (IOService *provider, SInt32 *score);

	virtual bool convertCursorImage (void *cursorImage, IOHardwareCursorDescriptor *hwDesc, IOHardwareCursorInfo *hwCursorInfo);
		
};

#endif