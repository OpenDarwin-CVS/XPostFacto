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

#define super IOSCSIPeripheralDeviceType05

#include "ObsoleteSCSIDeviceType05.h"
#include <IOKit/IOBufferMemoryDescriptor.h>

#if DEBUG
	#define STATUS_LOG(x) IOLog x
	#define ERROR_LOG(x) IOLog x
	#define PANIC_NOW(x) IOLog x
#else
	#define STATUS_LOG(x) 
	#define ERROR_LOG(x)
	#define PANIC_NOW(x)
#endif

OSDefineMetaClassAndStructors(ObsoleteSCSIDeviceType05, IOSCSIPeripheralDeviceType05)

bool 
ObsoleteSCSIDeviceType05::init(OSDictionary * propTable)
{
	fObsoleteSCSIMultimediaCommandObject = NULL;
	return super::init (propTable);
}

bool
ObsoleteSCSIDeviceType05::CreateCommandSetObjects ( void )
{
	fObsoleteSCSIMultimediaCommandObject = ObsoleteSCSIMultimediaCommands::CreateObsoleteSCSIMultimediaCommandObject ( );
	return super::CreateCommandSetObjects ();
}

void
ObsoleteSCSIDeviceType05::FreeCommandSetObjects ( void )
{
	super::FreeCommandSetObjects ();
	if (fObsoleteSCSIMultimediaCommandObject) {
		fObsoleteSCSIMultimediaCommandObject->release ();
		fObsoleteSCSIMultimediaCommandObject = NULL;
	}
}

bool
ObsoleteSCSIDeviceType05::READ_CD (
						SCSITaskIdentifier			request,
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
						SCSICmdField1Byte 			CONTROL )
{	
	SCSITask *	scsiRequest;
		
	STATUS_LOG ( ( "%s::%s called\n", getName ( ), __FUNCTION__ ) );
	
	scsiRequest = OSDynamicCast ( SCSITask, request );
	if ( scsiRequest == NULL )
	{
		
		PANIC_NOW ( ( "IOSCSIMultimediaCommandsDevice::READ_CD invalid SCSITaskIdentifier.\n" ) );
		ERROR_LOG ( ( "%s::%s invalid SCSITaskIdentifier.\n", getName ( ), __FUNCTION__ ) );
		return false;
		
	}

	if ( scsiRequest->ResetForNewTask ( ) == false )
	{
		
		ERROR_LOG ( ( "ResetForNewTask on the request SCSITask failed.\n" ) );
		return false;
		
	}
	
	return fObsoleteSCSIMultimediaCommandObject->READ_CD (
								scsiRequest,
								dataBuffer,
								EXPECTED_SECTOR_TYPE,
								RELADR,
								STARTING_LOGICAL_BLOCK_ADDRESS,
								TRANSFER_LENGTH,
								SYNC,
								HEADER_CODES,
								USER_DATA,
								EDC_ECC,
								ERROR_FIELD,
								SUBCHANNEL_SELECTION_BITS,
								CONTROL );
	
}

IOReturn
ObsoleteSCSIDeviceType05::GetMechanicalCapabilities ( void )
{
	// If our superclass (or it's patched version) had succeeded here, then 
	// we wouldn't be called at all. So we'll just succeed.

	fSupportedCDFeatures |= kCDFeaturesAnalogAudioMask | kCDFeaturesReadStructuresMask;
	
	return kIOReturnSuccess;
}

