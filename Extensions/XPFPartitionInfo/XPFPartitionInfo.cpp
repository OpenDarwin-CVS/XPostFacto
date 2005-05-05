/*

 Copyright (c) 2003, 2005
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 
*/

#include "XPFPartitionInfo.h"
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOApplePartitionScheme.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOUserClient.h>

#undef super
#define super IOService

OSDefineMetaClassAndStructors(XPFPartitionInfo, IOService);

IOService* 
XPFPartitionInfo::probe (IOService *provider, SInt32 *score)
{
	OSNumber *partitionID = OSDynamicCast (OSNumber, provider->getProperty ("Partition ID"));
	if (!partitionID) return NULL;
	fPartitionID = partitionID->unsigned32BitValue ();

	fPartitionScheme = OSDynamicCast (IOApplePartitionScheme, provider->getProvider ());
	if (!fPartitionScheme) return NULL;
		
	return this;
}

bool 
XPFPartitionInfo::start (IOService *provider)
{
	if (!super::start (provider)) return false;

	IOMemoryDescriptor *buffer = IOBufferMemoryDescriptor::withCapacity (512, kIODirectionIn);
	buffer->prepare (kIODirectionIn);
	
	IOReturn err = fPartitionScheme->read (this, 512 * fPartitionID, buffer);
	if (err != kIOReturnSuccess) {
		IOLog ("XPFPartitionInfo::start error %d\n", err);
		buffer->complete (kIODirectionIn);
		buffer->release ();
		return false;
	}
	
	buffer->readBytes (0, &fPartitionMapEntry, 512);
	buffer->complete (kIODirectionIn);
	buffer->release ();
		
	if (fPartitionMapEntry.dpme_signature != DPME_SIGNATURE) return false;
		
	updatePropertyList ();
	
	return true;
}

IOReturn
XPFPartitionInfo::writePropertiesToDisk ()
{
	IOMemoryDescriptor *buffer = IOBufferMemoryDescriptor::withBytes (&fPartitionMapEntry, 512, kIODirectionOut);
	buffer->prepare (kIODirectionOut);
	
	IOReturn err = fPartitionScheme->write (this, 512 * fPartitionID, buffer);
	if (err != kIOReturnSuccess) IOLog ("XPFPartitionInfo::writePropertiesToDisk error %d\n", err);
	
	buffer->complete (kIODirectionOut);
	buffer->release ();
	
	return err;
}

void
XPFPartitionInfo::updatePropertyList ()
{
	setProperty ("Physical Block Start", fPartitionMapEntry.dpme_pblock_start, 32);
	setProperty ("Physical Block Count", fPartitionMapEntry.dpme_pblocks, 32);
	setProperty ("Partition Name", fPartitionMapEntry.dpme_name);
	setProperty ("Partition Type", fPartitionMapEntry.dpme_type);
	setProperty ("Logical Block Start", fPartitionMapEntry.dpme_lblock_start, 32);
	setProperty ("Logical Block Count", fPartitionMapEntry.dpme_lblocks, 32);
	setProperty ("Boot Block", fPartitionMapEntry.dpme_boot_block, 32);
	setProperty ("Boot Bytes", fPartitionMapEntry.dpme_boot_bytes, 32);
	setProperty ("Boot Address", (unsigned) fPartitionMapEntry.dpme_load_addr, 32);
	setProperty ("Boot Entry", (unsigned) fPartitionMapEntry.dpme_goto_addr, 32);
	setProperty ("Boot Checksum", fPartitionMapEntry.dpme_checksum, 32);
	setProperty ("Processor Type", (char *) fPartitionMapEntry.dpme_process_id);
	setProperty ("Partition Status", fPartitionMapEntry.dpme_flags, 32);
}

IOReturn 
XPFPartitionInfo::setProperties (OSObject *properties)
{
    IOReturn err = IOUserClient::clientHasPrivilege (current_task(), kIOClientPrivilegeAdministrator);
    if (err != kIOReturnSuccess) return err;

	OSDictionary *dict = OSDynamicCast (OSDictionary, properties);
	if (!dict) return kIOReturnBadArgument;
		
	OSString *str = OSDynamicCast (OSString, dict->getObject ("Processor Type"));
	if (str) {
		const char *proctype = str->getCStringNoCopy ();
		if (strlen (proctype) > 15) return kIOReturnBadArgument;
		strcpy ((char *) fPartitionMapEntry.dpme_process_id, proctype);
	}

	OSNumber *num = OSDynamicCast (OSNumber, dict->getObject ("Boot Block"));
	if (num) fPartitionMapEntry.dpme_boot_block = num->unsigned32BitValue ();

 	num = OSDynamicCast (OSNumber, dict->getObject ("Boot Bytes"));
	if (num) fPartitionMapEntry.dpme_boot_bytes = num->unsigned32BitValue ();

 	num = OSDynamicCast (OSNumber, dict->getObject ("Boot Address"));
	if (num) fPartitionMapEntry.dpme_load_addr = (UInt8 *) num->unsigned32BitValue ();

 	num = OSDynamicCast (OSNumber, dict->getObject ("Boot Entry"));
	if (num) fPartitionMapEntry.dpme_goto_addr = (UInt8 *) num->unsigned32BitValue ();

 	num = OSDynamicCast (OSNumber, dict->getObject ("Boot Checksum"));
	if (num) fPartitionMapEntry.dpme_checksum = num->unsigned32BitValue ();
	
 	num = OSDynamicCast (OSNumber, dict->getObject ("Partition Status"));
	if (num) fPartitionMapEntry.dpme_flags = num->unsigned32BitValue ();
	
	updatePropertyList ();
	
	err = writePropertiesToDisk ();
	
	return err;
}
