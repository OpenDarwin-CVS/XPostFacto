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
 *  1 Dec 1998 suurballe  Created.
 */

#include <IOKit/IOSyncer.h>
#include "OpenPMURTCController.h"
#include "OpenPMU.h"

//#define super IORTCController
//OSDefineMetaClassAndStructors(OpenPMURTCController, IORTCController)

#define super IOService
OSDefineMetaClassAndStructors(OpenPMURTCController, IOService)


// **********************************************************************************
// init
//
// **********************************************************************************
bool OpenPMURTCController::init ( OSDictionary * regEntry, OpenPMU * driver )
{
PMUdriver = driver;

return super::init(regEntry);
}


// **********************************************************************************
// tickHandler
//
// **********************************************************************************
/* static */ void OpenPMURTCController::tickHandler ( IOService * us, UInt8, UInt32, UInt8 * )
{
((OpenPMURTCController *)us)->clientHandler(((OpenPMURTCController *)us)->tickClient);
}


// **********************************************************************************
// registerForClockTicks
//
// The RTC driver is calling to tell us that it is prepared to receive clock
// ticks every second.  The parameter block tells who to call when we get one.
//
// **********************************************************************************
void OpenPMURTCController::registerForClockTicks ( IOService * client, RTC_tick_handler handler )
{
    clientHandler = handler;
    tickClient = client;
    
   // We do not really care about these interrupts, but if we would this is the
   // way register for interrupts:
    /*
        if (!PMUdriver->registerForPMUInterrupts(kPMUoneSecInt, tickHandler, this)) {
#ifdef VERBOSE_LOGS_ON
            kprintf("OpenPMURTCController::registerForClockTicks registerForPMUInterrupts kPMUoneSecInt fails\n");
#endif // VERBOSE_LOGS_ON
        }
     */
}


// **********************************************************************************
// setRealTimeClock
//
// The RTC driver is calling to set the real time clock.  We translate this into
// a PMU command and enqueue it to our command queue.
//
// **********************************************************************************
IOReturn OpenPMURTCController::setRealTimeClock ( UInt8 * newTime )
{
    PMUrequest request;

    if ( newTime == NULL ) {
        return kPMUParameterError;
    }

    PMUdriver->sendMiscCommand (kPMUtimeWrite, (IOByteCount)4, newTime, NULL, NULL);

    return kPMUNoError;
}


// **********************************************************************************
// getRealTimeClock
//
// The RTC driver is calling to read the real time clock.  We translate this into
// a PMU command and enqueue it to our command queue.
//
// The length parameter is ignored on entry.  On exit it is set to the length
// in bytes of the time read.
// **********************************************************************************
IOReturn OpenPMURTCController::getRealTimeClock ( UInt8 * currentTime, IOByteCount * length )
{
    if ( currentTime == NULL ) {
        return kPMUParameterError;
    }

    PMUdriver->sendMiscCommand (kPMUtimeRead, 0, NULL, length, currentTime);

    return kPMUNoError;
}


