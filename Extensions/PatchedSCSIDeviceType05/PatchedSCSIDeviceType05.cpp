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

#define super IOSCSIPeripheralDeviceType05

#include "PatchedSCSIDeviceType05.h"
#include <IOKit/IOBufferMemoryDescriptor.h>

#define kModeSenseParameterHeaderSize		8

#if DEBUG
	#define STATUS_LOG(x) IOLog x
	#define ERROR_LOG(x) IOLog x
	#define PANIC_NOW(x) IOLog x
#else
	#define STATUS_LOG(x) 
	#define ERROR_LOG(x)
	#define PANIC_NOW(x)
#endif

OSDefineMetaClassAndStructors(PatchedSCSIDeviceType05, IOSCSIPeripheralDeviceType05)

IOReturn
PatchedSCSIDeviceType05::GetMechanicalCapabilities ( void )
{
	UInt8 *							mechanicalCapabilitiesPtr;
	IOBufferMemoryDescriptor *		bufferDesc;
	SCSIServiceResponse				serviceResponse = kSCSIServiceResponse_SERVICE_DELIVERY_OR_TARGET_FAILURE;
	SCSITaskIdentifier				request;
	UInt32							actualSize = 0;
	IOReturn						status;
	
	STATUS_LOG ( ("%s::%s called.\n", getName ( ), __FUNCTION__  ) );
	
	status = GetMechanicalCapabilitiesSize ( &actualSize );
	if ( status != kIOReturnSuccess ) {
	
		ERROR_LOG ( ( "GetMechanicalCapabilitiesSize returned error\n" ) );
		return status;
	
	}
			
	// in case we got junk from GetMechanicalCapabilitiesSize
	if (actualSize > 512) actualSize = 512;
	bufferDesc = IOBufferMemoryDescriptor::withCapacity ( 	actualSize,
															kIODirectionIn,
															true );
	
	if (!bufferDesc) return kIOReturnNoMemory;
		
	mechanicalCapabilitiesPtr = ( UInt8 * ) bufferDesc->getBytesNoCopy ( );
	bzero ( mechanicalCapabilitiesPtr, actualSize );
	
	request = GetSCSITask ( );
	
	if ( MODE_SENSE_10 ( 	request,
							bufferDesc,
							0x00,
							0x00,	// DBD
							0x00,
							0x2A,
							actualSize,
							0 ) == true )
	{
		// The command was successfully built, now send it
		serviceResponse = SendCommand ( request, 0 );
	}
	
	else
	{
		ERROR_LOG ( ("IOSCSIMultimediaCommandsDevice::GetMechanicalCapabilities malformed command" ) );
	}
	
	ReleaseSCSITask ( request );
	
	if ( serviceResponse != kSCSIServiceResponse_TASK_COMPLETE )
	{
		status = kIOReturnError;
	}
	
	if ( status == kIOReturnSuccess)
	{
		// ParseMechanicalCapabilities should be given a pointer to the page
		// data (skipping over the header, any block descriptor, and the first
		// two bytes of the page itself (which contain the page number and its
		// length). Really these two bytes should be sanity checked, but apple doesn't
		// check them either.
		// blockDescriptorLength is in bytes 6 and 7 of the mode sense header,
		// with the MSB in block 6. This is Mac byte order anyway, but the 
		// code below might one day be used on little-endian systems...
		
		if (mechanicalCapabilitiesPtr == 0)
		{
			status = kIOReturnError;
		}
		else
		{
			UInt16 blockDescriptorLength = ((*(mechanicalCapabilitiesPtr + 6)) << 8) +
										(*(mechanicalCapabilitiesPtr + 7));
			STATUS_LOG ( ("blockDescriptorLength = %d\n", blockDescriptorLength) );
		
			// we know that we're going to examine 4 bytes of page data, make sure
			// we don't fall off the end of our buffer
			if (actualSize >= (kModeSenseParameterHeaderSize + 2 + blockDescriptorLength + 4) )
			{
				status = ParseMechanicalCapabilities ( mechanicalCapabilitiesPtr + 
											kModeSenseParameterHeaderSize +
											2 +
											blockDescriptorLength);
			}
			else 
			{
				ERROR_LOG ( ("can't call ParseMechanicalCapabilities, only %d bytes to play with\n", actualSize) );
				status = kIOReturnError;
			}
			STATUS_LOG ( ("ParseMechanicalCapabilities returned 0x%x\n", status) );
		}
				
	}
	
	bufferDesc->release ( );
		
	return status;
}

