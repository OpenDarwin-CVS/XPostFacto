/*

Copyright (c) 2002
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

// This file contains some code (from pdisk) which is subject to the following copyright

/*
 * Copyright 1997,1998 by Apple Computer, Inc.
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

#include "ATADevice.h"
#include "XPFPartition.h"
#include "MoreFilesExtras.h"
#include "ATABus.h"
#include "MountedVolume.h"

#include "XPFLog.h"
#include "XPFErrors.h"
#include "XPostFacto.h"

#include <stdio.h>
#include <ATA.h>

#define kATATimeout 2000L

#define SWAP_SHORTS(x)  ((((x) & 0xFFFF) << 16) | (((x) >> 16) & 0xFFFF))

#define LBA_CAPABLE 0x0200

enum {
    kDevUnknown     =   0,
    kDevATA         =   1,
    kDevATAPI       =   2,
    kDevPCMCIA      =   3
};

struct ATA_identify_drive_info {        /* word */
    unsigned short  config_bits;        /*  0 */
    unsigned short  num_cylinders;      /*  1 */
    unsigned short  reserved2;          /*  2 */
    unsigned short  num_heads;          /*  3 */
    unsigned short  bytes_per_track;    /*  4 */
    unsigned short  bytes_per_sector;   /*  5 */
    unsigned short  sectors_per_track;  /*  6 */
    unsigned short  vendor7[3];         /*  7-9 */
    char            serial_number[20];  /* 10-19 */
    unsigned short  buffer_type;        /* 20 */
    unsigned short  buffer_size;        /* 21 */
    unsigned short  num_of_ecc_bytes;   /* 22 */
    char            firmware_rev[8];    /* 23-26 */
    char            model_number[40];   /* 27-46 */
    unsigned short  word47;             /* 47 */
    unsigned short  double_word_io;     /* 48 */
    unsigned short  capabilities;       /* 49 */
    unsigned short  reserved50;         /* 50 */
    unsigned short  pio_timing;         /* 51 */
    unsigned short  dma_timing;         /* 52 */
    unsigned short  current_is_valid;   /* 53 */
    unsigned short  cur_cylinders;      /* 54 */
    unsigned short  cur_heads;          /* 55 */
    unsigned short  cur_sec_per_track;  /* 56 */
    unsigned long   total_sectors;      /* 57-58 */
    unsigned short  multiple_sectors;   /* 59 */
    unsigned long   lba_sectors;        /* 60-61 */
    unsigned short  singleword_dma;     /* 62 */
    unsigned short  multiword_dma;      /* 63 */
    unsigned short  reserved64[64];     /* 64-127 */
    unsigned short  vendor128[32];      /* 128-159 */
    unsigned short  reserved160[96];    /* 160-255 */
};

struct ATAPI_identify_drive_info {      /* word */
    unsigned short  config_bits;        /*  0 */
    unsigned short  retired1[9];        /*  1-9 */
    char            serial_number[20];  /* 10-19 */
    unsigned short  retired20[3];       /* 20-22 */
    char            firmware_rev[8];    /* 23-26 */
    char            model_number[40];   /* 27-46 */
    unsigned short  retired47[2];       /* 47-48 */
    unsigned short  capabilities;       /* 49 */
    unsigned short  reserved50;         /* 50 */
    unsigned short  pio_timing;         /* 51 */
    unsigned short  dma_timing;         /* 52 */
    unsigned short  current_is_valid;   /* 53 */
    unsigned short  retired54[8];       /* 54-61 */
    unsigned short  singleword_dma;     /* 62 */
    unsigned short  multiword_dma;      /* 63 */
    unsigned short  pio_transfer;       /* 64 */
    unsigned short  min_cycle_time;     /* 65 */
    unsigned short  rec_cycle_time;     /* 66 */
    unsigned short  min_wo_flow;        /* 67 */
    unsigned short  min_with_flow;      /* 68 */
    unsigned short  reserved69[2];      /* 69-70 */
    unsigned short  release_over;       /* 71 */
    unsigned short  release_service;    /* 72 */
    unsigned short  major_rev;          /* 73 */
    unsigned short  minor_rev;          /* 74 */
    unsigned short  reserved75[53];     /* 75-127 */
    unsigned short  vendor128[32];      /* 128-159 */
    unsigned short  reserved160[96];    /* 160-255 */
};

bool
ATADevice::ATAHardwarePresent ()
{
	return ATABus::getBusCount () > 0;
}

void
ATADevice::Initialize ()
{
	ATABus::Initialize ();
	
	if (!ATAHardwarePresent ()) return;
		
	// Now the idea is to find all the devices on the ATA bus
	
	ataDrvrRegister     pb;
    OSErr               status;

	// Get first device ID 
	Erase_AC (&pb);
    pb.ataPBFunctionCode    =   kATAMgrFindDriverRefnum;
    pb.ataPBVers            =   kATAPBVers1;
    pb.ataPBDeviceID        =   kATAStartIterateDeviceID;
    status                  =   ataManager ((ataPB*) &pb);

	// loop through devices
    for (pb.ataPBDeviceID = (UInt32) pb.ataDeviceNextID;
         pb.ataPBDeviceID != kATAEndIterateDeviceID;
         pb.ataPBDeviceID = (UInt32) pb.ataDeviceNextID)
    {           
        ThrowIfOSErr_AC (ataManager ((ataPB*) &pb));
        try {
	        gDeviceList.InsertLast (new ATADevice (pb.ataPBDeviceID, pb.ataDrvrRefNum));
	    }
	    catch (...) {
	    
	    }
    }
}

void
ATADevice::readCapacity ()
{
	if (fATADeviceType == kDevATAPI) return; 

	// Figure out some things we'll need to know about this device
	{
		ataIdentify pb;
		ATA_identify_drive_info info;
		
		Erase_AC (&pb);
		Erase_AC (&info);
		
		pb.ataPBFunctionCode    =   kATAMgrDriveIdentify;
	    pb.ataPBVers            =   kATAPBVers1;
	    pb.ataPBDeviceID        =   fDeviceIdent;
	    pb.ataPBFlags           =   mATAFlagIORead | mATAFlagByteSwap;
	    pb.ataPBTimeOut         =   kATATimeout;
	    pb.ataPBBuffer          =   (UInt8 *) &info;
	    
	    ThrowIfOSErr_AC (ataManager ((ataPB*) &pb));

		info.total_sectors = SWAP_SHORTS (info.total_sectors);
		info.lba_sectors = SWAP_SHORTS (info.lba_sectors);
		
		if (info.capabilities & LBA_CAPABLE) {
			fUseLBA = true;
			fBlockCount = info.lba_sectors;
			fHeads = 0;
			fSectors = 0;
		} else {
			fUseLBA = false;
			fBlockCount = info.total_sectors;
			fHeads = info.cur_heads;
			fSectors = info.cur_sec_per_track;
		}
	}
	
}

ATADevice::ATADevice (UInt32 ataDevice, SInt16 driverRefNum)
	: XPFBootableDevice (driverRefNum)
{
	fInvalid = false;
	
	fDeviceIdent = ataDevice;
	
	#if qLogging
		gLogFile << "Creating ATADevice" << endl_AC;
	#endif
		
			// Figure out some configuration stuff
	
	{
	    ataDevConfiguration pb;
	    Erase_AC (&pb);

		pb.ataPBFunctionCode    =   kATAMgrGetDrvConfiguration;
		pb.ataPBVers            =   kATAPBVers2;
		pb.ataPBDeviceID        =   fDeviceIdent;
		pb.ataPBTimeOut    		=   kATATimeout;
		
		ThrowIfOSErr_AC (ataManager ((ataPB*) &pb));
		
		fATADeviceType = pb.ataDeviceType;
		fATASocketType = pb.ataSocketType;
		
		if (fATADeviceType == kDevATAPI) {
			fBlockSize = 2048;
		} else {
			fBlockSize = 512;
		}
	}

	
		
	// Now we try various ways to figure out which ATABus we're on
		
	fBus = NULL;
		
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

	ataDeviceID *deviceID = (ataDeviceID *) &ataDevice;

	// OK, just use the bus number
	if (!fBus) {
		fBus = ATABus::BusWithNumber (deviceID->busNum);
	}
	
	// Now, we should really have it.
	if (fBus) {
		fDefaultBus = fBus;
	} else {
		fBus = ATABus::GetDefaultBus ();
	}
	
	checkOpenFirmwareName ();
	
	#if qLogging
		if (fBus) {
			gLogFile << "OpenFirmwareName: " << fOpenFirmwareName << endl_AC;
			gLogFile << "ShortOpenFirmwareName: " << fShortOpenFirmwareName << endl_AC;
		} else {
			gLogFile << "Could not find Open Firmware name for ATA bus: " << deviceID->busNum << endl_AC;
		}
	#endif;
}

void
ATADevice::checkOpenFirmwareName ()
{
	ataDeviceID *deviceID = (ataDeviceID *) &fDeviceIdent;
	
	sprintf (fOpenFirmwareName, "%s/@%d", fBus->getOpenFirmwareName (false), deviceID->devNum);
	sprintf (fShortOpenFirmwareName, "%s/@%d", fBus->getOpenFirmwareName (true), deviceID->devNum);
	
	Changed (cSetOpenFirmwareName, this);
}

OSErr
ATADevice::writeBlocks (unsigned int start, unsigned int count, UInt8 *buffer)
{
	OSErr status;
	ataIOPB pb;
	if (fBlockSize != 512) ThrowException_AC (kWrite512ByteBlocksOnly, 0);

	for (unsigned x = 0; x < count; x++) {
		Erase_AC (&pb);
		
		pb.ataPBFunctionCode    =   kATAMgrExecIO;
	    pb.ataPBVers            =   kATAPBVers1;
	    pb.ataPBDeviceID        =   fDeviceIdent;
	    pb.ataPBFlags           =   mATAFlagTFRead | mATAFlagIOWrite ;
	    pb.ataPBTimeOut         =   kATATimeout;
	    
	    pb.ataPBLogicalBlockSize =  fBlockSize;
	    pb.ataPBBuffer          =   buffer + (x * fBlockSize);
	    pb.ataPBByteCount 		= 	fBlockSize;

	    long lba, cyl, head, sector, slave;
	    
	    if (fUseLBA) {
	    	lba = 0x40;
	    	sector = (start + x) & 0xFF;
	    	head = ((start + x) >> 24) & 0xF;
	    	cyl = ((start + x) >> 8) & 0xFFFF;
	    } else {
	    	lba = 0x00;
			sector = ((start + x) % fSectors) + 1;
			cyl = (start + x) / fSectors;
			head = cyl % fHeads;
			cyl = cyl / fHeads;
	    }
	    
	    pb.ataPBTaskFile.ataTFCount = 1;
	    pb.ataPBTaskFile.ataTFSector = sector;
	    pb.ataPBTaskFile.ataTFCylinder = cyl;
	    if (fDeviceIdent & 0x0FF00) {
			slave = 0x10;
	    } else {
			slave = 0x0;
	    }
				  			    /* std | L/C  | Drive | head */
	    pb.ataPBTaskFile.ataTFSDH = 0xA0 | lba | slave | head;
	    pb.ataPBTaskFile.ataTFCommand = kATAcmdWrite;

	    status = ataManager ((ataPB*) &pb);
		
		if (status != noErr) {
			#if qLogging
				gLogFile << "Error " << status << " writing blocks" << endl_AC;
			#endif
			ThrowException_AC (kErrorWritingBlocks, 0);
		}
	}
	return status;
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
#define kScsiCmdTestUnitReady       0x00

#define kATAcmdATAPIPacket          0x00A0       

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

bool
ATADevice::atapiTestUnitReady ()
{
    ataIOPB         pb;
    OSErr           status;
    long            slave;
    ATAPICmdPacket  cmdPacket;
    SCSI10ByteCommand *testUnit;

    Erase_AC (&pb);
    Erase_AC (&cmdPacket);
    
    pb.ataPBFunctionCode    =   kATAMgrExecIO;
    pb.ataPBVers            =   kATAPBVers1;
    pb.ataPBDeviceID        =   fDeviceIdent;
    pb.ataPBFlags           =   mATAFlagTFRead | mATAFlagIORead | mATAFlagProtocol1;
    pb.ataPBTimeOut         =   kATATimeout;
    
    if (fDeviceIdent & 0x0FF00) {
		slave = 0x10;
    } else {
		slave = 0x0;
    }
			      /* std | L/C  | Drive | head */
    pb.ataPBTaskFile.ataTFSDH = 0xA0 | 0x40 | slave;
    pb.ataPBTaskFile.ataTFCommand = kATAcmdATAPIPacket;
    pb.ataPBPacketPtr = &cmdPacket;
    
    cmdPacket.atapiPacketSize = 16;
    testUnit = (SCSI10ByteCommand *) &cmdPacket.atapiCommandByte[0];

    testUnit->opcode = kScsiCmdTestUnitReady;
    
    status = ataManager ((ataPB*) &pb);
    return (status == noErr);
}

OSErr 
ATADevice::readATAPIBlocks (unsigned int start, unsigned int count, UInt8 *buffer)
{
    ataIOPB         pb;
    OSErr           status;
    long            slave;
    ATAPICmdPacket  cmdPacket;
    SCSI10ByteCommand *readCmd;

	for (unsigned x = 0; x < count; x++) {

	    Erase_AC (&pb);
	    Erase_AC (&cmdPacket);
	    
	    pb.ataPBFunctionCode    =   kATAMgrExecIO;
	    pb.ataPBVers            =   kATAPBVers1;
	    pb.ataPBDeviceID        =   fDeviceIdent;
	    pb.ataPBFlags           =   mATAFlagTFRead | mATAFlagIORead | mATAFlagProtocol1;
	    pb.ataPBTimeOut         =   kATATimeout;
	    pb.ataPBBuffer          =   buffer + (x * fBlockSize);
	    pb.ataPBByteCount 		= 	fBlockSize;
	    pb.ataPBTaskFile.ataTFCylinder = fBlockSize;
	    if (fDeviceIdent & 0x0FF00) {
			slave = 0x10;
	    } else {
			slave = 0x0;
	    }
				    	  /* std | L/C  | Drive | head */
	    pb.ataPBTaskFile.ataTFSDH = 0xA0 | 0x40 | slave;
	    pb.ataPBTaskFile.ataTFCommand = kATAcmdATAPIPacket;
	    pb.ataPBPacketPtr = &cmdPacket;
	    
	    cmdPacket.atapiPacketSize = 16;
	    readCmd = (SCSI10ByteCommand *) &cmdPacket.atapiCommandByte[0];

	    readCmd->opcode = kScsiCmdRead10;

	    readCmd->lbn4 = ((start + x) >> 24) & 0xFF;
	    readCmd->lbn3 = ((start + x) >> 16) & 0xFF;
	    readCmd->lbn2 = ((start + x) >> 8) & 0xFF;
	    readCmd->lbn1 = (start + x) & 0xFF;

	    long blocks = 1;
	    readCmd->len2 = (blocks >> 8) & 0xFF;
	    readCmd->len1 = blocks & 0xFF;

	    status = ataManager ((ataPB*) &pb);
		if (status != noErr) {
			#if qLogging
				gLogFile << "Error " << status << " reading blocks" << endl_AC;
			#endif
			ThrowException_AC (kErrorReadingBlocks, 0);
		}
	}
	return status;
}
				
OSErr 
ATADevice::readATABlocks (unsigned int start, unsigned int count, UInt8 *buffer)
{
	OSErr status;
	ataIOPB pb;
	for (unsigned x = 0; x < count; x++) {
		Erase_AC (&pb);
		
		pb.ataPBFunctionCode    =   kATAMgrExecIO;
	    pb.ataPBVers            =   kATAPBVers1;
	    pb.ataPBDeviceID        =   fDeviceIdent;
	    pb.ataPBFlags           =   mATAFlagTFRead | mATAFlagIORead;
	    pb.ataPBTimeOut         =   kATATimeout;
	    
	    pb.ataPBLogicalBlockSize =  fBlockSize;
	    pb.ataPBBuffer          =   buffer + (x * fBlockSize);
	    pb.ataPBByteCount 		= 	fBlockSize;

	    long lba, cyl, head, sector, slave;
	    
	    if (fUseLBA) {
	    	lba = 0x40;
	    	sector = (start + x) & 0xFF;
	    	head = ((start + x) >> 24) & 0xF;
	    	cyl = ((start + x) >> 8) & 0xFFFF;
	    } else {
	    	lba = 0x00;
			sector = ((start + x) % fSectors) + 1;
			cyl = (start + x) / fSectors;
			head = cyl % fHeads;
			cyl = cyl / fHeads;
	    }
	    
	    pb.ataPBTaskFile.ataTFCount = 1;
	    pb.ataPBTaskFile.ataTFSector = sector;
	    pb.ataPBTaskFile.ataTFCylinder = cyl;
	    if (fDeviceIdent & 0x0FF00) {
			slave = 0x10;
	    } else {
			slave = 0x0;
	    }
				  			    /* std | L/C  | Drive | head */
	    pb.ataPBTaskFile.ataTFSDH = 0xA0 | lba | slave | head;
	    pb.ataPBTaskFile.ataTFCommand = kATAcmdRead;

	    status = ataManager ((ataPB*) &pb);
		
		if (status != noErr) {
			#if qLogging
				gLogFile << "Error " << status << " reading blocks" << endl_AC;
			#endif
			ThrowException_AC (kErrorReadingBlocks, 0);
		}
	}
	return status;
}

OSErr
ATADevice::readBlocks (unsigned int start, unsigned int count, UInt8 **buffer)
{
	ThrowIfNULL_AC (buffer);

    unsigned int byteOffset = 0;

    // the start and count will be in terms of 512 byte blocks
    // we will allocate the buffer ourselves with NewPtr
    // the caller must dispose of the buffer with DisposePtr
 
    if (fBlockSize != 512) {
    	long long startBytes = start * 512;
    	start = startBytes / fBlockSize;
    	byteOffset = startBytes % fBlockSize;
    	count = (((count * 512) + byteOffset - 1) / fBlockSize) + 1;
    }
     
 	*buffer = (UInt8 *) NewPtr (count * fBlockSize);
 	ThrowIfNULL_AC (*buffer);

	OSErr status;
	if (fATADeviceType == kDevATAPI) {
		status = readATAPIBlocks (start, count, (UInt8 *) *buffer);
	} else {
		status = readATABlocks (start, count, (UInt8 *) *buffer);
	}

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
