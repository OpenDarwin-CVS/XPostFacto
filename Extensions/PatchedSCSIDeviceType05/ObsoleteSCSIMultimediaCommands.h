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

#ifndef __OBSOLETESCSIMULTIMEDIACOMMANDS_H__
#define __OBSOLETESCSIMULTIMEDIACOMMANDS_H__

#include <IOKit/scsi-commands/SCSIMultimediaCommands.h>

class ObsoleteSCSIMultimediaCommands : public SCSIMultimediaCommands
{
	
	OSDeclareDefaultStructors ( ObsoleteSCSIMultimediaCommands )
		
public:

	// singleton accessor method
	static ObsoleteSCSIMultimediaCommands * CreateObsoleteSCSIMultimediaCommandObject ( void );
	
	virtual bool READ_CD (
				SCSITask *					request,
				IOMemoryDescriptor *		dataBuffer,
				SCSICmdField3Bit 			EXPECTED_SECTOR_TYPE,
				SCSICmdField1Bit 			RELADR,
				SCSICmdField4Byte 			STARTING_LOGICAL_BLOCK_ADDRESS,
				SCSICmdField3Byte 			TRANSFER_LENGTH,
				SCSICmdField1Bit 			SYNC,
				SCSICmdField2Bit 			HEADER_CODES,
				SCSICmdField1Bit 			USER_DATA,
				SCSICmdField1Bit 			EDC_ECC,
				SCSICmdField2Bit 			ERROR_FIELD,
				SCSICmdField3Bit 			SUBCHANNEL_SELECTION_BITS,
				SCSICmdField1Byte 			CONTROL );

	
};

#endif