/*
	File:		MoreInterfaceLib.c

	Contains:	Compatibility shim for routines not in InterfaceLib.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
Revision 1.3  2003/10/21 16:51:55  ryan
Get rid of a few warnings in Mac OS X.

Revision 1.2  2003/01/05 22:32:18  ryan
Fixed the line endings :-)

/*
	File:		MoreInterfaceLib.c

	Contains:	Compatibility shim for routines not in InterfaceLib.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	File:		MoreInterfaceLib.c

	Contains:	Compatibility shim for routines not in InterfaceLib.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	File:		MoreInterfaceLib.c

	Contains:	Compatibility shim for routines not in InterfaceLib.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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


Revision 1.18  2001/11/07 15:52:50  eskimo1
Tidy up headers, add CVS logs, update copyright.


        <17>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <16>     8/28/01    gaw     Clean up commnets (no code changes)
        <15>     15/2/01    Quinn   Complain if you try to comile this file into a Carbon binary.
        <14>    22/12/00    Quinn   Added compatibility for some Mac OS 8.5 toolbox routines.
        <13>     30/5/00    Quinn   Tidied up the TimeTask routines.
        <12>      6/3/00    Quinn   Added modern Time Manager interfaces.
        <11>    18/10/99    Quinn   Convert MoreAssert's to, more correct, MoreAssertQ's.
        <10>     16/9/99    Quinn   Add FSM FCB accessors.
         <9>     16/8/99    Quinn   Added comment about nasty restriction with MorePBRemoteAccess.
         <8>     22/7/99    Quinn   Use correct ProcInfo in MoreAddDrive.
         <7>     15/6/99    Quinn   Added MoreBlockZero.
         <6>     15/6/99    Quinn   Fixed Gestalt Value glue for pre-8.5 systems.  Added Extended
                                    Disk Init Package routines.
         <5>     22/4/99    Quinn   Added PBRemoteAccess.
         <4>     20/4/99    Quinn   Added Gestalt Value routines.
         <3>     16/3/99    Quinn   Added MoreUTFindDrive and MoreAddDrive.  Also fixed some mixups
                                    about which traps are toolbox traps and which aren't.
         <2>      1/3/99    Quinn   Added MoreFlushCodeCacheRange.  Also some general tidy up.
         <1>     25/2/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <CodeFragments.h>
	#include <LowMem.h>
	#ifndef __MACH__
		#include <Traps.h>
		#include <FSM.h>
	#endif
	#include <Gestalt.h>
	#include <DiskInit.h>
#endif

/////////////////////////////////////////////////////////////////
// Our Prototypes

#include "MoreInterfaceLib.h"

/////////////////////////////////////////////////////////////////
//
// This file contains implementations of various Mac OS API routines
// that were rolled in to InterfaceLib as of Mac OS 8.5.  They are
// in the InterfaceLib stub library as of Universal Interfaces 3.2.
// This code does the magic to decide whether the routine is available
// via InterfaceLib, or whether we have to synthesise our own MixedMode
// glue (or, in some cases, implementation).
//
// If you use any of the routines in this library, you must either:
//
// a) continue to hard link to InterfaceLib, in which case you won't
//    launch on pre-8.5 systems (with a cryptic error message), or
// b) weak link to InterfaceLib, in which case you must check that you're
//    on 8.5 or later before calling any of these routine lest
//    you die by trying to call nil.
//
// You can change the definition of the following compiler variable,
// which causes these routine to always use the InterfaceLib variant.
// You'd better make sure they're available, as described above.

#ifndef MORE_MAC_OS_8_5_OR_LATER
	#define MORE_MAC_OS_8_5_OR_LATER 0
#endif

// Also, Universal Interfaces 3.2 defines macros for some missing
// low memory accessors.  For CFM code, these macros override
// the external procedure definition.  So you can no longer
// call the real accessors, even if you do depend on Mac OS 8.5
// or greater.  This hack fixes the problem.  [Radar ID 2308604]

#undef LMGetUnitTableEntryCount
#undef LMSetUnitTableEntryCount

#if TARGET_API_MAC_CARBON
    #error You should not include "MoreInterfaceLib.c" in your Carbon project.

    // However it is reasonable to include the *header* in your 
    // Carbon project.
    
    // This check also prevents this file being included in 
    // a Mach-O build, which is a good thing (-:
#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

extern pascal SInt16 MoreLMGetUnitTableEntryCount(void)
{
	if (MORE_MAC_OS_8_5_OR_LATER || (LMGetUnitTableEntryCount != (void *) kUnresolvedCFragSymbolAddress)) {
		return LMGetUnitTableEntryCount();
	} else {
		return *((SInt16 *) 0x01d2);
	}
}

extern pascal void   MoreLMSetUnitTableEntryCount(SInt16 value)
{
	if (MORE_MAC_OS_8_5_OR_LATER || (LMSetUnitTableEntryCount != (void *) kUnresolvedCFragSymbolAddress)) {
		LMSetUnitTableEntryCount(value);
	} else {
		*((SInt16 *) 0x01d2) = value;
	}
}

#endif

/////////////////////////////////////////////////////////////////
// Mixed Mode ProcInfo Helper
//
// Bit Numbers
//
//  3         2         1         0
// 10987654321098765432109876543210
//
// Stack Based Layout
//
//                             cccc <- calling convention = kPascalStackBased (0),
//                             cccc <- 					  = kCStackBased (1),
//                             cccc <- 					  = kThinkCStackBased (5)
//                           ss     <- result size
//                         ss       <- stack parameter 1 size
//                       ss         <- stack parameter 2 size
//                     ss           <- stack parameter 3 size
//                   ss             <- stack parameter 4 size
//                 ss               <- stack parameter 5 size
//               ss                 <- stack parameter 6 size
//             ss                   <- stack parameter 7 size
//           ss                     <- stack parameter 8 size
//         ss                       <- stack parameter 9 size
//       ss                         <- stack parameter 10 size
//     ss                           <- stack parameter 11 size
//   ss                             <- stack parameter 12 size
//
// Register Based Layout
//
//                             cccc <- calling convention = kRegisterBased (2)
//                      rrrss       <- register result location and size
//                 rrrss            <- register parameter 1 location and size
//            rrrss                 <- register parameter 2 location and size
//       rrrss                      <- register parameter 3 location and size
//  rrrss                           <- register parameter 4 location and size
//
// Dispatched Stack Based Layout
//
//                             cccc <- calling convention = kD0DispatchedPascalStackBased (8),
//                             cccc <-                    = kD0DispatchedCStackBased (9),
//                             cccc <-                    = kD1DispatchedPascalStackBased (12),
//                             cccc <-                    = kStackDispatchedPascalStackBased (14)
//                           ss     <- result size
//                         ss       <- selector size
//                       ss         <- stack parameter 1 size
//                     ss           <- stack parameter 2 size
//                   ss             <- stack parameter 3 size
//                 ss               <- stack parameter 4 size
//               ss                 <- stack parameter 5 size
//             ss                   <- stack parameter 6 size
//           ss                     <- stack parameter 7 size
//         ss                       <- stack parameter 8 size
//       ss                         <- stack parameter 9 size
//     ss                           <- stack parameter 10 size
//   ss                             <- stack parameter 11 size
//
// Special Case Layout
//
//                             cccc <- calling convention = kSpecialCase (15)
//                       xxxxxx     <- special case selector (see "MixedMode.h")
//
// Values for "ss":
//
// 00 = kNoByteCode
// 01 = kOneByteCode
// 10 = kTwoByteCode
// 11 = kFourByteCode
//
// Values for "rrr":
//
// 000 = kRegisterD0
// 001 = kRegisterD1
// 010 = kRegisterD2
// 011 = kRegisterD3
// 100 = kRegisterA0
// 101 = kRegisterA1
// 110 = kRegisterA2
// 111 = kRegisterA3

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

// DriverInstallReserveMem is a bit trickier to implement than the
// previous two routines.  Basically we have get the address of the
// _DriverInstall ($A03D) trap and then call it using
// CallOSTrapUniversalProc.  There are a number of important things
// to note here:
//   a) We can just get the trap address and treat it as a UPP.
//      The trap tables are defined to contain UPPs, either pointers
//      to real 68K code, or pointers to a routine descriptor (if
//      the trap is native or fat).
//   b) We must use CallOSTrapUniversalProc, not CallUniversalProc.
//      CallOSTrapUniversalProc automatically does a number of things 
//      that are very critical for call OS traps.  See "IM:PowerPC
//      System Software", p2-42 for a full description.
//   c) When calling OS traps from PPC, it's important to get the
//      ProcInfo right.  Specifically, all OS traps assume an
//      implicit parameter of D1 as the first parameter.  After
//      that, the parameters should be defined in the order that
//      they are declared in the prototype.  If you fail to get
//      the order right, or to put D1 first, your code will work,
//      up until it encounters someone who has patched the OS trap
//      with a native or fat patch.  Then strange things will happen,
//      and you'll spend hours in MacsBug trying to figure it out.
//
// The trap number and ProcInfo description for DriverInstallReserveMem.

enum {
	_DriverInstallReserveMem = 0x0A43D
};

enum {
	uppDriverInstallProcInfo = kRegisterBased
		| REGISTER_RESULT_LOCATION(kRegisterD0)
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(long)))
		| REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(DRVRHeaderPtr)))
		| REGISTER_ROUTINE_PARAMETER(3, kRegisterD0, SIZE_CODE(sizeof(short)))
};

// IMPORTANT: Previous versions of TradDriverLoaderLib defined parameter 1
// as sizeof(short), not sizeof(long).  I fixed this based on information
// I extracted from the official Apple version of this glue, in the 
// InterfaceLib implementation on Mac OS 8.5.

extern pascal OSErr  MoreDriverInstallReserveMem(DRVRHeaderPtr drvrPtr, DriverRefNum refNum)
{
	if (MORE_MAC_OS_8_5_OR_LATER || (DriverInstallReserveMem != (void *) kUnresolvedCFragSymbolAddress)) {
		return DriverInstallReserveMem(drvrPtr, refNum);
	} else {
		UniversalProcPtr trapAddress;
		
		// Check that I've got the ProcInfo correct.  This magic value
		// was extracted from InterfaceLib on Mac OS 8.5.

		MoreAssertQ(uppDriverInstallProcInfo == 0x00533822);
		
		trapAddress = GetOSTrapAddress(_DriverInstallReserveMem);
		return CallOSTrapUniversalProc(trapAddress, uppDriverInstallProcInfo, _DriverInstallReserveMem, drvrPtr, refNum);
	}
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

enum {
	uppFlushCodeCacheRangeProcInfo = kRegisterBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			REGISTER_RESULT_LOCATION(kRegisterD0) |
			REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(long))) |
			REGISTER_ROUTINE_PARAMETER(2, kRegisterD1, SIZE_CODE(sizeof(long))) |
			REGISTER_ROUTINE_PARAMETER(3, kRegisterA0, SIZE_CODE(sizeof(void *))) |
			REGISTER_ROUTINE_PARAMETER(4, kRegisterA1, SIZE_CODE(sizeof(unsigned long)))
};

extern pascal OSErr  MoreFlushCodeCacheRange(void *address, unsigned long count)
{
	if (MORE_MAC_OS_8_5_OR_LATER || (FlushCodeCacheRange != (void *) kUnresolvedCFragSymbolAddress)) {
		return FlushCodeCacheRange(address, count);
	} else {
		UniversalProcPtr trapAddress;
		
		// Check that I've got the ProcInfo correct.  This magic value
		// was extracted from InterfaceLib on Mac OS 8.5.

		MoreAssertQ(uppFlushCodeCacheRangeProcInfo == 0x5e671822);
		
		trapAddress = GetOSTrapAddress(_HWPriv);
		return CallOSTrapUniversalProc(trapAddress, uppFlushCodeCacheRangeProcInfo, 9, _HWPriv, address, count);
	}
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

enum {
	uppUTFindDriveProcInfo = kD0DispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(long))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(SInt16))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DrvQElPtr *))),
	uppUTLocateFCBProcInfo = kD0DispatchedPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
			 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kFourByteCode)
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(VCBPtr)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned long)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringPtr)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short*)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(FCBRecPtr*))),
	uppUTLocateNextFCBProcInfo = kD0DispatchedPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
			 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kFourByteCode)
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(VCBPtr)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned long)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringPtr)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short*)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(FCBRecPtr*))),
	uppUTIndexFCBProcInfo = kD0DispatchedPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
			 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kFourByteCode)
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(VCBPtr)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short*)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(FCBRecPtr*))),
	uppUTResolveFCBProcInfo = kD0DispatchedPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
			 | DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kFourByteCode)
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
			 | DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FCBRecPtr*)))
};

extern pascal OSErr MoreUTFindDrive(SInt16 driveNum, DrvQElPtr *driveQElementPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (UTFindDrive != (void *) kUnresolvedCFragSymbolAddress)) {
			return UTFindDrive(driveNum, driveQElementPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppUTFindDriveProcInfo == 0x0EE8);
			
			trapAddress = GetToolTrapAddress(_HFSUtilDispatch);
			return CallUniversalProc(trapAddress, uppUTFindDriveProcInfo, 15, driveNum, driveQElementPtr);
		}
}

extern pascal OSErr MoreUTLocateFCB(VCBPtr 				volCtrlBlockPtr,
								 unsigned long 			fileNum,
								 StringPtr 				namePtr,
								 short *				fileRefNum,
								 FCBRecPtr *			fileCtrlBlockPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (UTLocateFCB != (void *) kUnresolvedCFragSymbolAddress)) {
			return UTLocateFCB(volCtrlBlockPtr, fileNum, namePtr, fileRefNum, fileCtrlBlockPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppUTLocateFCBProcInfo == 0x03FFE8);
			
			trapAddress = GetToolTrapAddress(_HFSUtilDispatch);
			return CallUniversalProc(trapAddress, uppUTLocateFCBProcInfo, 2, volCtrlBlockPtr, fileNum, namePtr, fileRefNum, fileCtrlBlockPtr);
		}
}

extern pascal OSErr MoreUTLocateNextFCB(VCBPtr 			volCtrlBlockPtr,
								 unsigned long 			fileNum,
								 StringPtr 				namePtr,
								 short *				fileRefNum,
								 FCBRecPtr *			fileCtrlBlockPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (UTLocateNextFCB != (void *) kUnresolvedCFragSymbolAddress)) {
			return UTLocateNextFCB(volCtrlBlockPtr, fileNum, namePtr, fileRefNum, fileCtrlBlockPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppUTLocateNextFCBProcInfo == 0x03FFE8);
			
			trapAddress = GetToolTrapAddress(_HFSUtilDispatch);
			return CallUniversalProc(trapAddress, uppUTLocateNextFCBProcInfo, 3, volCtrlBlockPtr, fileNum, namePtr, fileRefNum, fileCtrlBlockPtr);
		}
}

extern pascal OSErr MoreUTIndexFCB(VCBPtr 				volCtrlBlockPtr,
								 short *				fileRefNum,
								 FCBRecPtr *			fileCtrlBlockPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (UTIndexFCB != (void *) kUnresolvedCFragSymbolAddress)) {
			return UTIndexFCB(volCtrlBlockPtr, fileRefNum, fileCtrlBlockPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppUTIndexFCBProcInfo == 0x03FE8);
			
			trapAddress = GetToolTrapAddress(_HFSUtilDispatch);
			return CallUniversalProc(trapAddress, uppUTIndexFCBProcInfo, 4, volCtrlBlockPtr, fileRefNum, fileCtrlBlockPtr);
		}
}

extern pascal OSErr MoreUTResolveFCB(short 				fileRefNum,
								 FCBRecPtr *			fileCtrlBlockPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (UTResolveFCB != (void *) kUnresolvedCFragSymbolAddress)) {
			return UTResolveFCB(fileRefNum, fileCtrlBlockPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppUTResolveFCBProcInfo == 0x0EE8);
			
			trapAddress = GetToolTrapAddress(_HFSUtilDispatch);
			return CallUniversalProc(trapAddress, uppUTResolveFCBProcInfo, 5, fileRefNum, fileCtrlBlockPtr);
		}
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

enum {
	uppAddDriveProcInfo = kRegisterBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			REGISTER_RESULT_LOCATION(kRegisterD0) |
			REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(short))) |
			REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(DrvQElPtr))) |
			REGISTER_ROUTINE_PARAMETER(3, kRegisterD0, SIZE_CODE(sizeof(long)))
};

// Some comments about the ProcInfo value:
//
// o It's set to return a result even though the high-level glue doesn't.
//   This is because the trap returns a result in D0.  We can safely ignored
//   it from the C code.
//
// o The D1 parameter is listed as sizeof(short), not sizeof(long) which is
//   the size used by most other OS traps.  The reason: I'm not sure.
//   Come to think of it, sizeof(short) makes more sense to me because
//   that's what trap words are.  Anyway, I'm just parroting what
//   InterfaceLib 8.5 does; my specific goal is to make CFM patches to
//   AddDrive work properly, which requires that we all use identical
//   ProcInfo's.
//
// o Register D0 contains the drvrRefNum and drvNum parameters combined,
//   with drvNum in the high word and drvrRefNum in the low word.
//   Oh, those wacky 68K calling conventions (-:

extern pascal void MoreAddDrive(DriverRefNum drvrRefNum, SInt16 drvNum, DrvQElPtr qEl)
{
	UInt32 response;
	
	if ( MORE_MAC_OS_8_5_OR_LATER || ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x0850))) {
		AddDrive(drvrRefNum, drvNum, qEl);
	} else {
		UniversalProcPtr trapAddress;

		// Prior to Mac OS 8.5, the InterfaceLib glue for AddDrive was
		// messed up, so we have to roll our own.
		
		// Check that I've got the ProcInfo correct.  This magic value
		// was extracted from InterfaceLib on Mac OS 8.5.

		MoreAssertQ(uppAddDriveProcInfo == 0x0733022);
		
		trapAddress = GetOSTrapAddress(_AddDrive);
		(void) CallOSTrapUniversalProc(trapAddress, uppAddDriveProcInfo, _AddDrive, qEl, ( ((UInt32) drvNum) << 16) | (UInt16) drvrRefNum);
	}
}

#else

// Unlike most of these glue routines, the 68K version of AddDrive 
// is implemented as link-in glue.  Thus there are no inlines in the
// interface file.  We therefore have to implement this routine for
// classic 68K, as a simple call through to the real routine.

extern pascal void MoreAddDrive(DriverRefNum drvrRefNum, SInt16 drvNum, DrvQElPtr qEl)
{
	AddDrive(drvrRefNum, drvNum, qEl);
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

enum {
	_GestaltValue = 0xABF1
};

enum {
	uppNewGestaltValueProcInfo = kD0DispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OSType))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long))),
	uppReplaceGestaltValueProcInfo = kD0DispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OSType))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long))),
	uppSetGestaltValueProcInfo = kD0DispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OSType))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long))),
	uppDeleteGestaltValueProcInfo = kD0DispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OSType)))
};

// The glue I use here is especially cheesy.  In general, I try
// to avoid using the pre-processor for generating bogus C syntax,
// ie I only use #if where I could syntactically use if().  However,
// in this case I've been forced to break my own rules.  The problem
// is that NewGestaltValue etc are not present in Universal Interface's
// 3.2 InterfaceLib for CFM-68K.  Why?  I don't know for sure, but it
// could be because they are only available in the real InterfaceLib for
// Mac OS 8.5 and higher, and that can't be run on 68K machines.  Anyway,
// given that they're not available in InterfaceLib, you can't reference
// the symbol in the CFM-68K build, which requires ugly preprocessor
// stuff.  There may be a better way of doing this, but I'm pushed for
// time right now.  I hope to revisit it eventually.
// -- Quinn, 20 Apr 1999

extern pascal OSErr MoreNewGestaltValue(OSType selector, long newValue)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (NewGestaltValue != (void *) kUnresolvedCFragSymbolAddress)) {
			return NewGestaltValue(selector, newValue);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppNewGestaltValueProcInfo == 0x00000FA8);
			
			trapAddress = GetToolboxTrapAddress(_GestaltValue);
			return CallUniversalProc(trapAddress, uppNewGestaltValueProcInfo, 0x0401, selector, newValue);
		}
}

extern pascal OSErr MoreReplaceGestaltValue(OSType selector, long replacementValue)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (ReplaceGestaltValue != (void *) kUnresolvedCFragSymbolAddress)) {
			return ReplaceGestaltValue(selector, replacementValue);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppReplaceGestaltValueProcInfo == 0x00000FA8);
			
			trapAddress = GetToolboxTrapAddress(_GestaltValue);
			return CallUniversalProc(trapAddress, uppReplaceGestaltValueProcInfo, 0x0402, selector, replacementValue);
		}
}

extern pascal OSErr MoreSetGestaltValue(OSType selector, long newValue)		
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (SetGestaltValue != (void *) kUnresolvedCFragSymbolAddress)) {
			return SetGestaltValue(selector, newValue);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppSetGestaltValueProcInfo == 0x00000FA8);
			
			trapAddress = GetToolboxTrapAddress(_GestaltValue);
			return CallUniversalProc(trapAddress, uppSetGestaltValueProcInfo, 0x0404, selector, newValue);
		}
}

extern pascal OSErr MoreDeleteGestaltValue(OSType selector)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (DeleteGestaltValue != (void *) kUnresolvedCFragSymbolAddress)) {
			return DeleteGestaltValue(selector);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppDeleteGestaltValueProcInfo == 0x000003A8);
			
			trapAddress = GetToolboxTrapAddress(_GestaltValue);
			return CallUniversalProc(trapAddress, uppDeleteGestaltValueProcInfo, 0x0203, selector);
		}
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

// Golly, this one is ugly.  In prior to ARA 3.0, this trap was
// implemented by 68K code that used D0 as a selector.  But
// with ARA 3.0, this trap is implemented by PowerPC code whose
// ProcInfo doesn't include the D0 selector.  So, you can either
// declare the ProcInfo to include the selector, and then you won't
// work with ARA 3.0 and higher, or you can declare it without
// the selector, in which case register D0 won't be set up
// and you won't work with ARA < 3.0.  Nasty.
//
// The long term solution is to check the version number of ARA
// and do the right thing for each version.  Alas, I don't have
// time to code and test that solution.  So I'm sticking with
// the ARA 3.x solution for the moment.
//
// Bletch!
//
// Quinn, 16 Aug 1999

enum {
	_RemoteAccess = 0xAA5B
};

enum {
	uppPBRemoteAccessProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(TPRemoteAccessParamBlock)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Boolean)))
};

extern pascal OSErr MorePBRemoteAccess(TPRemoteAccessParamBlock paramBlock, Boolean async)
	// There is still no InterfaceLib entry for PBRemoteAccess, so
	// we always do the CallUniversalProc thing.
{
	UniversalProcPtr trapAddress;
	
	trapAddress = GetToolboxTrapAddress(_RemoteAccess);
	return CallUniversalProc(trapAddress, uppPBRemoteAccessProcInfo, paramBlock, async);
}

#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

enum {
	uppDIXFormatProcInfo = kStackDispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Boolean))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(unsigned long))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(unsigned long *))),
	uppDIXZeroProcInfo = kStackDispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ConstStr255Param))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(unsigned long))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(void *))),
	uppDIReformatProcInfo = kStackDispatchedPascalStackBased |
			RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
			DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(ConstStr255Param))) |
			DISPATCHED_STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(ConstStr255Param)))
};

// These routines use the same cheesy glue structure as the Gestalt Value
// routines, for much the same reasons.  See the comment above MoreNewGestaltValue
// for more details.

extern pascal OSErr MoreDIXFormat(	 short 					drvNum,
									 Boolean 				fmtFlag,
									 unsigned long 			fmtArg,
									 unsigned long *		actSize)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (DIXFormat != (void *) kUnresolvedCFragSymbolAddress)) {
			return DIXFormat(drvNum, fmtFlag, fmtArg, actSize);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppDIXFormatProcInfo == 0x0000F6AE);
			
			trapAddress = GetToolboxTrapAddress(_Pack2);
			return CallUniversalProc(trapAddress, uppDIXFormatProcInfo, 0x000C, drvNum, fmtFlag, fmtArg, actSize);
		}
}

extern pascal OSErr MoreDIXZero(	 short 					drvNum,
									 ConstStr255Param 		volName,
									 short 					fsid,
									 short 					mediaStatus,
									 short 					volTypeSelector,
									 unsigned long 			volSize,
									 void *					extendedInfoPtr)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (DIXZero != (void *) kUnresolvedCFragSymbolAddress)) {
			return DIXZero(drvNum, volName, fsid, mediaStatus, volTypeSelector, volSize, extendedInfoPtr);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppDIXZeroProcInfo == 0x003EAEAE);
			
			trapAddress = GetToolboxTrapAddress(_Pack2);
			return CallUniversalProc(trapAddress, uppDIXZeroProcInfo, 0x000E, drvNum, volName, fsid, mediaStatus, volTypeSelector, volSize, extendedInfoPtr);
		}
}


extern pascal OSErr MoreDIReformat(	 short 					drvNum,
									 short 					fsid,
									 ConstStr255Param 		volName,
									 ConstStr255Param 		msgText)
{
	#if !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (DIReformat != (void *) kUnresolvedCFragSymbolAddress)) {
			return DIReformat(drvNum, fsid, volName, msgText);
		} else
	#endif
		{
			UniversalProcPtr trapAddress;
			
			// Check that I've got the ProcInfo correct.  This magic value
			// was extracted from InterfaceLib on Mac OS 8.5.

			MoreAssertQ(uppDIReformatProcInfo == 0x0000FAAE);
			
			trapAddress = GetToolboxTrapAddress(_Pack2);
			return CallUniversalProc(trapAddress, uppDIReformatProcInfo, 0x0010, drvNum, fsid, volName, msgText);
		}
}

#endif

/////////////////////////////////////////////////////////////////
//
// the routine uses the same cheesy glue structure as the Gestalt Value
// routines, for much the same reasons.  See the comment above MoreNewGestaltValue
// for more details.

extern pascal void MoreBlockZero(void * destPtr, Size byteCount)
{
	#if TARGET_RT_MAC_CFM && !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (BlockZero != (void *) kUnresolvedCFragSymbolAddress)) {
			BlockZero(destPtr, byteCount);
		} else 
	#endif
		{
			UInt8 *cursor;
			
			cursor = (UInt8 *) destPtr;
			while (byteCount != 0) {
				*cursor = 0;
				cursor += 1;
				byteCount -= 1;
			}
		}
}

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

// These routines were added into InterfaceLib in Mac OS 9.0.2.
// Unfortunately, they�re still not in the InterfaceLib stub
// library (UI 3.3.1b4).  So I don�t use the "try InterfaceLib
// then revert to glue" approach I use for many other routines.
// Instead, I always use the glue.  I need to fix this when
// InterfaceLib gets update.
//
// Also, I don�t have hard values for the ProcInfos because
// I haven�t had the opportunity to extract them from a Mac OS
// 9.0.2 (or above) system.  I need to fix that ASAP.

enum {
	uppInstallTimeTaskProcInfo = kRegisterBased
		| REGISTER_RESULT_LOCATION(kRegisterD0)
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(long)))
		| REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(QElemPtr))),
	uppInstallXTimeTaskProcInfo = kRegisterBased
		| REGISTER_RESULT_LOCATION(kRegisterD0)
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(long)))
		| REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(QElemPtr))),
	uppPrimeTimeTaskProcInfo = kRegisterBased
		| REGISTER_RESULT_LOCATION(kRegisterD0)
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(long)))
		| REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(QElemPtr)))
		| REGISTER_ROUTINE_PARAMETER(3, kRegisterD0, SIZE_CODE(sizeof(long))),
	uppRemoveTimeTaskProcInfo = kRegisterBased
		| REGISTER_RESULT_LOCATION(kRegisterD0)
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| REGISTER_ROUTINE_PARAMETER(1, kRegisterD1, SIZE_CODE(sizeof(long)))
		| REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(QElemPtr)))
};

extern pascal OSErr MoreInstallTimeTask(QElemPtr tmTaskPtr)
{
	UniversalProcPtr trapAddress;
	
	MoreAssertQ(uppInstallTimeTaskProcInfo == 0x00133822);
	
	trapAddress = GetOSTrapAddress(_InsTime);
	return CallOSTrapUniversalProc(trapAddress, uppInstallTimeTaskProcInfo, _InsTime, tmTaskPtr);
}

extern pascal OSErr MoreInstallXTimeTask(QElemPtr tmTaskPtr)
{
	UniversalProcPtr trapAddress;
	
	MoreAssertQ(uppInstallTimeTaskProcInfo == 0x00133822);
	
	trapAddress = GetOSTrapAddress(_InsTime);
	return CallOSTrapUniversalProc(trapAddress, uppInstallTimeTaskProcInfo, _InsXTime, tmTaskPtr);
}

extern pascal OSErr MorePrimeTimeTask(QElemPtr tmTaskPtr, long count)
{
	UniversalProcPtr trapAddress;
	
	MoreAssertQ(uppPrimeTimeTaskProcInfo == 0x00733822);
	
	trapAddress = GetOSTrapAddress(_PrimeTime);
	return CallOSTrapUniversalProc(trapAddress, uppPrimeTimeTaskProcInfo, _PrimeTime, tmTaskPtr, count);
}

extern pascal OSErr MoreRemoveTimeTask(QElemPtr tmTaskPtr)
{
	UniversalProcPtr trapAddress;
	
	MoreAssertQ(uppRemoveTimeTaskProcInfo == 0x00133822);
	
	trapAddress = GetOSTrapAddress(_RmvTime);
	return CallOSTrapUniversalProc(trapAddress, uppRemoveTimeTaskProcInfo, _RmvTime, tmTaskPtr);
}

#endif

/////////////////////////////////////////////////////////////////
extern pascal OSStatus MoreInvalWindowRect(WindowRef window, const Rect *bounds)
{
	OSStatus err;
	
	#if TARGET_RT_MAC_CFM && !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (InvalWindowRect != (void *) kUnresolvedCFragSymbolAddress)) {
			err = InvalWindowRect(window, bounds);
		} else 
	#endif
		{
			GrafPtr oldPort;
			
			GetPort(&oldPort);
			SetPort( (GrafPtr) GetWindowPort(window) );
			
			InvalRect(bounds);
			
			SetPort(oldPort);
			
			err = noErr;
		}
	return err;
}

extern pascal void MoreEnableMenuItem(MenuRef theMenu, MenuItemIndex item)
{
	#if TARGET_RT_MAC_CFM && !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (EnableMenuItem != (void *) kUnresolvedCFragSymbolAddress)) {
			EnableMenuItem(theMenu, item);
		} else 
	#endif
		{
			MoreAssertQ(item < 32);		// Old style APIs don�t handle items beyond 31.
			EnableItem(theMenu, item);
		}
}

extern pascal void MoreDisableMenuItem(MenuRef theMenu, MenuItemIndex item)
{
	#if TARGET_RT_MAC_CFM && !TARGET_CPU_68K
		if (MORE_MAC_OS_8_5_OR_LATER || (DisableMenuItem != (void *) kUnresolvedCFragSymbolAddress)) {
			DisableMenuItem(theMenu, item);
		} else 
	#endif
		{
			MoreAssertQ(item < 32);		// Old style APIs don�t handle items beyond 31.
			DisableItem(theMenu, item);
		}
}

/////////////////////////////////////////////////////////////////
