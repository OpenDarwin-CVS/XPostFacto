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


#include "XPFBootableDevice.h"
#include "XPFPartition.h"
#include "MoreFilesExtras.h"
#include "SCSIBus.h"
#include "MountedVolume.h"

#include "ATADevice.h"
#include "FirewireDevice.h"

#include <Devices.h>
#include <Files.h>
#include <NameRegistry.h>
#include <iostream.h>
#include <HFSVolumes.h>
#include <DriverGestalt.h>
#include <stdio.h>

#include "XPFLog.h"
#include "XPFErrors.h"
#include "SCSI.h"

TemplateAutoList_AC <XPFBootableDevice> XPFBootableDevice::gDeviceList;

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

bool XPFBootableDevice::fInitialized = false;

void
XPFBootableDevice::Initialize ()
{
	FirewireDevice::Initialize ();	// we need to do this each time
	if (fInitialized) return;
	fInitialized = true;
	SCSIDevice::Initialize ();
	ATADevice::Initialize ();
}

XPFPartition*
XPFBootableDevice::partitionWithCreationDate (unsigned int date)
{
	extractPartitionInfo ();
	XPFPartition *retVal = NULL;
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next () ) {
		if (iter->getCreationDate () == date) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}

XPFPartition* 
XPFBootableDevice::partitionWithInfo (FSVolumeInfo *info)
{
	extractPartitionInfo ();
	XPFPartition *retVal = NULL;
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next () ) {
		if (iter->matchInfo (info)) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}
		
		
XPFPartition* 
XPFBootableDevice::partitionWithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name)
{
	extractPartitionInfo ();
	XPFPartition *retVal = NULL;
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next () ) {
		if (iter->matchInfoAndName (info, name)) {
			retVal = iter.Current ();
			break;
		}
	}
	return retVal;
}

void
XPFBootableDevice::extractPartitionInfo ()
{
	if (fPartitionList) return;
	fPartitionList = new PartitionList;

	readCapacity ();
	
	Partition *pm = NULL;
	OSErr err = readBlocks (1, 1, (UInt8 **) &pm);
	if (err != noErr) {
		#if qLogging
			gLogFile << "Error reading partition map block 1" << endl_AC;
		#endif
		ThrowException_AC (kInvalidPartitionMap, 0);
	} else {
		if (pm->pmSig != pMapSIG) {
			#if qLogging
				gLogFile << "Invalid partition map" << endl_AC;
			#endif
			ThrowException_AC (kInvalidPartitionMap, 0);
		}
		int pmCount = pm->pmMapBlkCnt;
		if (pm) DisposePtr ((Ptr) pm);
		pm = NULL;
		err = readBlocks (1, pmCount, (UInt8 **) &pm);
		if (err != noErr) {
			#if qLogging
				gLogFile << "Error reading partition map" << endl_AC;
			#endif
			ThrowException_AC (kErrorReadingPartitionMap, 0);
		}
		for (int x = 0; x < pmCount; x++) {
			#if qLogging
				gLogFile << "Partition: " << x + 1 << " Type: " << (char *) (pm + x)->pmParType
					<< " Processor: " << (char *) (pm + x)->pmProcessor << endl_AC;
			#endif
		
			if (!strcmp ((char *) (pm + x)->pmParType, "Apple_HFS")) {
				try {
					XPFPartition *info = new XPFPartition (this, pm + x, x + 1);
					fPartitionList->InsertLast (info);
				}
				catch (...) {
				
				}
			}
		}
	}	
	if (pm) DisposePtr ((Ptr) pm);
}

union VolumeHeader {
	HFSMasterDirectoryBlock hfs;
	HFSPlusVolumeHeader hfsplus;
};

void 
XPFBootableDevice::updateBootXIfInstalled (bool forceInstall)
{
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next ()) {
		iter->updateBootXIfInstalled (forceInstall);
	}
}

XPFBootableDevice::XPFBootableDevice (SInt16 driverRefNum)
{
	fValidOpenFirmwareName = false;
	fInvalid = false;
	fDriverRefNum = driverRefNum;
	fPartitionList = NULL;
}

XPFBootableDevice::~XPFBootableDevice ()
{
	if (fPartitionList) delete fPartitionList;
}

bool
XPFBootableDevice::isFirewireDevice ()
{
	return false;
}

void 
XPFBootableDevice::DeleteInvalidDevices ()
{
	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		XPFBootableDevice *current = iter.Current ();
		if (current->fInvalid) {
			delete current->fPartitionList;
			current->fPartitionList = NULL;
			current->fInvalid = false;
		}
	}
}

XPFBootableDevice* 
XPFBootableDevice::DeviceWithInfo (FSVolumeInfo *info)
{
	return DeviceWithDriverRefNum (info->driverRefNum);
}

XPFBootableDevice*
XPFBootableDevice::DeviceWithDriverRefNum (SInt16 driverRefNum)
{
	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		if (iter->fDriverRefNum == driverRefNum) {
			return iter.Current ();
		}
	}
	return NULL;
}

#if qDebug

void 
XPFBootableDevice::PrintPartitionMapEntry(Partition *part)
{
    printf("PrintPartitionMapEntry\r");
    printf("\r");
    printf("pmSig = '%2.2s'\r", &part->pmSig);
    printf("pmSigPad = %d\r", part->pmSigPad);
    printf("pmMapBlkCnt = %ld\r", part->pmMapBlkCnt);
    printf("pmPyPartStart = %ld\r", part->pmPyPartStart);
    printf("pmPartBlkCnt = %ld\r", part->pmPartBlkCnt);
    printf("pmPartName = Ò%sÓ\r", part->pmPartName);
    printf("pmParType = Ò%sÓ\r", part->pmParType);
    printf("pmLgDataStart = %ld\r", part->pmLgDataStart);
    printf("pmDataCnt = %ld\r", part->pmDataCnt);
    printf("pmPartStatus = %ld\r", part->pmPartStatus);
    printf("pmLgBootStart = %ld\r", part->pmLgBootStart);
    printf("pmBootSize = %ld\r", part->pmBootSize);
    printf("pmBootAddr = %ld\r", part->pmBootAddr);
    printf("pmBootAddr2 = %ld\r", part->pmBootAddr2);
    printf("pmBootEntry = %ld\r", part->pmBootEntry);
    printf("pmBootEntry2 = %ld\r", part->pmBootEntry2);
    printf("pmBootCksum = %08x\r", part->pmBootCksum);
    printf("pmProcessor = Ò%sÓ\r", part->pmProcessor);
    printf("\r");
}

void
XPFBootableDevice::Dump ()
{
	for (DeviceIterator iter (&gDeviceList); 
			iter.Current(); iter.Next() ) {
		iter->dump ();		
	}
}

void
XPFBootableDevice::dump ()
{
/*
		printf ("[%d.%d.%d] blockCount: %u, blockSize: %u\r", 
		(int) deviceIdent.bus, (int) deviceIdent.targetID, (int) deviceIdent.LUN
		, blockCount, blockSize);
		for (PartitionIterator iter (&partitionList); iter.Current(); iter.Next() ) {
			iter->dump ();
		}
*/
}

#endif



