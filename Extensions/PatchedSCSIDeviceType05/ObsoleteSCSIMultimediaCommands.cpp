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

#include "ObsoleteSCSIMultimediaCommands.h"
#include <IOKit/storage/IOCDTypes.h>

#define super SCSIMultimediaCommands

#if DEBUG
	#define STATUS_LOG(x) IOLog x
	#define ERROR_LOG(x) IOLog x
	#define PANIC_NOW(x) IOLog x
	#define DEBUG_ASSERT(x)
#else
	#define STATUS_LOG(x) 
	#define ERROR_LOG(x) 
	#define PANIC_NOW(x)
	#define DEBUG_ASSERT(x)
#endif

OSDefineMetaClassAndStructors ( ObsoleteSCSIMultimediaCommands, SCSIMultimediaCommands );


ObsoleteSCSIMultimediaCommands *
ObsoleteSCSIMultimediaCommands::CreateObsoleteSCSIMultimediaCommandObject ( void )
{

	return new ObsoleteSCSIMultimediaCommands;

}

bool
ObsoleteSCSIMultimediaCommands::READ_CD (
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
				SCSICmdField1Byte 			CONTROL )
{
	UInt32			blockSize;
	UInt32 			requestedByteCount;
	bool			validBlockSize;
	
	if (EXPECTED_SECTOR_TYPE != kCDSectorTypeCDDA) return super::READ_CD (request, dataBuffer, 	EXPECTED_SECTOR_TYPE, RELADR, STARTING_LOGICAL_BLOCK_ADDRESS, TRANSFER_LENGTH, SYNC, 	HEADER_CODES, USER_DATA, EDC_ECC, ERROR_FIELD, SUBCHANNEL_SELECTION_BITS, CONTROL);
	
	STATUS_LOG ( ( "PatchedSCSIMultimediaCommands::READ_CD called\n" ) );
	DEBUG_ASSERT ( ( request != NULL ) );
	
	// did we receive a valid RELADR?
	if ( RELADR != 0 )
	{

		ERROR_LOG ( ( "RELADR = %x not valid \n", RELADR ) );
		return false;

	}

	// did we receive a valid STARTING_LOGICAL_BLOCK_ADDRESS?
	if ( IsParameterValid ( STARTING_LOGICAL_BLOCK_ADDRESS,
							kSCSICmdFieldMask4Byte ) == false )
	{
		
		ERROR_LOG ( ( "STARTING_LOGICAL_BLOCK_ADDRESS = %x not valid \n",
						STARTING_LOGICAL_BLOCK_ADDRESS ) );
		return false;
		
	}

	// did we receive a valid TRANSFER_LENGTH?
	if ( IsParameterValid ( TRANSFER_LENGTH, kSCSICmdFieldMask3Byte ) == false )
	{
		
		ERROR_LOG ( ( "TRANSFER_LENGTH = %x not valid \n",
						TRANSFER_LENGTH ) );
		return false;
		
	}

	// did we receive a valid SUBCHANNEL_SELECTION_BITS?
	if ( IsParameterValid ( SUBCHANNEL_SELECTION_BITS,
							kSCSICmdFieldMask3Bit ) == false )
	{
		
		ERROR_LOG ( ( "SUBCHANNEL_SELECTION_BITS = %x not valid \n",
						SUBCHANNEL_SELECTION_BITS ) );
		return false;
		
	}
	
	// did we receive a valid CONTROL?
	if ( IsParameterValid ( CONTROL, kSCSICmdFieldMask1Byte ) == false )
	{
		
		ERROR_LOG ( ( "CONTROL = %x not valid \n", CONTROL ) );
		return false;
		
	}
	
	// determine the size of the transfer
	validBlockSize = GetBlockSize (	&blockSize,
									EXPECTED_SECTOR_TYPE,
									SYNC,
									HEADER_CODES,
									USER_DATA,
									EDC_ECC,
									ERROR_FIELD );
	
	if ( validBlockSize == false )
	{
		
		ERROR_LOG ( ( "blockSize = %x not valid \n", blockSize ) );
		return false;
		
	}
	
	requestedByteCount = TRANSFER_LENGTH * blockSize;
	
	STATUS_LOG ( ( "blockSize = %ld\n", blockSize ) );
	STATUS_LOG ( ( "TRANSFER_LENGTH = %ld\n", TRANSFER_LENGTH ) );
	STATUS_LOG ( ( "requestedByteCount = %ld\n", requestedByteCount ) );

	if ( IsBufferAndCapacityValid ( dataBuffer, requestedByteCount ) == false )
	{
		
		ERROR_LOG ( ( "dataBuffer = %x not valid, requestedByteCount = %ld\n",
						dataBuffer, requestedByteCount ) );
		return false;
		
	}
	
	SetCommandDescriptorBlock (request,
								0xD8,   // Read CD_DA
								0x00,
								( STARTING_LOGICAL_BLOCK_ADDRESS >> 24 ) & 0xFF,
								( STARTING_LOGICAL_BLOCK_ADDRESS >> 16 ) & 0xFF,
								( STARTING_LOGICAL_BLOCK_ADDRESS >>  8 ) & 0xFF,
								  STARTING_LOGICAL_BLOCK_ADDRESS         & 0xFF,
								0x00,
								( TRANSFER_LENGTH >> 16 ) & 0xFF,
								( TRANSFER_LENGTH >>  8 ) & 0xFF,
								  TRANSFER_LENGTH         & 0xFF,
								0x00,
								0x00);
/*	
	// fill out the cdb appropriately  
	SetCommandDescriptorBlock (	request,
								kSCSICmd_READ_CD,
								( EXPECTED_SECTOR_TYPE << 2 ) | RELADR,
								( STARTING_LOGICAL_BLOCK_ADDRESS >> 24 ) & 0xFF,
								( STARTING_LOGICAL_BLOCK_ADDRESS >> 16 ) & 0xFF,
								( STARTING_LOGICAL_BLOCK_ADDRESS >>  8 ) & 0xFF,
								  STARTING_LOGICAL_BLOCK_ADDRESS         & 0xFF,
								( TRANSFER_LENGTH >> 16 ) & 0xFF,
								( TRANSFER_LENGTH >>  8 ) & 0xFF,
								  TRANSFER_LENGTH         & 0xFF,
								( SYNC << 7 ) | ( HEADER_CODES << 5 ) | ( USER_DATA << 4 ) | ( EDC_ECC << 3 ) | ( ERROR_FIELD << 1 ),
								SUBCHANNEL_SELECTION_BITS,
								CONTROL );
*/
	
	SetDataTransferControl ( 	request,
                           		0,
								kSCSIDataTransfer_FromTargetToInitiator,
								dataBuffer,
								requestedByteCount );	
	
	return true;
	
}

