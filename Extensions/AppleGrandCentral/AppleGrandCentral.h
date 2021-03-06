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

#ifndef _APPLE_GRANDCENTRAL_H
#define _APPLE_GRANDCENTRAL_H

#include <IOKit/platform/AppleMacIO.h>

#include <IOKit/IOInterrupts.h>
#include <IOKit/IOInterruptController.h>

#define kNumVectors      (32)

#define kTypeLevelMask (0x3FF00000)

#define kEventsOffset    (0x00020)
#define kMaskOffset      (0x00024)
#define kClearOffset     (0x00028)
#define kLevelsOffset    (0x0002C)


class AppleGrandCentralInterruptController;

class AppleGrandCentral : public AppleMacIO
{
  OSDeclareDefaultStructors(AppleGrandCentral);
  
private:
  IOLogicalAddress                     grandCentralBaseAddress;
  AppleGrandCentralInterruptController *interruptController;
  
public:
  virtual bool start(IOService *provider);
  virtual IOService* createNub (IORegistryEntry *from);
  virtual bool passiveMatch (OSDictionary * matching, bool changesOK = false);
  
  void initForPM (IOService *provider);
  IOReturn setPowerState (unsigned long powerStateOrdinal, IOService* whatDevice);
  
};


class AppleGrandCentralInterruptController : public IOInterruptController
{
  OSDeclareDefaultStructors(AppleGrandCentralInterruptController);
  
private:
  IOService         *parentNub;
  IOLock            *taskLock;
  unsigned long     pendingEvents;
  unsigned long     eventsReg;
  unsigned long     maskReg;
  unsigned long     clearReg;
  unsigned long     levelsReg;
  
public:
  virtual IOReturn initInterruptController(IOService *provider, IOLogicalAddress interruptControllerBase);
  
  virtual IOInterruptAction getInterruptHandlerAddress(void);
  virtual IOReturn handleInterrupt(void *refCon, IOService *nub, int source);
  
  virtual bool vectorCanBeShared(long vectorNumber, IOInterruptVector *vector);
  virtual int  getVectorType(long vectorNumber, IOInterruptVector *vector);
  virtual void disableVectorHard(long vectorNumber, IOInterruptVector *vector);
  virtual void enableVector(long vectorNumber, IOInterruptVector *vector);
  virtual void causeVector(long vectorNumber, IOInterruptVector *vector);
};


class AppleGrandCentralDevice : public AppleMacIODevice
{

	OSDeclareDefaultStructors (AppleGrandCentralDevice);

public:

	virtual IOReturn getResources (void);
    
};

#endif /* ! _APPLE_GRANDCENTRAL_H */
