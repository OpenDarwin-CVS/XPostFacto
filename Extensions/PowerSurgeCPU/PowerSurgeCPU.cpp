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

#define L2EM		0x80000000
#define L2IM		0x00200000
#define L2RESVM		0x000000FE
#define L2IPM		0x00000001
#define L2SIZM		0x30000000		// size

#define L2CR		1017

#define PVR			287
#define PVR750		0x00080000		// a G3 processor
#define PVR7400		0x000C0000		// a G4 processor

#include "PerProcInfo.h"

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
unsigned char* PowerSurgeCPU::hammerHeadBase = NULL;
UInt32 PowerSurgeCPU::actualCPUs = 0;
UInt32 PowerSurgeCPU::wantedCPUs = 0;

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
		hammerHeadBase = (unsigned char *) ml_io_map (0xF8000000, 0x1000);
		actualCPUs = (readHammerHeadReg (0x0090) & 0x02) ? 2 : 1;

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
	sprintf(tmpStr, "Primary%d", getCPUNumber());
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
	if (oldValue & L2EM) return oldValue; // already enabled

	unsigned int newValue;
	int foundArg;
	foundArg = PE_parse_boot_arg ("L2CR", &newValue);
	if (foundArg) {
		if (!(newValue & L2EM)) return oldValue;
		
		newValue &= ~L2RESVM;
		IOLog ("Setting L2CR to: 0x%X\n", newValue);
						
		unsigned temp = newValue & ~L2EM;
		mtspr (L2CR, temp);
		__asm__ ("sync");
		
		temp |= L2IM;	// we disable, and we invalidate
		mtspr (L2CR, temp);
		__asm__ ("sync");
		
		volatile unsigned checkL2CR;
		unsigned x = 0;
		do {		// we check to see when invalidation is finished
			x++;
			mfspr (checkL2CR, L2CR);
			__asm__ ("sync");
		} while (checkL2CR & L2IPM);
		
		__asm__ ("isync");
		if (x == 1) {
			IOLog ("Error setting L2CR: L2CR never invalidated\n");
			setProperty ("Error", "L2CR invalidation error");
			return 0;
		}
		
		mtspr (L2CR, newValue); // now set the new value
		__asm__ ("sync");
		
		setProperty ("L2CR", newValue, 32);
		
		// we tell per_proc_info that we have an L2 cache
		per_proc_info[0].pf.Available |= pfL2;
		
		// We set the L2CR shadowed by per_proc_info
		per_proc_info[0].pf.l2cr = newValue;
		
		// We need to tell per_proc_into how big the cache is, which we can figure
		// out by looking at the L2CR. 					
		// We mask out everything but the two bits we want, and
		// then we shift everything 28 bits to the right
		unsigned l2Size = (newValue & L2SIZM) >> 28;
		
		// 01 means 256K, 10 means 512K, 11 means 1024K, and 00 means 2048
		// so we test for 3, and make it 4. Test for 0 and make it 8.
		// Then we have straight multiplication
		switch (l2Size) {
			case 3:
				l2Size = 4;
				break;
			case 0:
				l2Size = 8;
				break;
		}
		
		per_proc_info[0].pf.l2Size = l2Size * 256 * 1024;
		
		// Now, check if we are a G4. If so, set the flush assist available, in
		// case it helps.
		if (pvr == PVR7400) per_proc_info[0].pf.Available |= (pfL1fa | pfL2fa);
		
		return newValue; 
	} else { 
	
		// we didn't find one
		return oldValue;
	}
	
	return 0;
}
