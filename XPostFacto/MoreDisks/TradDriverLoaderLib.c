/*
	File:		TradDriverLoaderLib.c

	Contains:	Implementation for the pseudo-DriverLoaderLib for 'DRVR's.

	Written by:	Quinn

	Copyright:	Copyright (c) 1996-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):

$Log$
Revision 1.2  2003/01/05 22:32:19  ryan
Fixed the line endings :-)

/*
	File:		TradDriverLoaderLib.c

	Contains:	Implementation for the pseudo-DriverLoaderLib for 'DRVR's.

	Written by:	Quinn

	Copyright:	Copyright (c) 1996-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):

Revision 1.1  2003/01/05 20:58:02  ryan
/*
	File:		TradDriverLoaderLib.c

	Contains:	Implementation for the pseudo-DriverLoaderLib for 'DRVR's.

	Written by:	Quinn

	Copyright:	Copyright (c) 1996-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):

Initial check-in. For getting partition numbers of Firewire volumes.
/*
	File:		TradDriverLoaderLib.c

	Contains:	Implementation for the pseudo-DriverLoaderLib for 'DRVR's.

	Written by:	Quinn

	Copyright:	Copyright (c) 1996-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):


Revision 1.4  2001/11/07 15:58:54  eskimo1
Tidy up headers, add CVS logs, update copyright.


         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>      5/6/00    Quinn   Missed a check for MemError in TradInstallDriverFromHandle. 
                                    Thanks to Jim Luther for spotting this.
         <1>     25/2/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <LowMem.h>
	#include <DriverGestalt.h>
	#include <TextUtils.h>
#endif

// Switched from using:
//
//   #include <PLStringFuncs.h>
//
// to using BlockMoveData because it's so hard to get PLstrcpy working
// across a zillion different compilers.  *sigh*

/////////////////////////////////////////////////////////////////
// MIB Prototypes

#include "MoreInterfaceLib.h"

/////////////////////////////////////////////////////////////////
// Our Prototypes

#include "TradDriverLoaderLib.h"

///////////////////////////////////////////////////////////////////////////

extern pascal SInt16 TradHigherDriverVersion(NumVersion *dv1, NumVersion *dv2)
{
	UInt16 nonRelRev1, nonRelRev2;

	if (dv1->majorRev           > dv2->majorRev)        return  1;
	if (dv1->majorRev           < dv2->majorRev)        return -1;
	if (dv1->minorAndBugRev     > dv2->minorAndBugRev)  return  1;
	if (dv1->minorAndBugRev     < dv2->minorAndBugRev)  return -1;
	if (dv1->stage              > dv2->stage)           return  1;
	if (dv1->stage              < dv2->stage)           return -1;

	nonRelRev1 = dv1->nonRelRev;
	nonRelRev2 = dv2->nonRelRev;
	
	if (dv1->stage == finalStage) {
		if (dv1->nonRelRev == 0) 				nonRelRev1 = 0xFFFF;
		if (dv2->nonRelRev == 0) 				nonRelRev2 = 0xFFFF;
	}

	if (nonRelRev1 > nonRelRev2)                        return  1;
	if (nonRelRev1 < nonRelRev2)                        return -1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

extern pascal UnitNumber TradHighestUnitNumber(void)
	// See comment in header file.
{
	return (MoreLMGetUnitTableEntryCount() - 1);
}

///////////////////////////////////////////////////////////////////////////

extern pascal Boolean TradDriverGestaltIsOn(DriverFlags flags)
	// See comment in header file.
{
	return ( (flags & kmDriverGestaltEnableMask) != 0 );
}

///////////////////////////////////////////////////////////////////////////

static OSErr DriverGestaltOnOff(DriverRefNum refNum, Boolean setIt)
	// This routine is called by TradDriverGestaltOn and
	//  TradDriverGestaltOff to either set or clear the
	//  kmDriverGestaltEnableMask bit in the DCE flags.
{
	OSErr err;
	AuxDCEHandle thisDCE;
	
	// First called TradGetDriverInformation to validate the refNum
	//  and verify that the driver exists.
	err = TradGetDriverInformation(refNum, nil, nil, nil, nil);
	if (err == noErr) {
		thisDCE = (AuxDCEHandle) GetDCtlEntry(refNum);
		if (setIt) {
			(**thisDCE).dCtlFlags |= kmDriverGestaltEnableMask;
		} else {
			(**thisDCE).dCtlFlags &= ~kmDriverGestaltEnableMask;
		}
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradDriverGestaltOn(DriverRefNum refNum)
	// See comment in header file.
{
	return DriverGestaltOnOff(refNum, true);
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradDriverGestaltOff(DriverRefNum refNum)
	// See comment in header file.
{
	return DriverGestaltOnOff(refNum, false);
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradOpenInstalledDriver(DriverRefNum refNum, SInt8 ioPermission)
	// See comment in header file.
{
	OSErr 				err;
	Str255				driverName;
	DriverRefNum	realRefNum;

	// Check parameters.
	err = noErr;
	if (ioPermission != fsRdWrPerm) {
		err = paramErr;
	}
	
	// Get the name of the driver, then simply open it.
	if (err == noErr) {
		err = TradGetDriverInformation(refNum, nil, nil, driverName, nil);
	}
	if (err == noErr) {
		if ( driverName[0] == 0 ) {
			err = paramErr;
		}
	}
	if (err == noErr) {
		err = OpenDriver(driverName, &realRefNum);
	}
	if (err == noErr) {
		if (realRefNum != refNum) {
			err = paramErr;		// My favourite error code -- at some intrinsic level, every error is a paramErr (-;
		}
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradLookupDrivers(UnitNumber beginningUnit,
										UnitNumber endingUnit,
										Boolean emptyUnits,
										ItemCount *returnedRefNums, 
										DriverRefNum *refNums)
	// See comment in header file.
{
	OSErr err;
	AuxDCEHandle 	*unitTable;
	ItemCount 		maxRefNums;
	UnitNumber 		currentUnit;
	
	// Sanity check the parameters.
	if ( endingUnit > TradHighestUnitNumber() ) {
		endingUnit = TradHighestUnitNumber();
	}
	err = noErr;
	if ( beginningUnit > TradHighestUnitNumber() ) {
		err = badUnitErr;
	}
	if (err == noErr) {
		if (beginningUnit > endingUnit ) {
			err = paramErr;
		}
	}

	// Now do the real work...
	if (err == noErr) {
		unitTable = (AuxDCEHandle *) LMGetUTableBase();

		maxRefNums = *returnedRefNums;
		
		// Loop through each unit table entry from beginningUnit to endingUnit inclusive.
		*returnedRefNums = 0;
		currentUnit = beginningUnit;
		while ( currentUnit <= endingUnit ) {

			// If we've still got space to return a unit...
			if ( *returnedRefNums < maxRefNums ) {
			
				// and we're interested in this unit...
				if (	(emptyUnits && unitTable[currentUnit] == nil) ||
							(!emptyUnits && unitTable[currentUnit] != nil) ) {
					
					// then copy the unit out to the caller's array
					refNums[*returnedRefNums] = ~currentUnit;
					*returnedRefNums += 1;
				}
			}
			currentUnit += 1;
		}
	
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

enum {
	kNoUnitNumber = 0xFFFF
};

static UnitNumber IsDriverInstalled(ConstStr255Param name, UnitNumber skipThisUnit)
	// Look through the unit table to see if there is a driver with this name
	//  already installed.  Note that you might consider calling OpenDriver
	//  here, but that would be wrong.  OpenDriver has similar semantics, but
	//  if it fails to find a driver in the unit table it will search the
	//  current resource chain looking for a DRVR resource to install.
	//  Given that it's likely our client has a DRVR resource in their
	//  resource chain ('cause they're messing around trying to install
	//  drivers), and that OpenDriver will install it without detaching
	//  it from the client's resource file, and that the client's
	//  resource file may go away (ie they're a DropMounter-like application
	//  or some INIT running at system startup), this would be bad.
{
	UnitNumber	endingUnit;
	UnitNumber	unit;
	Str255			unitName;
	
	endingUnit = TradHighestUnitNumber();
	
	for (unit = 0; unit <= endingUnit; unit++) {
		if ( TradGetDriverInformation(~unit, nil, nil, unitName, nil) == noErr) {
			if ( unit != skipThisUnit && EqualString(name, unitName, false, true) ) {
				return unit;
			}
		}
	}
	
	return kNoUnitNumber;
}

///////////////////////////////////////////////////////////////////////////

enum {
	kMaximumNumberOfUnitTableEntries = 1024,
	// kMaximumNumberOfUnitTableEntries = 8000,
	
	// kMaximumNumberOfUnitTableEntries is documented in Technote
	//  DV 23 "Driver Education" <http://devworld.apple.com/dev/technotes/dv/dv_23.html>
	//  as being the maximum size that the classic Device Manager
	//  would grow the unit table.  In theory, this limits the system
	//  to 128 unit table entries.
	// However the traditional Mac OS is capable of dealing with much more
	//  than 128 units.  In fact, some multi-port serial card vendors
	//  regularly install more.
	// Prior to Mac OS 8, the PCI DriverLoaderLib enforced the 128 limit.
	//  I filed a bug against the PCI DriverLoaderLib to get this limit
	//  raised, and the new limit under Mac OS 8 is 1024.
	// Given that official sanction, I have now raised the standard
	//  limit enforced by this library to 1024.  I supply an alternative
	//  maximum (8000), designed to keep the unit table smaller than 32K.
	//  This is important because many people use 68K word indexing
	//  (ie x(a0,d0.w) to to access the entries.
	// I have tested TradDriverLoaderLib installing up to 500 device
	//  drivers.  
	
	kNumberOfEntriesToGrowUnitTable = 4
	
	// Technote DV 23 "Driver Education"
	//  <http://devworld.apple.com/dev/technotes/dv/dv_23.html>
	//  documents that the system grows the unit table by 4 entries
	//  at a time.
};

///////////////////////////////////////////////////////////////////////////

static OSErr GrowUnitTable()
	// This routine grows the unit table by kNumberOfEntriesToGrowUnitTable,
	//  up to a maximum of kMaximumNumberOfUnitTableEntries.  The routine
	//  is guaranteed to grow the table by at least one entry, or fail
	//  with an error.
{
	OSErr		err;
	Ptr 		oldTable;
	Ptr 		newTable;
	UInt32	oldCount;
	UInt32	newCount;

	// Get the info about the old table, and calculate the new table size.	
	oldTable = LMGetUTableBase();
	oldCount = MoreLMGetUnitTableEntryCount();
	newCount = oldCount + kNumberOfEntriesToGrowUnitTable;

	// Guard against growing the table too big.	
	err = noErr;
	if (newCount > kMaximumNumberOfUnitTableEntries) {
		err = unitTblFullErr;
	}
	
	// Allocate the new unit table in the system heap.  Note that we
	//  clear the newly allocated memory, so that later on, when we
	//  use this memory as the new unit table, the newly allocated
	//  entries will be empty.

	if (err == noErr) {
		newTable = NewPtrSysClear( newCount * sizeof(AuxDCEHandle));
		err = MemError();
	}

	// Copy the unit table entries over to the new table and then switch
	//  to that table.  Note that this sequence doesn't disable interrupts,
	//  instead it relies on the fact that programs can't modify the
	//  unit table at interrupt time, and thus we, running at non-interrupt
	//  time, have exclusive write access to the table.

	// Note that the sequence of these next few lines is *very* important.
	//  If we did this in any other order, you could get to a situation
	//  where interrupt code might be looking at an inconsistent 
	//  unit table, which would be bad.
	
	// The sequence is:
	//  1. copy the old unit table entries to the new table
	//  2. change the unit table base pointer, so that interrupts
	//		 start using the new unit table
	//  3. then change the unit table count, so that we have
	//		 more entries available
	
	if (err == noErr) {
		BlockMoveData(oldTable, newTable, oldCount * sizeof(AuxDCEHandle));	// 1.
		LMSetUTableBase(newTable);																					// 2.
		MoreLMSetUnitTableEntryCount(newCount);																	// 3.
		
		// Now its safe to dispose of the old unit table.
		DisposePtr(oldTable);
	}

	return err;
}

///////////////////////////////////////////////////////////////////////////

static OSErr FindFreeUnitNumber(UnitNumber beginningUnit,
								UnitNumber endingUnit, 
								UnitNumber *foundUnit)
	// This routine walks the unit table looking for a free
	//  slot.  The slot must be between beginningUnit
	//  and endingUnit.  If endingUnit is greater than
	//  TradHighestUnitNumber(), then we're allowed
	//  to grow the unit table to meet our needs.
{
	OSErr err;
	Boolean found;
	UnitNumber currentUnit;
	UnitNumber trueEndingUnit;
	AuxDCEHandle *unitTable;
	
	unitTable = (AuxDCEHandle *) LMGetUTableBase();
	
	// Find trueEndingUnit, which is the minimum
	//  of endingUnit and the highest unit number.
	trueEndingUnit = endingUnit;
	if ( trueEndingUnit > TradHighestUnitNumber() ) {
		trueEndingUnit = TradHighestUnitNumber();
	}

	// Scan through the unit table, starting at beginningUnit
	//  and ending at trueEndingUnit, looking for an
	//  empty slot.
	currentUnit = beginningUnit;
	found = false;
	while (currentUnit <= trueEndingUnit && !found) {
		found = (unitTable[currentUnit] == nil);
		if (!found) {
			currentUnit += 1;
		}
	}

	// Finish up.	
	if (found) {
		// We found an empty slot, return it.
		*foundUnit = currentUnit;
		err = noErr;
	} else {

		// We didn't find an empty slot.  If we're
		//  allowed to, grow the unit table, otherwise
		//  just return an error.
		
		if (endingUnit > trueEndingUnit) {
			err = GrowUnitTable();
			if (err == noErr) {
				*foundUnit = trueEndingUnit + 1;
			}
		} else {
			err = unitTblFullErr;
		}
	}
	
	return err;	
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradInstallDriverFromPtr(DRVRHeaderPtr driver,
												UnitNumber beginningUnit,
												UnitNumber endingUnit,
												DriverRefNum *refNum)
	// See comment in header file.
{
	OSErr err;
	UnitNumber foundUnit;
	AuxDCEHandle theDCE;
	
	// Sanity check parameters.
	err = noErr;
	if ( driver == nil ) {
		err = paramErr;
	}
	if ( beginningUnit > TradHighestUnitNumber() ) {
		err = badUnitErr;
	}
	if ( err == noErr && beginningUnit > endingUnit ) {
		err = paramErr;
	}
	
	// Check whether this driver is already installed.
	if ( err == noErr ) {
		// Check whether it's already installed.
		foundUnit = IsDriverInstalled(&driver->drvrName[0], kNoUnitNumber);
		if (foundUnit != kNoUnitNumber) {
			// Return the refnum of the existing driver to the caller.
			*refNum = ~foundUnit;
			err = dupFNErr;
		}
	}
	
	// Now walk the unit table looking for a free slot.
	if (err == noErr) {
		err = FindFreeUnitNumber(beginningUnit, endingUnit, &foundUnit);
	}

	// We've got a free slot, so let's install the device driver.
	//  Note that we use DriverInstallReserveMem, rather than the standard
	//  DriverInstall, so that the DCE is allocated low in the system
	//  heap.  DriverInstallReserveMem was introduced with the 128K ROM.

	if (err == noErr) {
		err = MoreDriverInstallReserveMem(driver, ~foundUnit);
	}
	
	// Now do some important tidying up.
	if (err == noErr) {

		// Return the refNum to the caller.
		*refNum = ~foundUnit;

		theDCE = (AuxDCEHandle) GetDCtlEntry(*refNum);
		
		// Now setup the DCE properly.  There's a whole pile of things we
		//  have to do, mainly because DriverInstall is such a brain-dead
		//  routine.
		
		// First up, DriverInstall seems to ignore the first parameter
		//  passed to it, so we have to blat the pointer to the driver code in
		//  yourself afterwards.
		
		(**theDCE).dCtlDriver = (Ptr) driver;

		// Then we have to set up the flags.  We do this by copying the flags
		//  out of the first word of the driver code.  We make sure to clear
		//  the dRAMBased bit because we're actually a pointer-based driver
		//  and DriverInstallReserveMem sets it to provisionally indicate that
		//  we're a handle based driver.  We also set dNeedLock because
		//  we want the the Device Manager to lock down the DCE.
		
		(**theDCE).dCtlFlags = (driver->drvrFlags & ~dRAMBasedMask) | dNeedLockMask;

		// There's also a bunch of fields we copy straight across without
		//  any modification.  You might expect DriverInstall to copy
		//  across these fields from the driver header to the DCE, but it doesn't
		//  do that, so we do it ourselves.

		(**theDCE).dCtlDelay = driver->drvrDelay;
		(**theDCE).dCtlEMask = driver->drvrEMask;
		(**theDCE).dCtlMenu  = driver->drvrMenu;

		// Finally, we lock the DCE.
		// Note that strictly speaking we don't need to HLock the DCE
		//  because the Device Manager will do it when it you open a driver
		//  that has dNeedLock set.  However, we want to
		//  lock it now because DriverInstallReserveMem has just made sure
		//  that the DCE was created low in the system heap, so we might as
		//  well lock it down low rather than let it float.

		HLock( (Handle) theDCE );
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradInstallDriverFromHandle(DRVRHeaderHandle driver,
												UnitNumber beginningUnit,
												UnitNumber endingUnit,
												DriverRefNum *refNum)
	// See comment in header file.
{
	OSErr err;
	Size  driverSize;
	DRVRHeaderPtr driverPtr;
	
	driverPtr = nil;
	
	err = noErr;
	if (driver == nil || *driver == nil) {
		err = paramErr;
	}
	if (err == noErr) {
		driverSize = GetHandleSize( (Handle) driver );
		err = MemError();
	}
	if (err == noErr) {
		driverPtr = (DRVRHeaderPtr) NewPtrSys( driverSize );
		err = MemError();
	}
	
	if (err == noErr) {
		// This is *not* a BlockMoveData call. This time, we really are moving code!
		//  I could have put cache flushing code in here, but then I would have
		//  had to check whether it was available or not.
		BlockMove( *driver, driverPtr, driverSize );
		
		err = TradInstallDriverFromPtr(driverPtr, beginningUnit, endingUnit, refNum);
	}
	
	// Clean up.
	if (err != noErr) {
		// We're returning an error.  The API says we should leave the handle untouched,
		//  but we should definitely clean up our new copy of the driver code.
		if (driverPtr != nil) {
			DisposePtr( (Ptr) driverPtr );
		}
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradInstallDriverFromResource(SInt16 rsrcID, StringPtr rsrcName,
												UnitNumber beginningUnit,
												UnitNumber endingUnit,
												DriverRefNum *refNum)
	// See comment in header file.
{
	OSStatus err;
	Handle driverHandle;
	
	// Note: We don't care which zone the resource gets loaded, because 
	//  TradInstallDriverFromHandle makes a copy of it anyway.

	// Get the resource, using either rsrcID or rsrcName.
	if (rsrcName == nil) {
		driverHandle = Get1Resource('DRVR', rsrcID);
	} else {
		driverHandle = Get1NamedResource('DRVR', rsrcName);
	}
	
	// Set err if we couldn't get the resource.
	if (driverHandle == nil) {
		err = ResError();
		if (err == noErr) {
			err = resNotFound;
		}
	} else {
		// Make sure we're not killed by some clown making the 'DRVR' purgeable.
		HNoPurge(driverHandle);					
		err = MemError();
	}
	
	// Now install the driver as if we'd got it from a memory handle.	
	if (err == noErr) {
		err = TradInstallDriverFromHandle( (DRVRHeaderHandle) driverHandle, beginningUnit, endingUnit, refNum);

		ReleaseResource(driverHandle);
		if (err == noErr) {
			err = ResError();
		}
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradGetDriverInformation(DriverRefNum refNum,
												UnitNumber *thisUnit,
												DriverFlags *flags,
												StringPtr name,
												DRVRHeaderPtr *driverHeader
												)
	// See comment in header file.
{
	OSErr err;
	UnitNumber 			tmpUnit;
	AuxDCEHandle		tmpDCE;
	DRVRHeaderPtr		tmpHeader;
	DRVRHeaderHandle	tmpDriverHandle;
	
	// Get some initial information.
	tmpUnit = ~refNum;
	
	// Sanity check the refNum parameter.
	err = noErr;
	if (tmpUnit > TradHighestUnitNumber()) {
		err = badUnitErr;
	}
	if (err == noErr) {
		tmpDCE = (AuxDCEHandle) GetDCtlEntry(refNum);
		if ( tmpDCE == nil ) {
			err = unitEmptyErr;
		}
	}
	if (err == noErr) {
		if ( (*tmpDCE == nil) || (GetHandleSize( (Handle) tmpDCE) < sizeof(DCtlEntry)) ) {
			err = dceExtErr;
		}
	}
	
	// Get the information from the DCE.
	if (err == noErr) {

		// From the DCE, find the DRVR header.  This can fail for a number of reasons:
		// 	1. dCtlDriver is nil
		// 	2. the driver is handle based, and the handle's master point is nil
		// 	3. the driver is handle based, and the driver's handle is too small
		// In all of these cases, we set tmpHeader to nil, returning nil to our
		// client.
		
		tmpHeader = (DRVRHeaderPtr) (**tmpDCE).dCtlDriver;
		if ( tmpHeader != nil ) {
			if ( ((**tmpDCE).dCtlFlags & dRAMBasedMask) != 0 ) {

				tmpDriverHandle = (DRVRHeaderHandle) tmpHeader;
				
				if ( (*tmpDriverHandle != nil) &&
							(GetHandleSize( (Handle) tmpDriverHandle) >= sizeof(DRVRHeader)) ) {
					tmpHeader = *tmpDriverHandle;
				}
			}
		}
		
		// Now copy out the various requested parameters
		if (thisUnit != nil) {
			*thisUnit = tmpUnit;
		}
		if (flags != nil) {
			*flags = (**tmpDCE).dCtlFlags;
		}
		if (name != nil) {
			if ( tmpHeader == nil ) {
				name[0] = 0;
			} else {
				BlockMoveData(&tmpHeader->drvrName[0], name, tmpHeader->drvrName[0] + 1);
			}
		}
		if (driverHeader != nil) {
			*driverHeader = tmpHeader;
		}
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

// Some extra dCtlFlags bits, as documented in:
// <http://developer.apple.com/qa/dv/dv27.html>

enum {
	dNativeDriverMask = 0x0008
};

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradRemoveDriver(DriverRefNum refNum, Boolean immediate)
	// See comment in header file.
{
	OSErr 				err;
	DriverFlags 	flags;
	DRVRHeaderPtr driverHeader;

	// Check parameters.
	err = noErr;
	if (immediate) {
		err = paramErr;
	}

	// Get information about the driver we're closing.
	if (err == noErr) {
		err = TradGetDriverInformation(refNum, nil, &flags, nil, &driverHeader);
	}
	if (err == noErr) {
		if ( driverHeader == nil || ((flags & dNativeDriverMask) != 0)) {
			err = paramErr;
		}
	}
	
	// If the driver is open, close it.
	if (err == noErr) {
		if ( (flags & dOpenedMask) != 0 ) {
			err = CloseDriver(refNum);
		}
	}
	
	// Now call the system to remove the driver from the unit table.  Note that this
	//  works because of a subtlety in DriverRemove.  If the driver being removed
	//  is a RAM-based driver (which our drivers aren't), DriverRemove will call
	//  ReleaseResource on the dCtlDriver.  We don't want this, so we make our drivers
	//  not RAM-based.
	
	if (err == noErr) {
		err = DriverRemove(refNum);
	}
	
	if (err == noErr) {
		// All is cool, so let's dispose of the code.
		DisposePtr( (Ptr) driverHeader);
	}
	
	return err;
}

///////////////////////////////////////////////////////////////////////////

extern pascal OSErr TradRenameDriver(DriverRefNum refNum, ConstStr255Param newDriverName)
	// See *important* comment in header file.
{
	OSErr 			err;
	Str255 			driverName;
	DRVRHeaderPtr 	driverHeader;
	DriverFlags		flags;
	
	err = noErr;
	if ( newDriverName[0] == 0 ) {
		err = paramErr;
	}
	if (err == noErr) {
		// Get information about the driver we're renaming.
		err = TradGetDriverInformation(refNum, nil, &flags, driverName, &driverHeader);
	}
	if (err == noErr) {
		if ( driverHeader == nil || ((flags & dNativeDriverMask) != 0)) {
			err = paramErr;
		}
	}
	
	// Now check the name lengths.  See comment in implementation for details.
	if (err == noErr) {
		if ( newDriverName[0] > driverName[0] ) {
			err = paramErr;
		}
	}
	
	// Now check whether the new name is already present in the unit table.
	if (err == noErr) {
		if ( IsDriverInstalled(newDriverName, ~refNum) != kNoUnitNumber ) {
			err = dupFNErr;
		}
	}
	
	// Now copy in the new driver name.
	if (err == noErr) {
		BlockMoveData( newDriverName, &driverHeader->drvrName[0], newDriverName[0] + 1 );
	}
	
	return err;
}
