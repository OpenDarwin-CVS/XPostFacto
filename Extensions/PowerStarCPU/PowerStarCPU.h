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

#ifndef _IOKIT_POWERSTARCPU_H
#define _IOKIT_POWERSTARCPU_H

#include <IOKit/IOCPU.h>

class PowerStarCPU : public IOCPU
{
	OSDeclareDefaultStructors (PowerStarCPU);
  
private:
	static IOCPUInterruptController *gCPUIC;

	const OSSymbol *ohare_sleepState;
	const OSSymbol *pmu_sleepNow;

	IOService *pmu;
	IOService *ohare;

    unsigned long tbLow, tbHigh, tbHigh2;
	
	virtual void ipiHandler (void *refCon, void *nub, int source);
	virtual void saveTimeBase(bool save);

public:
	virtual const OSSymbol *getCPUName(void);
  
	virtual bool           start(IOService *provider);
	virtual void           initCPU(bool boot);		// initial startup
	virtual void           quiesceCPU(void);    	// go to sleep
	virtual kern_return_t  startCPU(vm_offset_t start_paddr, vm_offset_t arg_paddr);  // start CPU (among others)
	virtual void           haltCPU(void);			// halt CPU (among others)
	virtual void           signalCPU(IOCPU *target);	// signal CPU
	virtual void           enableCPUTimeBase(bool enable);
};

#endif