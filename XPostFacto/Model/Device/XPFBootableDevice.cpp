/*

Copyright (c) 2001 - 2003
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
#include "MountedVolume.h"
#include "XPostFacto.h"

#ifndef __MACH__
	#include "ATADevice.h"
	#include "FirewireDevice.h"
	#include "SCSIDevice.h"
	
	#include <DriverGestalt.h>
	#include <HFSVolumes.h>
#endif

#include <iostream.h>
#include <stdio.h>

#include "XPFLog.h"
#include "XPFErrors.h"
#include "XPFAuthorization.h"
#include "vers_rsrc.h"
#include "OFAliases.h"

TemplateAutoList_AC <XPFBootableDevice> XPFBootableDevice::gDeviceList;

// .AppleCD contants 

enum {
	csSetPowerMode = 70,
	csQuiescence = 0x437
};

enum {
	pmActive  = 0,
	pmStandby  = 1,
	pmIdle   = 2,
	pmSleep   = 3
};

enum {
	quiescenceON = 0,
	quiescenceOFF = 1
}; 

bool XPFBootableDevice::fInitialized = false;

void
XPFBootableDevice::Initialize ()
{
// In Mac OS X, we don't both to initialize. We just construct as needed.
// Also, we don't use the subclasses--one class will do.
#ifndef __MACH__
	FirewireDevice::Initialize ();	// we need to do this each time
	if (fInitialized) return;

	fInitialized = true;

	DisableCDDriver ();

	try {
		SCSIDevice::Initialize ();
		ATADevice::Initialize ();
	}
	catch (...) {
		EnableCDDriver ();
		throw;
	}
	
	EnableCDDriver ();
#endif
}

void
XPFBootableDevice::DisableCDDriver ()
{
#ifndef __MACH__
	CntrlParam pb;
	SInt16 refNum;
	OSErr err;
	
	err = OpenDriver ("\p.AppleCD", &refNum); 	
	if (err != noErr) return;
	
	BlockZero_AC (pb);

 	pb.ioCRefNum = refNum;
 	pb.csCode = csSetPowerMode;
 	*(UInt8 *) &pb.csParam[0] = pmActive;
 	err = PBControlImmed ((ParmBlkPtr) &pb);
	if (err != noErr) return;	
	
	BlockZero_AC (pb);

 	pb.ioCRefNum = refNum;
 	pb.csCode = csQuiescence;
 	pb.csParam[0] = quiescenceON;
 
	err = PBControlImmed ((ParmBlkPtr) &pb); 
#endif
}

void
XPFBootableDevice::EnableCDDriver ()
{
#ifndef __MACH__
	CntrlParam pb;
	SInt16 refNum;
	OSErr err;
	
	err = OpenDriver ("\p.AppleCD", &refNum); 	
	if (err != noErr) return;
		
	BlockZero_AC (pb);

 	pb.ioCRefNum = refNum;
 	pb.csCode = csQuiescence;
 	pb.csParam[0] = quiescenceOFF;
 
	err = PBControlImmed ((ParmBlkPtr) &pb); 
#endif
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

XPFPartition* 
XPFBootableDevice::getFirstHFSPartition ()
{
	extractPartitionInfo ();
	XPFPartition *retVal = NULL;
	UInt32 partNum = 999;
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next () ) {
		if (iter->getIsHFSPlusVolume () && iter->getPartitionNumber () < partNum) {
			retVal = iter.Current ();
			partNum = iter->getPartitionNumber ();
		}
	}
	return retVal;
}

bool
XPFBootableDevice::isFirewireDevice ()
{
#ifdef __MACH__
	return fIsFireWireDevice;
#else
	return false;
#endif
}

bool
XPFBootableDevice::isReallyATADevice ()
{
#ifdef __MACH__
	return fIsATADevice;
#else
	return false;
#endif
}

void
XPFBootableDevice::extractPartitionInfo ()
{
	if (fPartitionList) return;
	fPartitionList = new PartitionList;
	
	DisableCDDriver ();

	try {
		readCapacity ();

		#ifdef __MACH__
		openDeviceFile ();
		#endif

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
				return;
			}
			int pmCount = pm->pmMapBlkCnt;
			if (pm) DisposePtr ((Ptr) pm);
			pm = NULL;
			err = readBlocks (1, pmCount, (UInt8 **) &pm);
			if (err != noErr) {
				#if qLogging
					gLogFile << "Error reading partition map" << endl_AC;
				#endif
				return;;
			}
			for (int x = 0; x < pmCount; x++) {
				try {
					XPFPartition *info = new XPFPartition (this, pm + x, x + 1);
					fPartitionList->InsertLast (info);
				}
				catch (...) {
				
				}
			}
		}	
		if (pm) DisposePtr ((Ptr) pm);

		#ifdef __MACH__
		closeDeviceFile ();
		#endif
	}
	catch (...) {
		EnableCDDriver ();
		throw;
	}
	
	EnableCDDriver ();
}

union VolumeHeader {
	HFSMasterDirectoryBlock hfs;
	HFSPlusVolumeHeader hfsplus;
};

void 
XPFBootableDevice::installBootXToPartition (XPFPartition *part)
{
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next ()) {
		// We install if the partition is the one specified, or if BootX is already installed
		if (iter->getClaimsBootXInstalled () || (iter.Current () == part)) {
			iter->installBootX ();
		}
	}
}

UInt32 
XPFBootableDevice::getActiveBootXVersion ()
{
	UInt32 retVal = 0xFFFFFFFF;
	for (PartitionIterator iter (fPartitionList); iter.Current (); iter.Next ()) {
		UInt32 vers = iter->getBootXVersion ();
		// The idea here is to return the oldest installed version of BootX on the device,
		// or 0 if there is no BootX installed at all. So we skip 0's that get returned, 
		// except that if nothing else gets returned then we will ultimately return 0 as well.
		// getBootXVersion will return kBootXImproperlyInstalled if BootX is not installed properly 
		if (vers == 0) continue;
		if (VERS_compare (vers, retVal) == -1) retVal = vers;	
	}
	if (retVal == 0xFFFFFFFF) retVal = 0;
	return retVal;
}

void 
XPFBootableDevice::setBus (XPFBus *bus)
{
	if (fBus != bus) {
		fBus = bus;
		checkOpenFirmwareName ();
		Changed (cSetBus, bus);
	}
}

XPFBootableDevice::XPFBootableDevice 
#ifdef __MACH__
(io_registry_entry_t entry)
#else
(SInt16 driverRefNum)
#endif
{
	fInvalid = false;
	fNeedsHelper = false;
	fBus = NULL;
	fDefaultBus = NULL;
	
#ifdef __MACH__
	fBSDName[0] = 0;
	fDeviceFile = NULL;
	fIsFireWireDevice = false;
	fIsATADevice = false;
#else
	fDriverRefNum = driverRefNum;
#endif

	fOpenFirmwareName[0] = 0;
	fShortOpenFirmwareName[0] = 0;

	fPartitionList = NULL;
	fBlockSize = 0;
	fBlockCount = 0;
	
#ifdef __MACH__
	// Figure out whether we need a helper or not
	// So far, we just check to see if we are a firewire device
	io_iterator_t iterator;
	IORegistryEntryCreateIterator (entry, kIOServicePlane, 
				kIORegistryIterateRecursively | kIORegistryIterateParents, &iterator);
 	io_registry_entry_t parent;
	
	while ((parent = IOIteratorNext (iterator)) != NULL) {
		if (IOObjectConformsTo (parent, "IOFireWireDevice")) {
			fNeedsHelper = true;
			fIsFireWireDevice = true;
			IOObjectRelease (parent);
			break;
		}
		if (IOObjectConformsTo (parent, "IOATABlockStorageDevice")) {
			fIsATADevice = true;
			IOObjectRelease (parent);
			break;
		}
		IOObjectRelease (parent);
	}
	IOObjectRelease (iterator);
	
	// Fill in some things we need to know
	
	CFNumberRef blockSize = (CFNumberRef) IORegistryEntryCreateCFProperty (entry, CFSTR ("Preferred Block Size"), NULL, 0);
	if (blockSize) {
		CFNumberGetValue (blockSize, kCFNumberLongType, &fBlockSize);
		CFRelease (blockSize);
	}

	SInt64 tempByteSize;
	CFNumberRef byteSize = (CFNumberRef) IORegistryEntryCreateCFProperty (entry, CFSTR ("Size"), NULL, 0);
	if (byteSize) {
		CFNumberGetValue (byteSize, kCFNumberSInt64Type, &tempByteSize);
		fBlockCount = tempByteSize / fBlockSize;	
		CFRelease (byteSize);	
	}
	
	CFStringRef bsdName = (CFStringRef) IORegistryEntryCreateCFProperty (entry, CFSTR ("BSD Name"), NULL, 0);
	if (bsdName) {
		strcpy (fBSDName, "/dev/r");
		CFStringGetCString (bsdName, fBSDName + 6, 26, kCFStringEncodingASCII); 
		CFRelease (bsdName);
	}
	
	gLogFile << "Created XPFBootableDevice for BSD name: " << fBSDName << endl_AC;

#endif
}

XPFBootableDevice::~XPFBootableDevice ()
{
	if (fPartitionList) delete fPartitionList;
#ifdef __MACH__
	closeDeviceFile ();
#endif
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

#ifdef __MACH__

XPFBootableDevice*
XPFBootableDevice::DeviceForRegEntry (io_registry_entry_t startpoint)
{
	XPFBootableDevice *retVal = NULL;
	if (!IORegistryEntryInPlane (startpoint, kIOServicePlane)) return retVal;

	// First we find the next IOMedia device above
	io_iterator_t iterator;
	IORegistryEntryCreateIterator (startpoint, kIOServicePlane, 
				kIORegistryIterateRecursively | kIORegistryIterateParents, &iterator);
 	io_registry_entry_t entry;
	bool found = false;
	
	for (entry = IOIteratorNext (iterator); entry != NULL; entry = IOIteratorNext (iterator)) {
		if (IOObjectConformsTo (entry, "IOMedia")) {
			found = true;
			break;
		}
		IOObjectRelease (entry);
	}
	IOObjectRelease (iterator);

	if (!found) return NULL;
	
	char bsdName[32];
	bsdName[0] = 0;
	CFStringRef bsdNameRef = (CFStringRef) IORegistryEntryCreateCFProperty (entry, CFSTR ("BSD Name"), NULL, 0);
	
	if (bsdNameRef) {
		strcpy (bsdName, "/dev/r");
		CFStringGetCString (bsdNameRef, bsdName + 6, 26, kCFStringEncodingASCII); 
		CFRelease (bsdNameRef);
	}

	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		if (!strcmp (iter->fBSDName, bsdName)) {
			IOObjectRelease (entry);
			return iter.Current ();
		}
	}
	
	retVal = new XPFBootableDevice (entry);
	gDeviceList.InsertLast (retVal);
	
	IOObjectRelease (entry);
	return retVal;
}

#else

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

#endif

XPFBootableDevice* 
XPFBootableDevice::DeviceWithOpenFirmwareName (char *ofName)
{
	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		if (OFAliases::MatchAliases (ofName, iter->getOpenFirmwareName (false))) {
			return iter.Current ();
		}
	}
	return NULL;
}

XPFBootableDevice* 
XPFBootableDevice::DeviceWithDeviceIdent (UInt32 deviceIdent)
{
	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		if (iter->getDeviceIdent () == deviceIdent) {
			return iter.Current ();
		}
	}
	return NULL;
}

XPFBootableDevice* 
XPFBootableDevice::DeviceWithInfoAndName (FSVolumeInfo *info, HFSUniStr255 *name)
{
	for (DeviceIterator iter (&gDeviceList); iter.Current (); iter.Next ()) {
		if (iter->partitionWithInfoAndName (info, name)) return iter.Current ();
	}
	return NULL;
}

#ifdef __MACH__

void
XPFBootableDevice::openDeviceFile ()
{
	if (fDeviceFile) return; // already open
	XPFSetUID uid (0);
	fDeviceFile = fopen (fBSDName, "rb");
}

void
XPFBootableDevice::closeDeviceFile ()
{
	if (!fDeviceFile) return; // already closed
	XPFSetUID myUID (0);
	fclose (fDeviceFile);
	fDeviceFile = NULL;
}

OSErr 
XPFBootableDevice::readBlocks (UInt32 start, UInt32 count, UInt8 **buffer)
{
	ThrowIfNULL_AC (buffer);

	if (fBlockSize == 0) {
		gLogFile << "XPFBootableDevice::readBlocks fBlockSize = 0" << endl_AC;
		return -1;
	}
	
	openDeviceFile ();
	if (fDeviceFile == NULL) {
		gLogFile << "XPFBootableDevice::readBlocks error opening device file" << endl_AC;
		return -1;
    }

//	gLogFile << "XPFBootableDevice::readBlocks start: " << start << " count: " << count << endl_AC;
    
    // the start and count will be in terms of 512 byte blocks
    // we will allocate the buffer ourselves with NewPtr
    // the caller must dispose of the buffer with DisposePtr
    
    UInt32 byteOffset = 0;
 
    if (fBlockSize != 512) {
    	long long startBytes = start * 512;
    	start = startBytes / fBlockSize;
    	byteOffset = startBytes % fBlockSize;
    	count = (((count * 512) + byteOffset - 1) / fBlockSize) + 1;
    }
     
 	*buffer = (UInt8 *) NewPtr (count * fBlockSize);
 	ThrowIfNULL_AC (*buffer);
 	
 	XPFSetUID uid (0);
 
	int err = fseeko (fDeviceFile, (off_t) start * (off_t) fBlockSize, SEEK_SET);
	if (err != noErr) {
		gLogFile << "XPFBootableDevice::readBlocks fseeko returned err: " << err << endl_AC;
		return err;
	}
	
	int numRead = fread (*buffer, fBlockSize, count, fDeviceFile);
	if (numRead != count) {
		gLogFile << "XPFBootableDevice::readBlocks wanted: " << count << " read: " << numRead << endl_AC;
	}
	
	if (byteOffset != 0) {
		// need to realign the buffer
		char *mark = (char *) *buffer;
		mark += byteOffset;
		BlockMoveData (mark, *buffer, (count * fBlockSize) - byteOffset);
	}

	return noErr;
}

OSErr 
XPFBootableDevice::writeBlocks (UInt32 start, UInt32 count, UInt8 *buffer)
{
	ThrowIfNULL_AC (buffer);
	if (fBlockSize != 512) ThrowException_AC (kWrite512ByteBlocksOnly, 0);

	XPFSetUID uid (0);
     
	openDeviceFile ();
	if (fDeviceFile == NULL) return -1;
	
	int err = fseeko (fDeviceFile, (off_t) start * (off_t) fBlockSize, SEEK_SET);
	if (err != noErr) {
		gLogFile << "XPFBootableDevice::writeBlocks fseeko returned err: " << err << endl_AC;
		return err;
	}

	int numWritten = fwrite (buffer, fBlockSize, count, fDeviceFile);
	if (numWritten != count) {
		gLogFile << "XPFBootableDevice::writeBlocks wanted: " << count << " written: " << numWritten << endl_AC;
	}
	
	return noErr;
}

void 
XPFBootableDevice::readCapacity ()
{
	// This is all done in the constructor now
}

#endif  // __MACH__

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



