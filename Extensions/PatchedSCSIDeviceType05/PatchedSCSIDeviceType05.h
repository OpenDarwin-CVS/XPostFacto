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

// This is a subclass of IOSCSIPeripheralDeviceType05 that fixes a problem with
// SCSI CD-ROM drives (as well as CD-R, CD-RW, DVD, DVD-R etc.). It was originally
// written by El Gato software (http://www.elgato.com), and put into this form by
// Ryan Rempel (ryan.rempel@utoronto.ca).

#include <IOKit/scsi-commands/IOSCSIPeripheralDeviceType05.h>

class PatchedSCSIDeviceType05 : public IOSCSIPeripheralDeviceType05
{

    OSDeclareDefaultStructors(PatchedSCSIDeviceType05)
		
    protected:
	
        virtual IOReturn GetMechanicalCapabilities ( void );

};