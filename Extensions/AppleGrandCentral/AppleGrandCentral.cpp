/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright (c) 1999-2000 Apple Computer, Inc.  All rights reserved.
 *
 *  DRI: Josh de Cesare
 *
 */


#include <ppc/proc_reg.h>

#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IODeviceMemory.h>
#include <IOKit/IOPlatformExpert.h>

#include <IOKit/platform/AppleNMI.h>

#include "AppleGrandCentral.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super AppleMacIO

OSDefineMetaClassAndStructors(AppleGrandCentral, AppleMacIO);

bool AppleGrandCentral::start(IOService *provider)
{
  IOInterruptAction  handler;
  OSData *           data;
  AppleNMI           *appleNMI;
  IOService          *sixty6;
  long               nmiSource;
  OSData             *nmiData;
  IOReturn           error;
  
  // Call MacIO's start.
  if (!super::start(provider))
    return false;
    
  // Make sure the sixty6 node exists.
  if (provider->childFromPath("sixty6", gIODTPlane) == 0) {
    sixty6 = new IOService;
    if(sixty6->init()) {
      sixty6->setName("sixty6");
      sixty6->attachToParent(provider, gIODTPlane);
      sixty6->registerService();
    }
  }
  
  // get the base address of the this GrandCentral.
  grandCentralBaseAddress = fMemory->getVirtualAddress();

  // Necessary for Control NDRV.
  data = OSData::withBytes (&grandCentralBaseAddress, sizeof (grandCentralBaseAddress));
  if (data != 0) {
	provider->setProperty ("AAPL,address", data);
    data->release ();
  }
  
  // Make nubs for the children.
  publishBelow( provider );
  
  getPlatform()->setCPUInterruptProperties(provider);
  
  // Allocate the interruptController instance.
  interruptController = new AppleGrandCentralInterruptController;
  if (interruptController == NULL) return false;
  
  // call the interruptController's init method.
  error = interruptController->initInterruptController(provider, grandCentralBaseAddress);
  if (error != kIOReturnSuccess) return false;
  
  handler = interruptController->getInterruptHandlerAddress();
  provider->registerInterrupt(0, interruptController, handler, 0);
  
  provider->enableInterrupt(0);
  
  // Register the interrupt controller so client can find it.
  getPlatform()->registerInterruptController(gIODTDefaultInterruptController,
					     interruptController);
  
  // Create the NMI Driver.
  nmiSource = 20;
  nmiData = OSData::withBytes(&nmiSource, sizeof(long));
  appleNMI = new AppleNMI;
  if ((nmiData != 0) && (appleNMI != 0)) {
    appleNMI->initNMI(interruptController, nmiData);
  } 
  
  registerService ();

  initForPM (provider);
  
  return true;
}

bool
AppleGrandCentral::passiveMatch (OSDictionary *matching, bool changesOK)
{
	// I'm also matching with "Heathrow" now, which works around a problem in the AppleMesh driver.
	// The AppleMesh driver is calling waitForService (serviceMatching ("Heathrow")), and for some
	// reason this call is not returning on the unsupported powerbooks. Matching here makes the
	// call return. I suppose it is also sort of the appropriate thing to do--I could then
	// intercept the callPlatformFunction calls and "do the right thing."

	OSString *str = OSDynamicCast (OSString, matching->getObject (gIOProviderClassKey));
	if (str && str->isEqualTo ("Heathrow")) return true;
	return super::passiveMatch (matching, changesOK);
}

IOService* 
AppleGrandCentral::createNub (IORegistryEntry *from)
{
    IOService *nub = new AppleGrandCentralDevice;

    if (nub && !nub->init (from, gIODTPlane)) {
		nub->release ();
		nub = 0;
    }

    return (nub);
}

// Joe van Tunen figured out that we need some power management code here

void 
AppleGrandCentral::initForPM (IOService *provider) 
{ 
	PMinit ();						// initialize superclass variables
	provider->joinPMtree (this);	// attach into the power management hierarchy

#define kNumberOfPowerStates 2

	static IOPMPowerState ourPowerStates[kNumberOfPowerStates] = {
		{1,0,0,0,0,0,0,0,0,0,0,0},
		{1,IOPMDeviceUsable,IOPMPowerOn,IOPMPowerOn,0,0,0,0,0,0,0,0}
	};

	// register ourselves with ourself as policy-maker 
	if (pm_vars != NULL) registerPowerDriver (this, ourPowerStates, kNumberOfPowerStates);
} 

IOReturn 
AppleGrandCentral::setPowerState (unsigned long powerStateOrdinal, IOService* whatDevice) 
{ 
	// Do not do anything if the state is invalid.
	if (powerStateOrdinal >= kNumberOfPowerStates) return IOPMAckImplied; 

	if (powerStateOrdinal == 0) { 
		kprintf("gc would be powered off here\n"); 
	} 

	if (powerStateOrdinal == 1) { 
		kprintf("gc would be powered on here\n"); 
	} 

	return IOPMAckImplied; 
} 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef  super
#define super IOInterruptController

OSDefineMetaClassAndStructors(AppleGrandCentralInterruptController, IOInterruptController);

IOReturn AppleGrandCentralInterruptController::initInterruptController(IOService *provider, IOLogicalAddress interruptControllerBase)
{
  int cnt;
  
  parentNub = provider;
  
  // Allocate the task lock.
  taskLock = IOLockAlloc();
  if (taskLock == 0) return kIOReturnNoResources;
  
  // Allocate the memory for the vectors
  vectors = (IOInterruptVector *)IOMalloc(kNumVectors * sizeof(IOInterruptVector));
  if (vectors == NULL) {
    IOLockFree(taskLock);
    return kIOReturnNoMemory;
  }
  bzero(vectors, kNumVectors * sizeof(IOInterruptVector));
  
  // Allocate locks for the 
  for (cnt = 0; cnt < kNumVectors; cnt++) {
    vectors[cnt].interruptLock = IOLockAlloc();
    if (vectors[cnt].interruptLock == NULL) {
      for (cnt = 0; cnt < kNumVectors; cnt++) {
	IOLockFree(taskLock);
	if (vectors[cnt].interruptLock != NULL)
	  IOLockFree(vectors[cnt].interruptLock);
      }
      return kIOReturnNoResources;
    }
  }
  
  // Setup the registers accessors
  eventsReg = (unsigned long)(interruptControllerBase + kEventsOffset);
  maskReg   = (unsigned long)(interruptControllerBase + kMaskOffset);
  clearReg  = (unsigned long)(interruptControllerBase + kClearOffset);
  levelsReg = (unsigned long)(interruptControllerBase + kLevelsOffset);
  
  // Initialize the registers.
  
  // Disable all interrupts.
  stwbrx(0x00000000, maskReg);
  eieio();
  
  // Clear all pending interrupts.
  stwbrx(0xFFFFFFFF, clearReg);
  eieio();
  
  // Disable all interrupts. (again?)
  stwbrx(0x00000000, maskReg);
  eieio();
  
  return kIOReturnSuccess;
}

IOInterruptAction AppleGrandCentralInterruptController::getInterruptHandlerAddress(void)
{
  return (IOInterruptAction)&AppleGrandCentralInterruptController::handleInterrupt;
}

IOReturn AppleGrandCentralInterruptController::handleInterrupt(void * /*refCon*/,
							       IOService * /*nub*/,
							       int /*source*/)
{
  int               done;
  long              events, vectorNumber;
  IOInterruptVector *vector;
  unsigned long     maskTmp;

  do {
    done = 1;
    
    // Do all the sources for events, plus any pending interrupts.
    // Also add in the "level" sensitive sources
    maskTmp = lwbrx(maskReg);
    events = lwbrx(eventsReg) & ~kTypeLevelMask;
    events |= lwbrx(levelsReg) & maskTmp & kTypeLevelMask;
    events |= pendingEvents & maskTmp;
    pendingEvents = 0;
    eieio();

    // Since we have to clear the level'd one clear the current edge's too.
    stwbrx(kTypeLevelMask | events, clearReg);
    eieio();
    
    if (events) done = 0;
    
    while (events) {
      vectorNumber = 31 - cntlzw(events);
      events ^= (1 << vectorNumber);
      vector = &vectors[vectorNumber];
      
      vector->interruptActive = 1;
      sync();
      isync();
      if (!vector->interruptDisabledSoft) {
	isync();
	
	// Call the handler if it exists.
	if (vector->interruptRegistered) {
	  vector->handler(vector->target, vector->refCon,
			  vector->nub, vector->source);
	}
      } else {
	// Hard disable the source.
	vector->interruptDisabledHard = 1;
	disableVectorHard(vectorNumber, vector);
      }
      
      vector->interruptActive = 0;
    }
  } while (!done);
  
  return kIOReturnSuccess;
}

bool AppleGrandCentralInterruptController::vectorCanBeShared(long /*vectorNumber*/, IOInterruptVector */*vector*/)
{
  return true;
}

int AppleGrandCentralInterruptController::getVectorType(long vectorNumber, IOInterruptVector */*vector*/)
{
  int interruptType;
  
  if (kTypeLevelMask & (1 << vectorNumber)) {
    interruptType = kIOInterruptTypeLevel;
  } else {
    interruptType = kIOInterruptTypeEdge;
  }
  
  return interruptType;
}

void AppleGrandCentralInterruptController::disableVectorHard(long vectorNumber, IOInterruptVector */*vector*/)
{
  unsigned long     maskTmp;
  
  // Turn the source off at hardware.
  maskTmp = lwbrx(maskReg);
  maskTmp &= ~(1 << vectorNumber);
  stwbrx(maskTmp, maskReg);
  eieio();
}

void AppleGrandCentralInterruptController::enableVector(long vectorNumber,
						        IOInterruptVector *vector)
{
  unsigned long     maskTmp;
  
  maskTmp = lwbrx(maskReg);
  maskTmp |= (1 << vectorNumber);
  stwbrx(maskTmp, maskReg);
  eieio();
  if (lwbrx(levelsReg) & (1 << vectorNumber)) {
    // lost the interrupt
    causeVector(vectorNumber, vector);
  }
}

void AppleGrandCentralInterruptController::causeVector(long vectorNumber, IOInterruptVector */*vector*/)
{
  pendingEvents |= 1 << vectorNumber;
  parentNub->causeInterrupt(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef super
#define super AppleMacIODevice

OSDefineMetaClassAndStructors(AppleGrandCentralDevice, AppleMacIODevice);

IOReturn 
AppleGrandCentralDevice::getResources (void)
{
    return (((AppleGrandCentral *) getProvider())->getNubResources (this));
}


