/*

Copyright (c) 2001, 2002
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


#include "SCSIDevice.h"
#include "XPFPartition.h"
#include "MoreFilesExtras.h"
#include "SCSIBus.h"
#include "MountedVolume.h"

#include <DriverGestalt.h>
#include <stdio.h>

#include "XPFLog.h"
#include "XPFErrors.h"

#if !qCarbonMach0

union DriverGestaltInfo
{
	DriverGestaltSyncResponse		sync;
	DriverGestaltBootResponse		boot;
	DriverGestaltDevTResponse		devt;
	DriverGestaltIntfResponse		intf;
	DriverGestaltEjectResponse		ejec;
	DriverGestaltPowerResponse		powr;
	DriverGestaltFlushResponse		flus;
	DriverGestaltOFBootSupportResponse ofbt;
	DriverGestaltNameRegistryResponse nmrg;
	DriverGestaltDeviceReferenceResponse dvrf;
	DriverGestaltAPIResponse		dAPI;
	UInt32							i;
};
typedef union DriverGestaltInfo DriverGestaltInfo;

#endif

void
SCSIDevice::Initialize ()
{
	SCSIBus::Initialize ();
	if (SCSIBus::getBusCount () == 0) return;
	
	// Now the idea is to find all the SCSI devices we can
	
	#if qLogging
		gLogFile << "Finding SCSI devices" << endl_AC;
	#endif

	SCSIDriverPB driverPB;
	Erase_AC (&driverPB);

	driverPB.scsiPBLength = sizeof (SCSIDriverPB);
	driverPB.scsiCompletion = NULL;
	driverPB.scsiFlags = 0;
#if qCarbonMach0
	return;
#else
	driverPB.scsiFunctionCode = SCSILookupRefNumXref;
#endif
	* ((long *) &driverPB.scsiDevice) = 0xFFFFFFFFL;
	
	do {
		OSErr status = SCSIAction ((SCSI_PB *) &driverPB);
		if (status != noErr) {
			break;
		} else if (driverPB.scsiDevice.bus != 0xFF) {
			gDeviceList.InsertLast (new SCSIDevice (driverPB.scsiDevice, driverPB.scsiDriver));
		}
		driverPB.scsiDevice = driverPB.scsiNextDevice;
	} while (driverPB.scsiDevice.bus != 0xFF);	
}

SCSIDevice::SCSIDevice (DeviceIdent scsiDevice, SInt16 driverRefNum)
	: XPFBootableDevice (driverRefNum)
{
	fValidOpenFirmwareName = false;
	
	fDeviceIdent = scsiDevice;
	
	fPB = NULL;
	fPBLength = 0;
	fInvalid = false;
	
	#if qLogging
		gLogFile << "Creating SCSIDevice" << endl_AC;
	#endif
	
	// Figure out the length for the pb
	{
	    SCSIBusInquiryPB busInquiryPB;
		Erase_AC (&busInquiryPB);

		busInquiryPB.scsiPBLength = sizeof busInquiryPB;
		busInquiryPB.scsiFunctionCode = SCSIBusInquiry;
		busInquiryPB.scsiDevice.bus = 0xFF;     
		
		OSErr status = SCSIAction((SCSI_PB *) &busInquiryPB);
		
		if (status == noErr) status = busInquiryPB.scsiResult;
		ThrowIfOSErr_AC (status);
		fPBLength = busInquiryPB.scsiMaxIOpbSize;
	    fPB = (SCSIExecIOPB *) NewPtr (fPBLength);
		ThrowIfNULL_AC (fPB);
	}
	
	// Now we try various ways to figure out which SCSIBus we're on
		
	SCSIBus *bus = NULL;
		
// None of these methods seem to actually work, and they sometimes
// cause crashes. So I'm just removing them for the moment.
// It seems that assigning bus numbers in order starting at 2 actually
// works, at least in the cases I've seen.


/*
		
	// Try GetDriverInformation		
	if (!bus) {
		RegEntryID regEntryID;
		RegistryEntryIDInit (&regEntryID);
		GetDriverInformation (ioVDRefNum, NULL, NULL, NULL, NULL, &regEntryID, NULL, NULL, NULL, NULL);	
		if (!RegistryEntryIDCompare (&regEntryID, NULL)) {
			#if qLogging
				gLogFile << "Got entry from GetDriverInformation" << endl_AC;
			#endif
			bus = SCSIBus::BusWithRegEntryID (&regEntryID);
		}
//		RegistryEntryIDDispose (&regEntryID);
	}	
	
	// Try DriverGestalt
	if (!bus) {
		DriverGestaltParam pbGestalt;
		Erase_AC (&pbGestalt);

		pbGestalt.ioVRefNum = ioVDrvInfo;
		pbGestalt.ioCRefNum	= ioVDRefNum;
		pbGestalt.csCode = kDriverGestaltCode;
		pbGestalt.driverGestaltSelector = kdgNameRegistryEntry;

		OSErr status = PBStatusSync((ParmBlkPtr) &pbGestalt);

		if (status == noErr) {
			DriverGestaltInfo response;
			response.i = pbGestalt.driverGestaltResponse;
			if (!RegistryEntryIDCompare (response.nmrg.entryID, NULL)) {		
				#if qLogging
					gLogFile << "Got entry from DriverGestalt" << endl_AC;
				#endif		
				bus = SCSIBus::BusWithRegEntryID (response.nmrg.entryID);
			}
//			RegistryEntryIDDispose (response.nmrg.entryID);
		} 
	}

*/

	// Try Boot. This doesn't actually tell me anything I know how to use yet, but
	// I may be able to figure out how to use it yet.
/*
	if (!bus) {
		DriverGestaltParam pbGestalt;
		Erase_AC (&pbGestalt);

		pbGestalt.ioVRefNum 	= ioVDrvInfo;
		pbGestalt.ioCRefNum	= ioVDRefNum;
		pbGestalt.csCode 		= kDriverGestaltCode;
		pbGestalt.driverGestaltSelector = kdgBoot;

		OSErr status = PBStatusSync((ParmBlkPtr) &pbGestalt);

		if (status == noErr) {
			DriverGestaltInfo response;
			response.i = pbGestalt.driverGestaltResponse;
			#if qLogging
				gLogFile << "SIMSlot: " << (int) response.boot.SIMSlot << endl_AC;
				gLogFile << "SIMsRSRC: " << (int) response.boot.SIMsRSRC << endl_AC;
			#endif					
		} 
	}
*/

	// OK, just use the bus number
	if (!bus) {
		#if qLogging
			gLogFile << "Getting BusWithNumber: " << (int) scsiDevice.bus << endl_AC;
		#endif
		bus = SCSIBus::BusWithNumber (scsiDevice.bus);
	}
	
	// Now, we should really have it.
	if (bus) {
		fValidOpenFirmwareName = true;
		if (bus->getIsActuallyATABus ()) {
			if (scsiDevice.targetID < 2) {
				fOpenFirmwareName.CopyFrom (bus->getATAOpenFirmwareName0 ());
				fShortOpenFirmwareName.CopyFrom (bus->getATAShortOpenFirmwareName0 ());
			} else {
				fOpenFirmwareName.CopyFrom (bus->getATAOpenFirmwareName1 ());
				fShortOpenFirmwareName.CopyFrom (bus->getATAShortOpenFirmwareName1 ());
			}
			scsiDevice.targetID %= 2;
		} else {
			fOpenFirmwareName.CopyFrom (bus->getOpenFirmwareName ());
			fShortOpenFirmwareName.CopyFrom (bus->getShortOpenFirmwareName ());
		}
		char buffer[16];
		snprintf (buffer, 16, "/@%X", scsiDevice.targetID); 
		fOpenFirmwareName += buffer;
		fShortOpenFirmwareName += buffer;
	} else {
		fValidOpenFirmwareName = false;
	}
	
	#if qLogging
		if (bus) {
			gLogFile << "OpenFirmwareName: ";
			gLogFile.WriteCharBytes ((char *) &fOpenFirmwareName[1], fOpenFirmwareName[0]);
			gLogFile << endl_AC;
			gLogFile << "ShortOpenFirmwareName: ";
			gLogFile.WriteCharBytes ((char *) &fShortOpenFirmwareName[1], fShortOpenFirmwareName[0]);
			gLogFile << endl_AC;
		} else {
			gLogFile << "Could not find Open Firmware name for SCSI bus: " << scsiDevice.bus << endl_AC;
		}
	#endif;
}

SCSIDevice::~SCSIDevice ()
{
	if (fPB) DisposePtr ((Ptr) fPB);
}

struct SCSI10ByteCommand {               /* Ten-byte command         */
    unsigned char       opcode;             /*  0                       */
    unsigned char       lun;                /*  1                       */
    unsigned char       lbn4;               /*  2                       */
    unsigned char       lbn3;               /*  3                       */
    unsigned char       lbn2;               /*  4                       */
    unsigned char       lbn1;               /*  5                       */
    unsigned char       pad;                /*  6                       */
    unsigned char       len2;               /*  7                       */
    unsigned char       len1;               /*  8                       */
    unsigned char       ctrl;               /*  9                       */
};

#define kSCSICmdReadCapacity        0x25
#define kScsiCmdRead10              0x28
#define kScsiCmdWrite10             0x2A

struct SCSICapacityData {
    unsigned char       lbn4;               /* Number                   */
    unsigned char       lbn3;               /*  of                      */
    unsigned char       lbn2;               /*   logical                */
    unsigned char       lbn1;               /*    blocks                */
    unsigned char       len4;               /* Length                   */
    unsigned char       len3;               /*  of each                 */
    unsigned char       len2;               /*   logical block          */
    unsigned char       len1;               /*    in bytes              */
};

void
SCSIDevice::readCapacity ()
{
	ThrowIfNULL_AC (fPB);
	BlockSet_AC (fPB, fPBLength, 0);

	SCSI10ByteCommand *readCapacityCommand = (SCSI10ByteCommand *) &fPB->scsiCDB.cdbBytes[0];
	readCapacityCommand->opcode = kSCSICmdReadCapacity;
    readCapacityCommand->lun = (fDeviceIdent.LUN & 0x03) << 5;
    
    fPB->scsiCDBLength = 10;    	
	fPB->scsiPBLength = fPBLength;
	fPB->scsiFunctionCode = SCSIExecIO;
	fPB->scsiDevice = fDeviceIdent;
	fPB->scsiTimeout = 2000L;
	fPB->scsiFlags = scsiDirectionIn | scsiSIMQNoFreeze | scsiDontDisconnect;

    SCSICapacityData capacityData;
    Erase_AC (&capacityData);

	fPB->scsiDataPtr = (unsigned char *) &capacityData;
	fPB->scsiDataLength = sizeof (capacityData);
	fPB->scsiDataType = scsiDataBuffer;
	fPB->scsiTransferType = scsiTransferPolled;

  	OSErr status = SCSIAction ((SCSI_PB *) fPB);
	if (status == noErr) status = fPB->scsiResult;

	if (status != noErr) {
		#if qLogging
			gLogFile << "Error reading capacity" << endl_AC;
		#endif
		ThrowException_AC (kErrorReadingCapacity, 0);
	}
	
	fBlockCount = capacityData.lbn4;
	fBlockCount = (fBlockCount << 8) | capacityData.lbn3;
	fBlockCount = (fBlockCount << 8) | capacityData.lbn2;
	fBlockCount = (fBlockCount << 8) | capacityData.lbn1;

	fBlockSize = capacityData.len4;
	fBlockSize = (fBlockSize << 8) | capacityData.len3;
	fBlockSize = (fBlockSize << 8) | capacityData.len2;
	fBlockSize = (fBlockSize << 8) | capacityData.len1;
}


OSErr
SCSIDevice::writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer)
{
	ThrowIfNULL_AC (fPB);
	if (fBlockSize != 512) ThrowException_AC (kWrite512ByteBlocksOnly, 0);
	
	BlockSet_AC (fPB, fPBLength, 0);

	SCSI10ByteCommand *writeCommand = (SCSI10ByteCommand *) &fPB->scsiCDB.cdbBytes[0];
	writeCommand->opcode = kScsiCmdWrite10;
    writeCommand->lun = (fDeviceIdent.LUN & 0x03) << 5;
     
    writeCommand->lbn4 = (start >> 24) & 0xFF;
    writeCommand->lbn3 = (start >> 16) & 0xFF;
    writeCommand->lbn2 = (start >> 8) & 0xFF;
    writeCommand->lbn1 = start & 0xFF;

    writeCommand->len2 = (count >> 8) & 0xFF;
    writeCommand->len1 = count & 0xFF;

    fPB->scsiCDBLength = 10;    	
	fPB->scsiPBLength = fPBLength;
	fPB->scsiFunctionCode = SCSIExecIO;
	fPB->scsiDevice = fDeviceIdent;
	fPB->scsiTimeout = 2000L;
	fPB->scsiFlags = scsiDirectionOut | scsiSIMQNoFreeze | scsiDontDisconnect;
 
	fPB->scsiDataPtr = (unsigned char *) buffer;
	fPB->scsiDataLength = count * fBlockSize;
	fPB->scsiDataType = scsiDataBuffer;
	fPB->scsiTransferType = scsiTransferPolled;

  	OSErr status = SCSIAction ((SCSI_PB *) fPB);
	if (status == noErr) status = fPB->scsiResult;
	
	if (status != noErr) {
		#if qLogging
			gLogFile << "Error " << status << " writing blocks" << endl_AC;
		#endif
		ThrowException_AC (kErrorWritingBlocks, 0);
	}
	
	return status;
}

OSErr
SCSIDevice::readBlocks (unsigned int start, unsigned int count, UInt8 **buffer)
{
	ThrowIfNULL_AC (fPB);
	ThrowIfNULL_AC (buffer);
	BlockSet_AC (fPB, fPBLength, 0);

	SCSI10ByteCommand *readCommand = (SCSI10ByteCommand *) &fPB->scsiCDB.cdbBytes[0];
	readCommand->opcode = kScsiCmdRead10;
    readCommand->lun = (fDeviceIdent.LUN & 0x03) << 5;
    
    // the start and count will be in terms of 512 byte blocks
    // we will allocate the buffer ourselves with NewPtr
    // the caller must dispose of the buffer with DisposePtr
    
    unsigned int byteOffset = 0;
 
    if (fBlockSize != 512) {
    	long long startBytes = start * 512;
    	start = startBytes / fBlockSize;
    	byteOffset = startBytes % fBlockSize;
    	count = (((count * 512) + byteOffset - 1) / fBlockSize) + 1;
    }
     
 	*buffer = (UInt8 *) NewPtr (count * fBlockSize);
 	ThrowIfNULL_AC (*buffer);
 
    readCommand->lbn4 = (start >> 24) & 0xFF;
    readCommand->lbn3 = (start >> 16) & 0xFF;
    readCommand->lbn2 = (start >> 8) & 0xFF;
    readCommand->lbn1 = start & 0xFF;

    readCommand->len2 = (count >> 8) & 0xFF;
    readCommand->len1 = count & 0xFF;

    fPB->scsiCDBLength = 10;    	
	fPB->scsiPBLength = fPBLength;
	fPB->scsiFunctionCode = SCSIExecIO;
	fPB->scsiDevice = fDeviceIdent;
	fPB->scsiTimeout = 2000L;
	fPB->scsiFlags = scsiDirectionIn | scsiSIMQNoFreeze | scsiDontDisconnect;
 
	fPB->scsiDataPtr = (unsigned char *) *buffer;
	fPB->scsiDataLength = count * fBlockSize;
	fPB->scsiDataType = scsiDataBuffer;
	fPB->scsiTransferType = scsiTransferPolled;

  	OSErr status = SCSIAction ((SCSI_PB *) fPB);
	if (status == noErr) status = fPB->scsiResult;
	
	if (status != noErr) {
		#if qLogging
			gLogFile << "Error " << status << " reading blocks." << endl_AC;
		#endif
		return status;
	}
	
	if (byteOffset != 0) {
		// need to realign the buffer
		char *mark = (char *) *buffer;
		mark += byteOffset;
		BlockMoveData (mark, *buffer, (count * fBlockSize) - byteOffset);
	}

	return status;
}


