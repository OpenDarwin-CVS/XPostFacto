/*
 * Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

extern "C" {
	#include <machine/machine_routines.h>
}

// The include doesn't work, so here's the one I want.

extern "C" {
	vm_offset_t ml_io_map (vm_offset_t phys_addr, vm_size_t size);
}

// Some defines for SPR access
#define __mtspr(spr, val)  __asm__ volatile("mtspr  " # spr ", %0" : : "r" (val))
#define __mfspr(reg, spr)  __asm__ volatile("mfspr  %0, " # spr : "=r" (reg))
#define mtspr(spr, val) __mtspr(spr, val)
#define mfspr(reg, spr) __mfspr(reg, spr)

#define L2CR		1017

#define PVR			287
#define PVR750		0x00080000		// a G3 processor
#define PVR7400		0x000C0000		// a G4 processor

#include "PowerSurgeCPU.h"
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOPlatformExpert.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef super
#define super IOCPU

OSDefineMetaClassAndStructors(PowerSurgeCPU, IOCPU);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Static variable initialization

IOCPUInterruptController* PowerSurgeCPU::gCPUIC = NULL;
#if 0   // need to fix for Panther
unsigned char* PowerSurgeCPU::hammerHeadBase = NULL;
#endif
UInt32 PowerSurgeCPU::actualCPUs = 0;
UInt32 PowerSurgeCPU::wantedCPUs = 0;

#if 0
// This needs to be fixed for Panther. And it isn't really accomplishing anything
// anyway, since MP support isn't working.
unsigned char 
PowerSurgeCPU::readHammerHeadReg (unsigned long offset)
{
	unsigned char data = hammerHeadBase[offset];
	eieio();
	return data;
}

void 
PowerSurgeCPU::writeHammerHeadReg (unsigned long offset, unsigned char data)
{
	hammerHeadBase[offset] = data;
	eieio();
}
#endif

bool 
PowerSurgeCPU::start(IOService *provider)
{
	kern_return_t       result;
	ml_processor_info_t processor_info;

	if (!getPlatform()->metaCast ("ApplePowerSurgePE")) return false;
	
	if (!super::start (provider)) return false;
  
  	// Get the physical CPU number from the "reg" property.
	OSData *tmpData = OSDynamicCast(OSData, provider->getProperty("reg"));
	if (tmpData == 0) return false;
	UInt32 physCPU = * (long *) tmpData->getBytesNoCopy();
	setCPUNumber (physCPU);
	  
	// Find out if this is the boot CPU.
	bootCPU = (physCPU == 0);
	
	if (bootCPU) {
#if 0   // This needs to be fixed for Panther
		hammerHeadBase = (unsigned char *) ml_io_map (0xF8000000, 0x1000);
		actualCPUs = (readHammerHeadReg (0x0090) & 0x02) ? 2 : 1;
#else   
		actualCPUs = 1;
#endif

		// Limit the number of CPUs by the cpu=# boot arg.
		wantedCPUs = actualCPUs;
		UInt32 maxCPUs;
		if (PE_parse_boot_arg ("cpus", &maxCPUs)) {
			if (wantedCPUs > maxCPUs) wantedCPUs = maxCPUs;
		}
		
		// Limit to 1, because MP support isn't finished yet
		wantedCPUs = 1;
				
		gCPUIC = new IOCPUInterruptController;
		if (gCPUIC == NULL) return false;
		if (gCPUIC->initCPUInterruptController (wantedCPUs) != kIOReturnSuccess) return false;
		gCPUIC->attach(this);
		gCPUIC->registerCPUInterruptController();
	}
		
	setProperty ("actualCPUs", actualCPUs, 32);
	setProperty ("wantedCPUs", wantedCPUs, 32);
		
	setCPUState (kIOCPUStateUninitalized);

	if (physCPU < wantedCPUs) {
		processor_info.cpu_id           = (cpu_id_t) this;
		processor_info.boot_cpu         = bootCPU;
		processor_info.start_paddr      = 0x0100;
		processor_info.l2cr_value       = setupL2Cache ();
		processor_info.supports_nap     = false;
		processor_info.time_base_enable = (time_base_enable_t) &PowerSurgeCPU::enableCPUTimeBase;

		// Register this CPU with mach.
		result = ml_processor_register (&processor_info, &machProcessor, &ipi_handler);
		if (result == KERN_FAILURE) return false;
		processor_start (machProcessor);
	}
  
	registerService ();
  
  	if (bootCPU && (actualCPUs == 2)) {
		IOPlatformDevice *secondCPU = new IOPlatformDevice;
		
		OSDictionary *newDict = provider->dictionaryWithProperties ();
		secondCPU->init (newDict);
		newDict->release ();
		
		secondCPU->removeProperty ("AAPL,phandle");
		
		int reg = 1;
		OSData *data = OSData::withBytes (&reg, sizeof (reg));
		secondCPU->setProperty ("reg", data);
		data->release ();
		
		const OSSymbol *name = provider->copyName ();
		secondCPU->setName (name);
		name->release ();
		
		secondCPU->setLocation ("1");
		
		secondCPU->attach (getPlatform ());
		secondCPU->attachToParent (provider->getParentEntry (gIODTPlane), gIODTPlane);
		secondCPU->registerService ();
		secondCPU->release ();
	}
  
	return true;
}

void 
PowerSurgeCPU::initCPU (bool boot)
{
	if (boot) {
		gCPUIC->enableCPUInterrupt(this);
	}
  
	setCPUState (kIOCPUStateRunning);	
}

void 
PowerSurgeCPU::quiesceCPU(void)
{
//	IOLog ("PowerSurgeCPU::quiesceCPU unsupported\n");
}

kern_return_t 
PowerSurgeCPU::startCPU (vm_offset_t /*start_paddr*/, vm_offset_t /*arg_paddr*/)
{
//	IOLog ("PowerSurgeCPU::startCPU unsupported\n");
	return KERN_FAILURE;
}

void 
PowerSurgeCPU::haltCPU (void)
{
//	IOLog ("PowerSurgeCPU::haltCPU unsupported\n");
}

const OSSymbol*
PowerSurgeCPU::getCPUName(void)
{
	char tmpStr[32];
	sprintf(tmpStr, "Primary%ld", getCPUNumber());
	return OSSymbol::withCString(tmpStr);
}
  
void 
PowerSurgeCPU::ipiHandler (void *refCon, void *nub, int source)
{
	// Call mach IPI handler for this CPU.
	if (ipi_handler) ipi_handler();
}

void
PowerSurgeCPU::signalCPU (IOCPU *target)
{
//	IOLog ("PowerSurgeCPU::signalCPU unsupported\n");
}

void
PowerSurgeCPU::enableCPUTimeBase (bool enable)
{
//	IOLog ("PowerSurgeCPU::enableCPUTimeBase unsupported\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

unsigned
PowerSurgeCPU::setupL2Cache ()
{
	unsigned pvr;
	mfspr (pvr, PVR);
	pvr &= 0xFFFF0000;
	if ((pvr != PVR750) && (pvr != PVR7400)) return 0;
		
	unsigned oldValue;
	mfspr (oldValue, L2CR);
	return oldValue; // already enabled
}
