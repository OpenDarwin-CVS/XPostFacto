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
#include <ppc/proc_reg.h>
}

#include "PowerStarCPU.h"
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOPlatformExpert.h>

// Some defines for SPR access
#define __mtspr(spr, val)  __asm__ volatile("mtspr  " # spr ", %0" : : "r" (val))
#define __mfspr(reg, spr)  __asm__ volatile("mfspr  %0, " # spr : "=r" (reg))

#undef mtspr
#undef mfspr
#define mtspr(spr, val) __mtspr(spr, val)
#define mfspr(reg, spr) __mfspr(reg, spr)

#define L2CR		1017
#define PVR			287

#define PVR750		0x00080000		// a G3 processor
#define PVR7400		0x000C0000		// a G4 processor

extern "C" {
	extern void cacheInit (void);
	extern void cacheDisable (void);
}
	
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef super
#define super IOCPU

OSDefineMetaClassAndStructors(PowerStarCPU, IOCPU);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Static variable initialization

IOCPUInterruptController* PowerStarCPU::gCPUIC = NULL;

bool 
PowerStarCPU::start(IOService *provider)
{
	kern_return_t       result;
	ml_processor_info_t processor_info;

	ohare = NULL;
	pmu = NULL;

	if (!getPlatform()->metaCast ("ApplePowerStarPE")) return false;

	if (!super::start (provider)) return false;

	ohare_sleepState = OSSymbol::withCString ("ohare_sleepState");
	pmu_sleepNow = OSSymbol::withCString ("sleepNow");

	gCPUIC = new IOCPUInterruptController;
	if (gCPUIC == NULL) return false;
	if (gCPUIC->initCPUInterruptController (1) != kIOReturnSuccess) return false;
	gCPUIC->attach(this);
	gCPUIC->registerCPUInterruptController();
				
	unsigned pvr;
	mfspr (pvr, PVR);
	pvr &= 0xFFFF0000;
	unsigned l2cr = 0;
	if ((pvr == PVR750) || (pvr == PVR7400)) {
		mfspr (l2cr, L2CR);
		l2cr &= 0x7FFFFFFF;
	}

	processor_info.cpu_id           = (cpu_id_t) this;
	processor_info.boot_cpu         = true;
	processor_info.start_paddr      = 0x0100;
	processor_info.l2cr_value       = l2cr;
	processor_info.supports_nap     = false;
	processor_info.time_base_enable = (time_base_enable_t) &PowerStarCPU::enableCPUTimeBase;

	// Register this CPU with mach.
	result = ml_processor_register (&processor_info, &machProcessor, &ipi_handler);
	if (result == KERN_FAILURE) return false;
	
	setCPUState (kIOCPUStateUninitalized);

	processor_start (machProcessor);
  
	registerService ();
    
	return true;
}

void 
PowerStarCPU::initCPU (bool boot)
{
    if (ohare != NULL) {
        // we are waking up from sleep so:
        ohare->callPlatformFunction(ohare_sleepState, false, (void *)false, 0, 0, 0);
        ohare = NULL;
    } else {
        kprintf("PowerStarCPU::initCPU not found OHare\n");
	}

    // Restore time base after wake (since CPU's TBR was set to zero during sleep)
    if (!boot) saveTimeBase(false);

	if (boot) gCPUIC->enableCPUInterrupt(this);
	
	setCPUState (kIOCPUStateRunning);	
}

void 
PowerStarCPU::quiesceCPU(void)
{
	// Send PMU command to shutdown system before io is turned off
	kprintf ("PowerStarCPU::quiesceCPU telling pmu to sleep");
  if (pmu) {
        pmu->callPlatformFunction(pmu_sleepNow, false, 0, 0, 0, 0);
    } else {
        kprintf("PowerStarCPU::quiesceCPU can't find ApplePMU\n");
	}
	
	kprintf ("PowerStarCPU::quiesceCPU telling ohare to sleep\n");
    if (ohare) {
        ohare->callPlatformFunction(ohare_sleepState, false, (void *)true, 0, 0, 0);
    } else {
        kprintf("PowerStarCPU::quiesceCPU not found AppleOHare\n");
	}

   // Save time base before sleep since CPU's TBR will be set to zero at wake.
 	kprintf ("PowerStarCPU::quiesceCPU saving time base\n");
    saveTimeBase(true);

	// Set the wake vector to point to the reset vector
 	kprintf ("PowerStarCPU::quiesceCPU writing reset vector\n");
	ml_phys_write(0x0080, 0x100);

	kprintf ("PowerStarCPU::quiesceCPU nighty night");	
    ml_ppc_sleep();
	
}

kern_return_t 
PowerStarCPU::startCPU (vm_offset_t /*start_paddr*/, vm_offset_t /*arg_paddr*/)
{
	kprintf ("PowerStarCPU::startCPU unsupported\n");
	return KERN_FAILURE;
}

void 
PowerStarCPU::haltCPU (void)
{
	// we get these for use in quiesceCPU
	kprintf ("PowerStarCPU::haltCPU finding OpenPMU and AppleOHare\n");
	pmu = waitForService (serviceMatching ("OpenPMU"));
	ohare = waitForService (serviceMatching ("AppleOHare"));

	kprintf ("PowerStarCPU::haltCPU doing the processor_exit\n");

	processor_exit(machProcessor);

}

const OSSymbol*
PowerStarCPU::getCPUName(void)
{
    return OSSymbol::withCStringNoCopy("Primary0");
}
  
void 
PowerStarCPU::ipiHandler (void *refCon, void *nub, int source)
{
	// Call mach IPI handler for this CPU.
	if (ipi_handler) ipi_handler();
}

void
PowerStarCPU::signalCPU (IOCPU *target)
{
	kprintf ("PowerStarCPU::signalCPU unsupported\n");
}

void
PowerStarCPU::enableCPUTimeBase (bool enable)
{
	kprintf ("PowerSurgeCPU::enableCPUTimeBase unsupported\n");
}

void
PowerStarCPU::saveTimeBase(bool save)
{
    if(save) {        	// Save time base.
        do {
			tbHigh  = mftbu();
			tbLow   = mftb();
			tbHigh2 = mftbu();
        } while (tbHigh != tbHigh2);
    } else {		// Restore time base
        mttb(0);
        mttbu(tbHigh);
        mttb(tbLow);
    }
}



