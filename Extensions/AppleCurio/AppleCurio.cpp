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

	/* Copyright 1997-1999 Apple Computer Inc. All Rights Reserved.			*/
	/* @author  Martin Minow												*/
	/* Edit History															*/
	/* 1997.02.13	MM	Initial conversion from AMDPCSCSIDriver sources.	*/
	/*																		*/




#include <string.h>					/* bzero is defined in string.h */
#include <mach/clock_types.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/ppc/IODBDMA.h>
#include <IOKit/system.h>			/* Definition of flush_dcache	*/
#include <IOKit/scsi/IOSCSIParallelInterface.h>

#include "AppleCurio.h"




 	extern void		IOGetTime( mach_timespec_t *clock_time );
	extern void		kprintf( const char *, ... );
    extern void     call_kdp();     // for debugging


	static globals	g;	/**** Instantiate the globals ****/
	globals			*gCurio;

	OSDefineMetaClassAndStructors( CurioSCSIController, IOSCSIParallelController )	;


#define kIOReturnParityError		1	


    enum 						/*****	values for g.intLevel:		*****/
    {
        kLevelISR       = 0x80,		/* In Interrupt Service Routine		*/
        kLevelLocked    = 0x40,		/* Curio interrupts locked out		*/
        kLevelSIH       = 0x20,		/* In Secondary Interrupt Handler	*/
        kLevelLatched   = 0x10		/* Interrupt latched				*/
    };



#if USE_ELG
static void AllocateEventLog( UInt32 size )
{
    if ( g.evLogBuf )
		return;

	g.evLogFlag = 0;            /* assume insufficient memory   */
	g.evLogBuf = (UInt8*)kalloc( size );
	if ( !g.evLogBuf )
	{
		kprintf( "AllocateEventLog - CurioSCSIController evLog allocation failed " );
		return;
	}

	bzero( g.evLogBuf, size );
	g.evLogBufp	= g.evLogBuf;
	g.evLogBufe	= g.evLogBufp + kEvLogSize - 0x20; // ??? overran buffer?
//	g.evLogFlag  = 0xFEEDBEEF;
//	g.evLogFlag  = 'step';
	g.evLogFlag  = 0x0333;
    return;
}/* end AllocateEventLog */


static void EvLog( UInt32 a, UInt32 b, UInt32 ascii, char* str )
{
	register UInt32		*lp;           /* Long pointer      */
	mach_timespec_t		time;

	if ( g.evLogFlag == 0 )
		return;

	IOGetTime( &time );

    lp = (UInt32*)g.evLogBufp;
    g.evLogBufp += 0x10;

    if ( g.evLogBufp >= g.evLogBufe )       /* handle buffer wrap around if any */
    {    g.evLogBufp  = g.evLogBuf;
        if ( g.evLogFlag != 0xFEEDBEEF )
            g.evLogFlag = 0;                /* stop tracing if wrap undesired   */
    }

        /* compose interrupt level with 3 byte time stamp:  */

	*lp++ = (g.intLevel << 24) | ((time.tv_nsec >> 10) & 0x003FFFFF);   // ~ 1 microsec resolution
    *lp++ = a;
    *lp++ = b;
    *lp   = ascii;

    if( g.evLogFlag == 'step' )
	{	char	code[ 5 ];
		*(UInt32*)&code = ascii;		code[ 4 ] = 0;
	//	kprintf( "%8x curio: %8x %8x %s        %s\n", time.tv_nsec>>10, a, b, code, str );
		kprintf( "%8x curio: %8x %8x %s\n", time.tv_nsec>>10, a, b, code );
	}

    return;
}/* end EvLog */


static void Pause( UInt32 a, UInt32 b, UInt32 ascii, char* str )
{
    char        work [ 256 ];
    char        name[] = "CurioSCSIController:";
    char        *bp = work;
    UInt8       x;
    int         i;


    EvLog( a, b, ascii, str );
    EvLog( '****', '** P', 'ause', "*** Pause" );

    bcopy( name, bp, sizeof( name ) );
    bp += sizeof( name ) - 1;

    *bp++ = '{';                               // prepend p1 in hex:
    for ( i = 7; i >= 0; --i )
    {
        x = a & 0x0F;
        if ( x < 10 )
             x += '0';
        else x += 'A' - 10;
        bp[ i ] = x;
        a >>= 4;
    }
    bp += 8;

    *bp++ = ' ';                               // prepend p2 in hex:

    for ( i = 7; i >= 0; --i )
    {
        x = b & 0x0F;
        if ( x < 10 )
             x += '0';
        else x += 'A' - 10;
        bp[ i ] = x;
        b >>= 4;
    }
    bp += 8;
    *bp++ = '}';

    *bp++ = ' ';

    for ( i = sizeof( work ) - (int)(bp - work); i && (*bp++ = *str++); --i )   ;

//	kprintf( work );
//	panic( work );
//  call_kdp();         // ??? use kdp=3 in boot parameters
	Debugger( work );
    return;
}/* end Pause */
#endif // USE_ELG



bool CurioSCSIController::configure(	IOService			*provider,
										SCSIControllerInfo	*controllerInfo )
{
	IOReturn		ioReturn = kIOReturnInternalError;


	gCurio			= &g;
	g.intLevel		= 0;
	g.curioInstance	= this;
#if USE_ELG
    AllocateEventLog( kEvLogSize );
	ELG( g.evLogBufp, &g.evLogFlag, 'Curo', "configure - event logging set up." );
#endif /* USE_ELG */

	ELG( this, provider, 'Cnfg', "configure" );

    fProvider = (IOPCIDevice*)provider;

	ioReturn = initHardware();
	if ( ioReturn != kIOReturnSuccess )
		return false;

		/* Register our interrupt handler routine:	*/

    fInterruptEvent = IOInterruptEventSource::interruptEventSource(
					(OSObject*)this,
                    (IOInterruptEventAction)&CurioSCSIController::interruptOccurred,
					provider,
					0 );

    if ( fInterruptEvent == NULL )
    {
		ELG( 0, 0, 'IES-', "CurioSCSIController::configure - can't register interrupt action" );
        return false;
    }

    getWorkLoop()->addEventSource( fInterruptEvent );
    fInterruptEvent->enable();

		/* allocate a big-endian memory cursor:	*/

    fMemoryCursor = IOBigMemoryCursor::withSpecification( kMaxDMATransfer, kMaxDMATransfer );
    if ( fMemoryCursor == NULL )
    {
		ELG( 0, kMaxDMATransfer, 'Mem-', "CurioSCSIController::start - IOBigMemoryCursor::withSpecification NG" );
        return false;
    }


		/* Fill in the  SCSIControllerInfo structure and return:	*/

	controllerInfo->initiatorId				= 7;

	controllerInfo->maxTargetsSupported		= 8;
	controllerInfo->maxLunsSupported		= 8;

	controllerInfo->minTransferPeriodpS		= 0;	    /* Sync not supported   */
	controllerInfo->maxTransferOffset		= 0;
	controllerInfo->maxTransferWidth		= 1;

	controllerInfo->maxCommandsPerController= 0;		// 0 is unlimited
	controllerInfo->maxCommandsPerTarget	= 0;
	controllerInfo->maxCommandsPerLun		= 0;

	controllerInfo->tagAllocationMethod		= kTagAllocationPerLun;
	controllerInfo->maxTags					= 256;

	controllerInfo->commandPrivateDataSize	= sizeof( PrivCmdData );

	controllerInfo->disableCancelCommands	= false;

	return true;
}/* end configure */


void CurioSCSIController::executeCommand( IOSCSIParallelCommand *scsiCommand )
{
	SCSICDBInfo			scsiCDB;
	SCSITargetParms		targetParms;
	UInt8				msgByte;
	bool				rc;



	if ( fCmd || fBusBusy )
	{
		ELG( fCmd, scsiCommand, 'Busy', "CurioSCSIController::executeCommand - bus busy so bounce this cmd" );
		disableCommands();
		rescheduleCommand( scsiCommand );
		return;
	}

    fCmd 		= scsiCommand;
    fCmdData	= (PrivCmdData*)scsiCommand->getCommandData();
	bzero( fCmdData, sizeof( *fCmdData ) );

    scsiCommand->getTargetLun( &fCurrentTargetLun );
    scsiCommand->getCDB( &scsiCDB );
    scsiCommand->getDevice( kIOSCSIParallelDevice )->getTargetParms( &targetParms );

	ELG(	scsiCommand,
			*(UInt16*)&fCurrentTargetLun<<16 | (scsiCDB.cdbTag & 0xFF),
			'Exec',			"CurioSCSIController::executeCommand" );
	ELG( *(UInt32*)&scsiCDB.cdb[0], *(UInt32*)&scsiCDB.cdb[4] , '=CDB', "executeCommand - CDB" );

	fpMsgOut = fpMsgPut = &fMsgOutBuffer[ 0 ];

		/* Identify byte:	*/

    msgByte = kSCSIMsgIdentify | kSCSIMsgEnableDisconnectMask | fCurrentTargetLun.lun;
    if ( scsiCDB.cdbFlags & kCDBFlagsNoDisconnect )
         msgByte &= ~kSCSIMsgEnableDisconnectMask;
    *fpMsgOut++ = msgByte;

		/* Tag msg:	*/

    if ( scsiCDB.cdbTagMsg )
    {
        *fpMsgOut++ = scsiCDB.cdbTagMsg;
        *fpMsgOut++ = scsiCDB.cdbTag;
		ELG( 0, scsiCDB.cdbTagMsg<<16 | (scsiCDB.cdbTag & 0xFF), ' tag', "CurioSCSIController::executeCommand - tag" );
    }

		/* Abort msg:	*/

    if ( scsiCDB.cdbAbortMsg )
    {
		ELG( scsiCommand->getOriginalCmd(), scsiCDB.cdbAbortMsg, 'Abor', "CurioSCSIController::executeCommand - abort msg." );
        *fpMsgOut++ = scsiCDB.cdbAbortMsg;
    }

	fMsgOutCount = (UInt8)(fpMsgOut - &fMsgOutBuffer[ 0 ]);
	ELG( fMsgOutCount, *(UInt32*)&fMsgOutBuffer[0], '=MgO', "executeCommand" );

		/***** Try to start the command on the hardware:	*****/

	rc = startCommand();			/* Call the hardware layer.	*/

	if ( rc )
	{								/* Hardware can't start now	*/
		ELG( fCmd, 0, 'Exe-', "CurioSCSIController::executeCommand - command bounced back" );
		fCmd = NULL;		/// ?redundant? - rescheduleCommand done in fsmSelecting
		return;
	}

    return;
}/* end executeCommand */



bool CurioSCSIController::startCommand()
{
	SCSICDBInfo		scsiCDB;
	UInt8			selectCmd;


    fCmd->getPointers( &fCmdData->mdp, &fCmdData->xferCount, &fCmdData->isWrite );

	fCmdData->results.bytesTransferred	= 0;
	fCmdData->savedDataPosition			= 0;

	fCmd->getCDB( &scsiCDB );

	setCmdReg( cFlshFFO );
	WRITE_REGISTER( rSTA, fCurrentTargetLun.target );

		/* Put the contents of the message buffer into the FIFO.		*/
		/* Figure out which flavor of Select command is indicated.		*/
		/* The command depends on the number of message bytes.			*/

	switch ( fMsgOutCount )
	{
	case 0:				/* Select, no ATN, send CDB	    */
	    selectCmd = cSlctNoAtn;
	    break;

	case 1:				/* Select, ATN, 1 Msg, send CDB */
	    selectCmd = cSlctAtn;
	    break;

	case 3:				/* Select, ATN, 3 Msg, send CDB */
	    selectCmd = cSlctAtn3;
	    break;

	default:			/* Select, ATN, 1 Msg, stop	*/
	    selectCmd = cSlctAtnStp;
	    break;
	}/* end SWITCH on size of MsgOut */

	if ( (fMsgOutCount > 3) || (fMsgOutCount + scsiCDB.cdbLength >= 16) )
	{		/* The FIFO only holds 16 bytes.							*/
			/* We have to do the command phase in the state automaton.	*/
		WRITE_REGISTER( rFFO, *fpMsgPut++ );
	    selectCmd = cSlctAtnStp;
	}
	else
	{		/* MsgOut and CDB both fit in FIFO:	*/
	    while ( fpMsgPut < fpMsgOut )
		{
			WRITE_REGISTER( rFFO, *fpMsgPut++ );
		}
	}

	if ( selectCmd != cSlctAtnStp )
	{		/* If we're not stopping after the messages,	*/
			/* stuff the command into the FIFO, too.		*/
	    putCDBIntoFIFO();
	}
		/* Init the finite state automaton:	*/

	fBusState			= SCS_SELECTING;
	fCurrentBusPhase	= kBusPhaseBusFree;

		/***** Issue the Arbitrate/Select/MessageOut combo:	*****/

	setCmdReg( selectCmd );

	return false;					/* return No Problem		*/
}/* end startCommand */


void CurioSCSIController::completeCommand()
{
	ELG( fCmdData->results.returnCode, fCmdData->results.bytesTransferred, ' IOC', "CurioSCSIController::completeCommand" );

	switch ( fCmdData->results.scsiStatus )
	{
	case kSCSIStatusGood:
		break;

	case kSCSIStatusCheckCondition:
		ELG( 0, 0, 'Chek', "CurioSCSIController::completeCommand - Check Condition" );
		break;

	default:
		ELG( fCmd, fCmdData->results.scsiStatus, 'Sta?', "CurioSCSIController::completeCommand - bad status" );
		break;
	}/* end SWITCH on SCSI status */

    switch ( fCmdData->results.returnCode )
    {
        case kIOReturnInternalError:
            fCmdData->results.returnCode = kIOReturnSuccess;
            fCmdData->results.adapterStatus = kSCSIAdapterStatusProtocolError;
            break;
            
        case kIOReturnParityError:
            fCmdData->results.returnCode = kIOReturnSuccess;
            fCmdData->results.adapterStatus = kSCSIAdapterStatusParityError;
            break;
            
        case kIOReturnNoDevice:
            fCmdData->results.returnCode = kIOReturnSuccess;
            fCmdData->results.adapterStatus = kSCSIAdapterStatusSelectionTimeout;
            break;

        default:
            ;
    }            

    fCmd->setResults( &fCmdData->results );
    fCmd->complete();

	fCmd						= NULL;
	fCmdData					= NULL;
	fCurrentTargetLun.target	= kInvalidTarget;
	fCurrentTargetLun.lun		= kInvalidLUN;
	return;
}/* end completeCommand */


void CurioSCSIController::cancelCommand( IOSCSIParallelCommand *scsiCommand )
{
    IOSCSIParallelCommand	*origCmd;
 	PrivCmdData		*origCmdData;
	SCSIResults		results;

    origCmd		= scsiCommand->getOriginalCmd();
    origCmdData	= (PrivCmdData*)origCmd->getCommandData();

	ELG( scsiCommand, origCmd, 'Can-', "CurioSCSIController::cancelCommand" );

	origCmd->getResults( &results );
	results.bytesTransferred = origCmdData->results.bytesTransferred;
	origCmd->setResults( &results );

    origCmd->complete();

    scsiCommand->complete();
	return;
}/* end cancelCommand */


void CurioSCSIController::resetCommand( IOSCSIParallelCommand *scsiCommand )
{
	ELG( scsiCommand, 0, 'Rst-', "CurioSCSIController::resetCommand" );
	resetBus();
	fCmdData = (PrivCmdData*)scsiCommand->getCommandData();
	bzero( &fCmdData->results, sizeof( fCmdData->results ) );
	scsiCommand->setResults( &fCmdData->results );
    scsiCommand->complete();
	return;
}/* end resetCommand */


    /* Fetch the device's bus address and interrupt port number.  */
    /* Also, allocate one page of memory for the Channel Program. */

IOReturn CurioSCSIController::initHardware()
{
    IOReturn        ioReturn;


	fInitiatorID		= kInitiatorIDDefault;
	fInitiatorIDMask	= 1 << kInitiatorIDDefault;	/* BusID bitmask for reselection. */

	ELG( 0, fInitiatorID, 'IniH', "initHardware" );

	ioReturn = getHardwareMemoryMaps();

    if ( ioReturn == kIOReturnSuccess )	ioReturn = allocHdwAndChanMem();
    if ( ioReturn == kIOReturnSuccess )	ioReturn = doHBASelfTest();
    if ( ioReturn == kIOReturnSuccess )	ioReturn = resetHardware( true );

    if ( ioReturn != kIOReturnSuccess )
        this->free();

    return ioReturn;
}/* end initHardware */


IOReturn CurioSCSIController::getHardwareMemoryMaps()
{

	if ( !fSCSIMemoryMap )
	{
	    fSCSIMemoryMap = fProvider->mapDeviceMemoryWithIndex( kCurioRegisterBase );
	    if ( !fSCSIMemoryMap )
		{
			ELG( 0, 0, 'Map-', "CurioSCSIController::getHardwareMemoryMaps - can't map Curio." );
			return kIOReturnInternalError;
	    }

		fCurioPhysAddr	= fSCSIMemoryMap->getPhysicalAddress();
		fCurioAddr		= (UInt8*)fSCSIMemoryMap->getVirtualAddress();
		ELG( fCurioPhysAddr, fCurioAddr, '=Cur', "getHardwareMemoryMaps - Curio regs" );
		g.curioAddr = (UInt32)fCurioAddr;      // for debugging, miniMon ...
	}

	if ( !fDBDMAMemoryMap )
	{
	    fDBDMAMemoryMap	= fProvider->mapDeviceMemoryWithIndex( kDBDMARegisterBase );
	    if ( !fDBDMAMemoryMap )
		{
			ELG( 0, 0, 'map-', "CurioSCSIController::getHardwareMemoryMaps - can't map DBDMA." );
			return kIOReturnInternalError;
	    }
		fDBDMAAddrPhys	= fDBDMAMemoryMap->getPhysicalAddress();
		fDBDMAAddr		= (IODBDMAChannelRegisters*)fDBDMAMemoryMap->getVirtualAddress();
		ELG( fDBDMAAddrPhys, fDBDMAAddr, '=DMA', "getHardwareMemoryMaps - DBDMA regs" );
	}

	return kIOReturnSuccess;
}/* end getHardwareMemoryMaps */



    /* Fetch the device's bus address and allocate one page of memory   */
    /* for the channel command. (Strictly speaking, we don't need an    */
    /* entire page, but we can use the rest of the page for a permanent */
    /* status log).                                                     */
    /* @param   deviceDescription   Specify the device to initialize.   */
    /* @return  kIOReturnSuccess if successful, else an error status.       */

IOReturn CurioSCSIController::allocHdwAndChanMem()
{
    IOReturn    ioReturn = kIOReturnSuccess;


    fCCLSize  = page_size;
    fCCL      = (UInt8*)kalloc( fCCLSize );
    if ( !fCCL )
    {   PAUSE( 0, fCCLSize, 'CCA-', "allocHdwAndChanMem - can't allocate channel command area.\n" );
        ioReturn = kIOReturnNoMemory;
    }

    if ( ioReturn == kIOReturnSuccess )
    {
            /* Get the physical address corresponding the DBDMA channel area:   */

		fCCLPhysAddr = pmap_extract(	kernel_pmap,
										(vm_offset_t)fCCL );

		g.cclPhysAddr   = (UInt32)fCCLPhysAddr;  // for debugging ease
		g.cclLogAddr    = (UInt32)fCCL;
        if ( ioReturn != kIOReturnSuccess )
            PAUSE( 0, ioReturn, 'MAP-', "allocHdwAndChanMem - DBDMA mapping err.\n" );
    }

    if ( ioReturn == kIOReturnSuccess )
    {
        ELG( fCCLPhysAddr, fCCL, '=CCL', "allocHdwAndChanMem - CCL phys/logical addresses." );

			/* Set the Interrupt, Branch, and Wait DBDMA registers.		*/
			/* Caution: the following Curio interrupt register bits		*/
			/* are reverse polarity and are in a different position.	*/
			/* The pattern is: 0x00MM00VV, where MM is a mask byte		*/
			/* and VV is a value byte to match.							*/
			/*  0x80    means NO errors			(kCurioIntrError)		*/
			/*  0x40    means NO exceptions	    (kCurioIntrException)	*/
			/*  0x20    means NO command done   (kCurioIntrCmdDone)		*/
			/* Branch Select is used with BRANCH_FALSE					*/

	//	IOSetDBDMAInterruptSelect( fDBDMAAddr, 0x00000000);	/* Never let DBDMA interrupt	*/
	//	IOSetDBDMAWaitSelect(      fDBDMAAddr, 0x00200020);	/* Wait until command done		*/
	//	IOSetDBDMABranchSelect(    fDBDMAAddr, 0x00000000);	/* Never branch on error		*/
		fDBDMAAddr->interruptSelect = 0x00000000;
		fDBDMAAddr->waitSelect		= 0x20002000;
		fDBDMAAddr->branchSelect	= 0x00000000;
	    SynchronizeIO();
    }
        /* What do we do on failure? Should we try to deallocate    */
        /* the stuff we created, or will the system do this for us? */

    return  ioReturn;
}/* end allocHdwAndChanMem */


	/* doHBASelfTest - This should be extended to perform a real chip self-test.	*/

IOReturn CurioSCSIController::doHBASelfTest()
{
	IOReturn		ioReturn	= kIOReturnSuccess;
	UInt8			chipID;
	UInt8			temp;


    ELG( fCurioPhysAddr, fCurioAddr, 'Test', "doHBASelfTest" );
	resetCurio();

	WRITE_REGISTER( rCF2, CF2_EPL );
	setCmdReg( cNOP | bDMAEnBit );

	chipID = fCurioAddr[ rTCH ];

		/* Check data integrity by writing a NOP to the command register	*/
		/* and verifying that it was read back correctly.					*/

	setCmdReg( cNOP );
	temp = fCurioAddr[ rCMD ];
	if ( temp != cNOP )
	{
		PAUSE( cNOP, temp, 'hba-', "doHBASelfTest - Expected NOP" );
		ioReturn = kIOReturnNoDevice;
	}
		/* To do: write a test pattern into the FIFO and	*/
		/* check for correct count and bits.				*/

	return ioReturn;
}/* end doHBASelfTest */


void CurioSCSIController::interruptOccurred( IOInterruptEventSource *ies, int intCount )
{
//	DBDMAChannelRegisters	*DBDMARegs = (DBDMAChannelRegisters*)fDBDMAAddr;


//	ELG( ies, intCount, 'Int+', "interruptOccurred" );

	g.intLevel |=  kLevelISR;                               /* set ISR flag     */
	g.intLevel &= ~kLevelLatched;                           /* clear latched    */

//	ELG( DBDMARegs->channelStatus, DBDMARegs->commandPtrLo, 'Int+', "interruptOccurred." );
	ELG( fCmd, 0, 'Int+', "interruptOccurred." );
//  ELG( *(UInt32*)0xF3000024, *(UInt32*)0xF300002C, 'Int ', "interruptOccurred." );

	doHardwareInterrupt();						/**** HANDLE THE INTERRUPT  ****/

//  ELG( fCmd, *(UInt32*)0xF300002C, 'Intx', "interruptOccurred." );

    g.intLevel &= ~kLevelISR;                  /* clear ISR flag    */
    return;
}/* end interruptOccurred */



void CurioSCSIController::doHardwareInterrupt()
{
	if ( !interruptPending() )
	{
		ELG( 0, 0, 'Int?', "CurioSCSIController::doHardwareInterrupt - spurious interrupt" );
		return;
	}

	fCkForAnotherInt = true;	// keep going around
	do
	{
		ELG( fCkForAnotherInt<<16 | fNeedAnotherInterrupt, fBusState, 'loop', "doHardwareInterrupt" );
		fNeedAnotherInterrupt = false;

			/* Gross error is set incorrectly (according to Clinton Bauder).	*/

		if ( fSaveStatus & sGrossErr )	// ??? mlj was fSaveInterrupt
		{
			ELG( 0, fSaveInterrupt, 'GEr-', "doHardwareInterrupt - Gross Error set." );
			fSaveStatus &= ~sGrossErr;
		}

				/* iIlegalCmd - Software messed up.	*/
				/* Start over from scratch.			*/
				/* We can get this if we write too	*/
				/* many commands into the register.	*/

		if ( fSaveInterrupt & iIlegalCmd )
		{
			ELG( 0, fSaveInterrupt, 'IllC', "doHardwareInterrupt - Illegal command interrupt" );
			if ( fBusState != SCS_DEATH_MARCH )
				fsmStartErrorRecovery( kIOReturnInternalError );

			fCkForAnotherInt = false;
		}/* end IF iIlegalCmd */


			/* Check for disconnect:	*/

		if ( fSaveInterrupt & iDisconnect )
		{
			setCmdReg( cEnSelResel );
				/* Radar 1678545: this is the only (normal) place	*/
				/* that fBusBusy is cleared. (It's also cleared by	*/
				/* bus reset and driver initialization.)			*/
			fBusBusy = false;
		}

		if ( fSaveInterrupt & iResetDetect )
		{
			ELG( fSaveStatus, fSaveInterrupt, 'BusR', "doHardwareInterrupt - SCSI Bus Reset\n" );
			if ( fCmd )
			{
	            fCmd						= NULL;
	            fCmdData					= NULL;
	            fCurrentTargetLun.target	= kInvalidTarget;
	            fCurrentTargetLun.lun		= kInvalidLUN;
			}
			super::resetOccurred();
			enableCommands();			/* let superclass issue another command	*/
			fCkForAnotherInt = false;
		}/* end IF iResetDetect */
		else if ( (fSaveStatus & sParityErr) && fBusBusy )
		{
			fsmStartErrorRecovery( kIOReturnParityError );
			fCkForAnotherInt = false;
		} /* If Parity Error and not disconnected */
		else
		{		/* Only certain states are legal if the bus is busy.	*/
			if ( fBusBusy )
			{
					/* This is a legitimate interrupt (parity error and	*/
					/* bus reset have already been handled). The entity	*/
					/* that started the chip action that caused the		*/
					/* interrupt (deep breath) set fBusState to indicate*/
					/* why the interrupt happened.						*/
					/* Call the proper finite-state machine function.	*/
					/* On return, fBusState will be set to one of the	*/
					/* following values:								*/
					/*	SCS_INITIATOR			Start an action on the	*/
					/* 							current phase.			*/
					/*	SCS_DISCONNECT			The bus is free.		*/
					/*	SCS_WAIT_FOR_BUS_FREE	The bus should go free	*/
					/*							shortly.				*/
					/* Note that SCS_WAIT_FOR_BUS_FREE and				*/
					/* fsmWaitForBusFree short-circuit some of the		*/
					/* automaton to avoid unnecessary interrupt events.	*/
				switch ( fBusState )
				{
				case SCS_DISCONNECTED:		fsmDisconnected();	break;
				case SCS_SELECTING:			fsmSelecting();		break;
				case SCS_RESELECTING:		fsmReselecting();	break;
				case SCS_INITIATOR:			fsmInitiator();		break;
				case SCS_COMPLETING:		fsmCompleting();	break;
				case SCS_DMACOMPLETE:		fsmDMAComplete();	break;
				case SCS_SENDINGMSG:		fsmInitiator();		break;
				case SCS_GETTINGMSG:		fsmGettingMsg();	break;
				case SCS_SENDINGCMD:		fsmSendingCmd();	break;
				case SCS_WAIT_FOR_BUS_FREE:	fsmWaitForBusFree();break;
				case SCS_DEATH_MARCH:		fsmErrorRecoveryInterruptService(); break;

				default:
					PAUSE( fSaveStatus, fBusState, 'FSM-', "doHardwareInterrupt - Illegal bus automaton state." );
				}/* end SWITCH fBusState */
			}/* end IF bus is busy */
			else
			{		/* The bus is (or just went) free.					*/
					/* We only allow selection, reselection,			*/
					/* wait for disconnect, or death march interrupts.	*/
				switch ( fBusState )
				{
				case SCS_DISCONNECTED:		fsmDisconnected();		break;
				case SCS_SELECTING:			fsmSelecting();			break;
				case SCS_WAIT_FOR_BUS_FREE:	fsmWaitForBusFree();	break;
				case SCS_DEATH_MARCH:		fsmErrorRecoveryInterruptService(); break;

				default:
					 fsmStartErrorRecovery(	kIOReturnInternalError );
					 fCkForAnotherInt = false;
					 break;
				}/* end SWITCH on fBusState */
			}/* end ELSE bus is not busy */
		}

			/* We have (presumably) completely handled the previous		*/
			/* interrupt. At this point, there are five legitimate		*/
			/* fBusState values:										*/
			/*	SCS_RESELECTING		Just got a reselection interrupt	*/
			/*	SCS_INITIATOR		Continue operation for this target.	*/
			/*	SCS_DISCONNECTED	Enable selection/reselection		*/
			/*	SCS_WAIT_FOR_BUS_FREE	Command completion transition	*/
			/*	SCS_DEATH_MARCH		Error recovery						*/
			/*  Handle a SCSI Phase change if necessary. This will		*/
			/* leave the bus state in the "expected" state for the next	*/
			/* operation.												*/

		switch ( fBusState )
		{
		case SCS_DEATH_MARCH:
			fCkForAnotherInt = false;
			break;

		case SCS_INITIATOR:
			ELG( fCkForAnotherInt<<16 | fNeedAnotherInterrupt, 0, 'SCSi', "doHardwareInterrupt" );
			if ( fNeedAnotherInterrupt )
			{		// from tagged reselect
				fNeedAnotherInterrupt = false;
				if ( quickCheckForInterrupt() )
				{
					fsmPhaseChange();
				}
			}
			else
			{
				fsmPhaseChange();
			}
			break;

		case SCS_WAIT_FOR_BUS_FREE:
		default:
			break;
		}/* end SWITCH on fBusState */

			/* This is the final check: if we determine that the	*/
			/*  previous action will complete quickly (for example,	*/
			/* it's a message in byte), we'll spin for up to ten	*/
			/* microseconds to see if the chip is ready for another	*/
			/* operation.											*/

	} while ( fCkForAnotherInt && quickCheckForInterrupt() );

	if ( fBusState == SCS_DEATH_MARCH )
		fsmContinueErrorRecovery();

	if ( fBusState == SCS_DISCONNECTED )
		enableCommands();		/* Try to start another command	*/

	return;
}/* end doHardwareInterrupt */


	/* This is called from fsmDisconnected when we receive a reselected interrupt.	*/

void CurioSCSIController::handleReselectionInterrupt()
{
	UInt8			selectByte;
	UInt32			fifoDepth;
	UInt8			msgByte;
	IOReturn		ioReturn = kIOReturnSuccess;


		/* Make sure there's a selection byte and an Identify message in the FIFO.	*/

	fBusBusy	= ((fSaveInterrupt & iDisconnect) == 0);
	fifoDepth	= fCurioAddr[ rFOS ] & kFIFOCountMask;

	ELG( fifoDepth, fSaveInterrupt, 'fHRI', "handleReselectionInterrupt" );
	if ( fifoDepth != 2 )
	{
	    ioReturn	= kIOReturnInternalError;
		PAUSE( 0, 0, 'Rsl-', "handleReselectionInterrupt - Bad FIFO count on reselect" );
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    selectByte = fCurioAddr[ rFFO ];
			/* During reselection, the first message byte	*/
			/* must be an Identify with the LUN.			*/
	    ioReturn = getReselectionTargetID( selectByte );
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    msgByte = fCurioAddr[ rFFO ];		/* Pull the Identify byte	*/
	    if ( fSaveStatus & sParityErr )
			ioReturn	= kIOReturnParityError;
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    if ( (msgByte & kSCSIMsgIdentify) == 0 )
		{
			ioReturn = kIOReturnInternalError;
			PAUSE( 0, msgByte, ' ID-', "handleReselectionInterrupt - Bad ID Message (no Identify) on reselect" );
	    }

	    fCurrentTargetLun.lun	= msgByte & ~kSCSIMsgIdentify;
		fTag					= kInvalidTag;
	    fBusState				= SCS_RESELECTING;

			/* At this point, the chip is waiting for us to validate	*/
			/* the Identify message. While we have a Target and LUN,	*/
			/* we don't have a command to reconnect to.					*/
		setCmdReg( cMsgAcep );			/* Accept the Identify message	*/
	}

	if ( ioReturn != kIOReturnSuccess )
	    fsmStartErrorRecovery( ioReturn );

	return;
}/* end handleReselectionInterrupt */


	/* Complete the processing of a reselection interrupt.				*/
	/* We have just ACK'd the Identify message. Try to find the command	*/
	/* that corresponds to this target.lun (without a queue tag).		*/
	/* If we're successful, complete the reselection and start			*/
	/* following phases by calling fsmPhaseChange directly.				*/
	/* If the first disconnected command has a non-zero tag queue, we	*/
	/* expect to be in messsage in phase, and will receive a tagged		*/
	/* queue message eventually.										*/

void CurioSCSIController::fsmReselecting()
{
	ELG( fBusState, *(UInt16*)&fCurrentTargetLun<<16 | (UInt16)fTag, 'fRsl', "fsmReselecting" );

	reselectNexus();
	if ( fCmd )
	{		/* We have successfully reselected this target.		*/
			/* Wait until the target tells us what to do next.	*/
	    fBusState = SCS_INITIATOR;
		return;
	}

		/* We don't have a nexus yet. If we're in MSGI phase,	*/
		/* continue grabbing message bytes until we receive		*/
		/* a queue tag message (we might receive an SDTR),		*/
		/* continuing at fsmReselecting to process interrupts.	*/

	quickCheckForInterrupt();	// clear pending interrupt(s) and update phase
	fCurrentBusPhase = fSaveStatus & mPhase;
	if ( fCurrentBusPhase == kBusPhaseMSGI )
	{
		if ( (fCurioAddr[ rFOS ] & kFIFOCountMask) == 0 )
		{
			if ( fMsgInState == kMsgInInit )
			{
				setCmdReg( cNOP | bDMAEnBit );	// clear TC zero bit
				setCmdReg( cIOXfer );
				if ( !quickCheckForInterrupt() )
				{	fCkForAnotherInt = true;
					return;
				}
			}
		}
		fsmGettingMsg();
	}
	else
	{		/* We're in the wrong phase. Bail out.	*/
		PAUSE( 0, fSaveStatus, 'RMI-', "fsmReselecting - bus phase is not MsgIn" );
		fsmStartErrorRecovery( kIOReturnInternalError );
	}
	return;
}/* end fsmReselecting */



	/* Disconnected - only legal event here is reselection.				*/
	/* handleReselectionInterrupt may set fBusState to SCS_INITIATOR	*/

void CurioSCSIController::fsmDisconnected()
{
	UInt8	selectByte, target;


	ELG( 0, fSaveInterrupt, 'fDis', "fsmDisconnected" );

	if ( fSaveInterrupt & iReselected )
	{
	    handleReselectionInterrupt();
		return;
	}

		/* See if some target trying to Select us - should never happen:	*/

	if ( fSaveInterrupt & (iSelected | iSelectWAtn) )
	{
	    selectByte = fCurioAddr[ rFFO ];
	    selectByte &= ~fInitiatorIDMask;
	    for ( target = 0; target < 8; target++ )
		{
			if ( selectByte & (1 << target) )
				break;
	    }
	    PAUSE( selectByte, target, 'Tar-', "fsmDisconnected - Selected by external target - not supported." );
	    setCmdReg( cDisconnect );
		return;
	}/* end IF bogus selection by target */


	fCkForAnotherInt = false;

	if ( fBusBusy == false )
	{		/* disconnect interrupted while disconnected			*/
			/* This always happens when we finish an IO request.	*/
			/* fBusBusy is cleared when the "bus disconnected" bit	*/
			/* is set in the NCR 53C96 interrupt register.			*/
		ELG( 0, 0, 'Dis-', "fsmDisconnected - disconnect interrupted while disconnected" );
		enableCommands();
		return;
	}

			/* Since we're disconnected, just ignore the interrupt	*/

	if ( fCmd )
	{
		fCmdData->results.returnCode = kIOReturnInternalError;
		completeCommand();
	}

	fCurrentBusPhase = kBusPhaseBusFree;
	return;
}/* end fsmDisconnected */


	/* This state is called when we expect to disconnect from a		*/
	/* target after receiving a Command Complete, Disconnect, or	*/
	/* Abort message and the bus is still busy on exit from the		*/
	/* finite state automaton (because curioQuickCheckForBusFree	*/
	/* returned false).												*/

void CurioSCSIController::fsmWaitForBusFree()
{
	ELG( fBusBusy, fSaveInterrupt, 'fWBF', "fsmWaitForBusFree" );

	if ( fSaveInterrupt & (iReselected | iSelected | iSelectWAtn) )
	{
			/* We went straight from command completion to (presumably)	*/
			/* reselection. Treat it as a normal selection/reselection	*/
			/* interrupt from "disconnected" state. (This is an abuse of*/
			/* the finite-state automaton design.)						*/
	    fsmDisconnected();
	}
	else if ( fBusBusy == false )
	{
			/* This is expected: the bus has just gone free after a	*/
			/* command completed. We can now safely try to start	*/
			/* another command.										*/
	    fBusState = SCS_DISCONNECTED;
		fCkForAnotherInt = false;
		enableCommands();
	}
	else
	{		/* This is strange. It may not be an error, but I don't know	*/
			/* if there are other legal bus states after disconnect.		*/
			/* (The one counter example would be after a linked command		*/
			/* when the target goes to Command phase, but we should have	*/
			/* rejected this at command complete.)							*/
		ELG( fCmd, 0, 'WBF-', "fsmWaitForBusFree - strangeness" );
	    fsmStartErrorRecovery( kIOReturnInternalError );
	}
	return;
}/* end fsmWaitForBusFree */


	/* fsmSelecting - One of three things can happen here:		*/
	/*		the selection could succeed (though with possible	*/
	/*				incomplete message out),					*/
	/*		it could time out, or								*/
	/*		we can be reselected.								*/

void CurioSCSIController::fsmSelecting()
{
	SCSICDBInfo			scsiCDB;
	UInt8				fifoDepth;
	IOReturn			ioReturn	= kIOReturnSuccess;


	ELG( fSaveSeqStep, fSaveInterrupt, 'fSel', "fsmSelecting" );

	fBusBusy = ((fSaveInterrupt & iDisconnect) == 0 );
	if ( fBusBusy == false )
	{		/* Selection timed-out. Abort this request.	*/
		ELG( fCmd, 0, 'Tim-', "fsmSelecting - timeout" );

	    setCmdReg( cFlshFFO );
	    fBusState = SCS_DISCONNECTED;
		fCmdData->results.returnCode = kIOReturnNoDevice;
        completeCommand();
		return;
	}

	fCmd->getCDB( &scsiCDB );

	if ( fSaveInterrupt == (iFuncComp | iBusService) )
	{
	    switch ( fSaveSeqStep & INS_STATE_MASK )
		{
	    case 0:	/* No message phase. If we really wanted one,			*/
				/* this could be significant.							*/
				/* OK, let's try to continue following phase changes.	*/
			fBusState = SCS_INITIATOR;
			break;

	    case 3: /* didn't complete cmd phase, parity?	*/
	    case 4: /* everything worked					*/
	    case 1: /* everything worked, SCMD_SELECT_ATN_STOP case 				*/
				/* We're connected. Start following the target's phase changes.	*/
				/* If we're trying to do sync negotiation,						*/
				/* this is the place to do it. In that case, we					*/
				/* sent a SCMD_SELECT_ATN_STOP command, and						*/
				/* ATN is now asserted (and we're hopefully in					*/
				/* msgOut phase). We want to send 5 bytes.						*/
				/* Drop them into currMsgOut									*/
			fBusState = SCS_INITIATOR;
			break;

	    case 2:		/* Either no command phase, or incomplete message transfer.	*/
			fifoDepth = fCurioAddr[ rFOS ] & kFIFOCountMask;
				/* Spec says ATN is asserted if all message bytes were not sent.	*/
			if ( fifoDepth > scsiCDB.cdbLength )
				setCmdReg( cRstAtn );

				/* 970611: clear the FIFO. If we don't do this and the	*/
				/* target is entering MSGI phase (trying to send us a	*/
				/* Message Reject), it will sit on top of the FIFO, and	*/
				/* we'll read garbage from the FIFO.					*/

			setCmdReg( cFlshFFO );	/* The command is still in the FIFO */
				/* OK, let's try to continue following phase changes.	*/
			fBusState = SCS_INITIATOR;
			break;

	    default:	/* fBusBusy = true;	    -- Radar 1678545 */
			ioReturn = kIOReturnInternalError;
			break;
	    }/* end SWITCH */
	}
	else if ( fSaveInterrupt & iReselected )
	{
			/* We were reselected while trying to do a selection.		*/
			/* Enqueue this cmdBuf on the HEAD of pendingQ, then deal	*/
			/* with the reselect.										*/
			/* Tricky case, we have to "deactivate" this command		*/
			/* since this hardwareStart attempt failed.					*/

		ELG( fCmd, fSaveInterrupt, 'Arb-', "fsmSelecting - Reselection interrupt while selecting" );
		disableCommands();
		rescheduleCommand( fCmd );
		fCmd = NULL;

	    fBusState = SCS_DISCONNECTED;
	    handleReselectionInterrupt();		/* Go deal with reselect.	*/
	}
	else
	{
		ioReturn = kIOReturnInternalError;
		PAUSE( 0, 0, 'int-', "Bogus select/reselect interrupt" );
	}

	if ( ioReturn != kIOReturnSuccess )
	    fsmStartErrorRecovery( ioReturn );

	return;
}/* end fsmSelecting */


	/* This is a dummy interrupt service state that we enter after selection	*/
	/* when the previous Curio operation has no interrupt-service completion	*/
	/* requirement. For example, we can get here after a Message Out or			*/
	/* reselection. Nothing happens here; we continue at fsmPhaseChange.		*/

void CurioSCSIController::fsmInitiator()
{
	ELG( 0, fSaveInterrupt, 'fIni', "fsmInitiator" );
	fBusState = SCS_INITIATOR;
	return;
}/* end fsmInitiator */


	/* We just did a SCMD_INIT_CMD_CMPLT command, hopefully all that's left		*/
	/* is to drop ACK. Command Complete message is handled in fscAcceptinfMsg.	*/
	/* We can't go to SCS_DISCONNECTED until the target disconnects. If we		*/
	/* go to "disconnected" state too soon, we'll encounter a load-dependent	*/
	/* race condition that causes us to start another command before we've		*/
	/* cleaned up from the last command. The actual state change is in			*/
	/* fsmProcessMessage.														*/

void CurioSCSIController::fsmCompleting()
{
	unsigned		fifoDepth;
	IOReturn		ioReturn	= kIOReturnSuccess;
	UInt8			statusByte;


	ELG( 0, fSaveInterrupt, 'fCmp', "fsmCompleting" );

	if ( fSaveInterrupt & iDisconnect )
	{
	    ioReturn = kIOReturnInternalError;
		PAUSE( 0, 0, ' BF-', "fsmCompleting - Bus free before target completion complete" );
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    fifoDepth = fCurioAddr[ rFOS ] & kFIFOCountMask;
	    if ( fSaveInterrupt & iFuncComp )
		{		/* Got both Status and MsgIn in FIFO; ACK is still asserted.	*/
			if ( fifoDepth != 2 )
			{		/* This is pretty bogus - we expect a status and msg in the FIFO.	*/
				PAUSE( 0, fifoDepth, 'ffc-', "fsmCompleting - FIFO problem with Status/MsgIn" );
				ioReturn = kIOReturnInternalError;
			}

			if ( ioReturn == kIOReturnSuccess )
			{
				statusByte		= fCurioAddr[ rFFO ];
				fMsgInBuffer[0]	= fCurioAddr[ rFFO ];
				fCmdData->results.scsiStatus = statusByte;
				fMsgInIndex	    = 1;
				ioReturn	    = fsmProcessMessage();
			}
	    }
	    else
		{		/* We only received a status byte.								*/
				/* This can occur if we responded to the interrupt before		*/
				/* the device successfully transmitted the Command Complete		*/
				/* message. This is kind of weird, but let's try to handle it.	*/
			if ( fifoDepth != 1 )
			{
				ioReturn = kIOReturnInternalError;
				PAUSE( 0, fifoDepth, 'Cmp-', "IO complete: incorrect FIFO count (expecting one byte)" );
			}

				/* Back to watching phase changes.						*/
				/* Presumably, the target will switch to MSGI phase and	*/
				/* complete the command at its leisure.					*/
			fBusState = SCS_INITIATOR;
	    }
	}

	if ( ioReturn != kIOReturnSuccess )
	    fsmStartErrorRecovery( ioReturn );

	return;
}/* end fsmCompleting */


	/* DMA Complete.	*/

void CurioSCSIController::fsmDMAComplete()
{
	UInt32		fifoResidualCount, actualTransferCount, dmaResidualCount;
	UInt8		residualByte;


		/* Stop the DMA engine and retrieve the total number of		*/
		/* bytes transferred. This will be							*/
		/* fCmd->thisTransferLength	The number of bytes requested	*/
		/*	- the current DMA residual count						*/
		/*	- the current FIFO residual count						*/
		/* Note that there may still be bytes in the FIFO.			*/

//	IODBDMAReset( fDBDMAAddr );
	fDBDMAAddr->channelControl = SWAP( 0x20002000 );		// set FLUSH bit
		SynchronizeIO();
	fDBDMAAddr->channelControl = SWAP( 0x80000000 );		// clr RUN   bit
		SynchronizeIO();

		/* Get residual count from DMA transfer.	*/

	dmaResidualCount	= fCurioAddr[ rXCM ] << 8 | fCurioAddr[ rXCL ];
	actualTransferCount	= fThisTransferLength - dmaResidualCount;

	ELG( dmaResidualCount, actualTransferCount, 'fDMA', "fsmDMAComplete" );

		/* Now, see if there are any bytes lurking in the FIFO:	*/

	fifoResidualCount = fCurioAddr[ rFOS ] & kFIFOCountMask;

	if ( fifoResidualCount > 0 )
	{		/* Drain the FIFO after DMA for a normal transfer:	*/
		ELG( 0, fifoResidualCount, 'MTFF', "fsmDMAComplete - empty the FIFO" );

	    if ( !fCmdData->isWrite )
		{
			while ( fifoResidualCount > 0 )
			{		/* Try to move residual data from the FIFO into	*/
					/* the user's buffer.							*/
				residualByte = fCurioAddr[ rFFO ];
				fCmdData->mdp->writeBytes(	fCmdData->results.bytesTransferred
												+ actualTransferCount,
											&residualByte, 1 );
				++actualTransferCount;
			}/* end WHILE bytes in FIFO */
	    }
	    else
		{		/* Write command - just flush the data */
			setCmdReg( cFlshFFO );
			setCmdReg( cNOP | bDMAEnBit );	// clear TC zero bit;
			actualTransferCount	-= fifoResidualCount;
	    }/* end IF Write */
	}/* end IF bytes left in FIFO */

	fCmdData->results.bytesTransferred += actualTransferCount;

	if ( fSaveStatus & sParityErr )
	{
		PAUSE( 0, 0, 'par-', "fsmDMAComplete - parity error" );
	    fsmStartErrorRecovery( kIOReturnParityError );
	}
	else
	{
	    fBusState = SCS_INITIATOR;		/* Continue following phase changes.	*/
	}

	return;
}/* end fsmDMAComplete */


	/* Just completed the SCMD_TRANSFER_INFO operation for MessageIn.	*/
	/* ACK is not asserted (we have not ACK'ed this byte).				*/
	/* There is no parity error.										*/
	/* We will not have a command if we're reselecting.					*/

void CurioSCSIController::fsmGettingMsg()
{
	UInt32		    fifoCount;
	UInt8		    msgInByte;
	IOReturn	    ioReturn = kIOReturnSuccess;


	ELG( fBusBusy, fMsgInState<<16 | fCurioAddr[ rFOS ], 'fGtM', "fsmGettingMsg" );

	if ( fBusBusy == false )
	{
	    	/* This (non-fatal) error is handled on return...	*/
	    ioReturn = kIOReturnInternalError;	/* Any non-zero non-fatal error status	*/
	}
	else
	{
		fifoCount = fCurioAddr[ rFOS ] & kFIFOCountMask;
	    if ( fifoCount != 1 )
		{
			PAUSE( 0, fifoCount, 'GtM-', "fsmGettingMsg - MsgIn FIFO count error" );
			ioReturn = kIOReturnInternalError;
	    }
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    msgInByte = fCurioAddr[ rFFO ];
		ELG( 0, msgInByte, '=Msg', "fsmGettingMsg - got a byte" );
	    if ( fSaveStatus & sParityErr )
		{
			PAUSE( 0, fSaveStatus, 'Par-', "fsmGettingMsg - Parity error getting MsgIn" );
			ioReturn = kIOReturnParityError;
	    }
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    if ( fMsgInState == kMsgInInit )
		{
			fMsgInCount = 0;
			fMsgInIndex = 0;
	    }
	    if ( fMsgInIndex >= kMessageInBufferLength )
		{
			PAUSE( 0, fMsgInIndex, 'Par-', "fsmGettingMsg -  too many bytes" );
			ioReturn = kIOReturnInternalError;
	    }
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    fMsgInBuffer[ fMsgInIndex++ ] = msgInByte;
	    switch ( fMsgInState )
		{
		case kMsgInInit:				/* This is the first message byte.		*/
			if ( (msgInByte == kSCSIMsgCmdComplete)
			  || (msgInByte >= (UInt8)kSCSIMsgIdentify) )
			{		 /* This is 1-byte cmdComplete or Identify message.				*/
				fMsgInState = kMsgInReady;
			}
			else if ( msgInByte == kSCSIMsgExtended )
            {       /* This is an extended message. The next byte has the count.	*/
                fMsgInState = kMsgInCounting;
            }
			else if ( msgInByte <= kSCSIMsgOneByteMax )
			{		/* These are other 1-byte messages.	*/
				fMsgInState = kMsgInReady;
			}
            else if ( msgInByte >= kSCSIMsgTwoByteMin
                  &&  msgInByte <= kSCSIMsgTwoByteMax )
            {		/* This is a two-byte message.					*/
                    /* Set the count and read the next byte.		*/
                fMsgInState = kMsgInReading;	/* Need one more	*/
                fMsgInCount = 1;
            }
            else
            {       /* This is an unknown message. */
                fMsgInState = kMsgInReady;
            }
            break;

		case kMsgInCounting:        /* Count byte of multi-byte message:   */
            fMsgInCount = msgInByte;
            fMsgInState = kMsgInReading;
            break;

		case kMsgInReading:                 /* Body of multi-byte message:  */
			if ( --fMsgInCount <= 0 )
				fMsgInState = kMsgInReady;
			break;

		default:
			PAUSE( 0, 0, 'Msg-', "DoMessageInPhase  - Bogus MSGI state!\n" );
			ioReturn = kIOReturnInternalError;
	    }/* SWITCH on message state */
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    switch ( fMsgInState )
		{
	    case kMsgInReading:
	    case kMsgInCounting:	/* We have more message bytes to read.			*/
								/* Accept this byte (this sets ACK) and setup	*/
								/* to transfer the next byte.					*/
			setCmdReg( cMsgAcep );
				/* Since the message accept command sets "interrupt",	*/
				/* eat it here so we don't get a second interrupt.		*/
			quickCheckForInterrupt();
			setCmdReg( cNOP );
			setCmdReg( cIOXfer );
			if ( fBusState != SCS_RESELECTING )
				 fBusState  = SCS_GETTINGMSG;
				/* This would be a good place to spin-wait for completion,	*/
				/* continuing at the start of this method if there is		*/
				/* another byte (and we're still in message in phase).		*/
				/* Perhaps this method should return a "spinwait" status to	*/
				/* the mainline FSM.										*/
			break;

	    case kMsgInReady:
			fMsgInState	= kMsgInInit;
			ioReturn	= fsmProcessMessage();
			break;

	    case kMsgInInit:
	    default:
				/* that's strange: we should never be in idle state	*/
				/* *after* successfully reading a message byte.		*/
			ioReturn = kIOReturnInternalError;
			break;
	    }/* end SWITCH */
	}

	if ( ioReturn != kIOReturnSuccess )
	    fsmStartErrorRecovery( ioReturn );

	return;
}/* end fsmGettingMsg */


	/* Just read a message; ACK is false and the message			*/
	/* has been read into fMsgInBuffer[0..fMsgInIndex]. We have not	*/
	/* ACK'ed the last byte yet.									*/
	/* If we fail here, the caller will start error recovery.		*/

IOReturn CurioSCSIController::fsmProcessMessage()
{
	IOReturn	    ioReturn			= kIOReturnSuccess;
	bool		    messageAckNeeded	= true;


    ELG( fBusState, *(UInt32*)fMsgInBuffer, 'fMgI', "fsmProcessMessage" );

		/* Message-in complete. Handle message(s) in currMsgIn:	*/

	if ( fBusState == SCS_RESELECTING )
	{
	    	/* The only interesting message here is queue tag.	*/
			/* (What about target SDTR or Abort?)				*/

	    switch ( fMsgInBuffer[0] )
		{
	    case kSCSIMsgHeadOfQueueTag:
	    case kSCSIMsgOrderedQueueTag:
	    case kSCSIMsgSimpleQueueTag:
			if ( fMsgInIndex != 2 )
			{
				PAUSE( fMsgInBuffer[0], fMsgInIndex, 'Que-', "fsmProcessMessage - queue tag without tag value" );
				ioReturn = kIOReturnInternalError;
			}
			else
			{	fTag = fMsgInBuffer[ 1 ];
				ELG( 0, fTag, '=Tag', "fsmProcessMessage - tag" );
				reselectNexus();
				if ( fCmd )
				{
					setCmdReg( cMsgAcep );
					fNeedAnotherInterrupt   = true;
					fBusState				= SCS_INITIATOR;
					return kIOReturnSuccess;
				}
			}
			break;

	    default:
			ioReturn = kIOReturnInternalError;
			break;
	    }/* end SWITCH on message */

			/* Since we process commands one by one, whack this			*/
			/* command so we fall through the regular message handler.	*/
	    fMsgInBuffer[0] = kSCSIMsgNop;
	    fMsgInIndex = 1;
	}/* end IF reselecting */

	fBusState		    	= SCS_INITIATOR;
	fNeedAnotherInterrupt   = true;

	switch ( fMsgInBuffer[0] )
	{
	case kSCSIMsgNop:
	    break;

	case kSCSIMsgCmdComplete:
			/* Normally, we get here from fsmCommandComplete.			*/
			/* All we need to do is to Ack the message and complete the	*/
			/* command. Exit in a transitional bus state that becomes	*/
			/* SCS_DISCONNECTED when the bus is no longer busy.			*/
	    fBusState = SCS_WAIT_FOR_BUS_FREE;
		completeCommand();
	    break;

	case kSCSIMsgDisconnect:
		fCmd = NULL;
			/* Exit the interrupt service routine so	*/
			/* we don't miss a reselection interrupt.	*/
	    fBusState = SCS_WAIT_FOR_BUS_FREE;
	    fCkForAnotherInt = false;
	    break;

	case kSCSIMsgSaveDataPointers:
        if ( fCmd )
			fCmdData->savedDataPosition = fCmdData->results.bytesTransferred;
	    break;

	case kSCSIMsgRestorePointers:
        if ( fCmd )
			fCmdData->results.bytesTransferred = fCmdData->savedDataPosition;
	    break;

	case kSCSIMsgRejectMsg:
		ioReturn = kIOReturnInternalError;

	//	don't break - fall through to kSCSIMsgAbort	*/

	case kSCSIMsgAbort:
	case kSCSIMsgAbortTag:
		if ( fCmd )
		{		/* Oops: something is terribly wrong with this command.	*/
				/* This can happen if we get a parity error, set ATN,	*/
				/* and send an inititor detected error to the target.	*/
			fCmdData->results.returnCode = kIOReturnInternalError;
			completeCommand();
			fCkForAnotherInt = false;
	    }
		    /* After receiving an Abort, the target will go free	*/
		fBusState = SCS_WAIT_FOR_BUS_FREE;
	    break;

	case kSCSIMsgLinkedCmdComplete:
	case kSCSIMsgLinkedCmdCompleteFlag:
			/* These are impossible: we reject commands with the link bit set.	*/
			/* About all we can do is to fail the client's command and			*/
			/* stagger onwards.													*/
		PAUSE( fCmd, 0, 'Abo-', "fsmProcessMessage - recv'd Abort" );	
		fCmdData->results.returnCode = kIOReturnInternalError;
		completeCommand();
	    fBusState = SCS_WAIT_FOR_BUS_FREE;
	    break;

	case kSCSIMsgExtended:
	default:	/* The only expected extended messages are Wide	and			*/
				/* Synchronous Target Initiated Negotiations which			*/
				/* aren't supported. All other messages are unacceptable.	*/
		ELG( *(UInt32*)&fMsgInBuffer[0], *(UInt32*)&fMsgInBuffer[4], 'TIN?',
				"fsmProcessMessage - Target Initiated Negotiation?" );	
			/* We don't support SDTR or WDTR, so send a Message Reject.	*/
			/* Curio doesn't really support Synchronous and definitely	*/
			/* doesn't support Wide.									*/
		fpMsgOut = fpMsgPut = &fMsgOutBuffer[ 0 ];
		*fpMsgOut++ = kSCSIMsgRejectMsg;	/* Stuff the Reject Message.*/

		setCmdReg( cSetAtn );				/* Set Atn					*/
		IODelay( 30 );
		messageAckNeeded = true;
		break;
	}/* end SWITCH on Message byte */

	if ( messageAckNeeded )
	    setCmdReg( cMsgAcep );

	return ioReturn;
}/* end fsmProcessMessage */


	/* Just completed the SCMD_TRANSFER_INFO operation for command.	*/
void CurioSCSIController::fsmSendingCmd()
{
	fBusState = SCS_INITIATOR;
	return;
}/* end fsmSendingCmd */


	/* Follow SCSI Phase change. Called while SCS_INITIATOR. */

void CurioSCSIController::fsmPhaseChange()
{
	int		    i;


	ELG( 0, fSaveStatus, 'fPhC', "fsmPhaseChange" );

	fCurrentBusPhase = fSaveStatus & mPhase;

	switch ( fCurrentBusPhase )
	{
	case kBusPhaseCMD:
	    	/* The normal case here is after a host-initiated SDTR sequence.	*/
	    setCmdReg( cFlshFFO );
	    putCDBIntoFIFO();
	    setCmdReg( cIOXfer );			/* Start non-DMA xfer	*/
	    fMsgInState = kMsgInInit;
	    fBusState = SCS_SENDINGCMD;
	    break;

	case kBusPhaseDATI:				/* From target to Initiator (read)	*/
	case kBusPhaseDATO:				/* To Target from Initiator (write) */
	    fMsgInState = kMsgInInit;
	    fsmStartDataPhase( fCurrentBusPhase == kBusPhaseDATI );
	    fCkForAnotherInt = false;
	    break;

	case kBusPhaseSTS:			/* Status from Target to Initiator	*/
	    	/* fsmCompleting will collect the STATUS byte (and		*/
			/* hopefully a MSG) from the FIFO when this completes.	*/
	    fMsgInState = kMsgInInit;
	    fBusState = SCS_COMPLETING;
	    setCmdReg( cFlshFFO );
	    setCmdReg( cCmdComp );
	    break;

	case kBusPhaseMSGI:		/* Message from Target to Initiator */
		if ( fBusState != SCS_RESELECTING )
	    	 fBusState  = SCS_GETTINGMSG;
	 	setCmdReg( cNOP );
		setCmdReg( cIOXfer );
	    break;

	case kBusPhaseMSGO:		/* Message from Initiator to Target */
	    fMsgInState = kMsgInInit;
	    setCmdReg( cFlshFFO );

	    if ( fpMsgPut == fpMsgOut )
			*fpMsgOut++ = kSCSIMsgNop;	/* no message waiting to be sent.	*/


	    for ( i = 0; (i < 16) && (fpMsgPut < fpMsgOut); i++ )
		{	WRITE_REGISTER( rFFO , *fpMsgPut );
			fpMsgPut++;
	    }

	    if ( fpMsgPut == fpMsgOut )
			 fpMsgPut =  fpMsgOut = fMsgOutBuffer;

	    fBusState = SCS_SENDINGMSG;
	    	/* ATN is automatically cleared when transfer info completes.	*/
	    setCmdReg( cIOXfer );			/* Start non-DMA xfer	*/
	    break;

	default:
	    fsmStartErrorRecovery( kIOReturnInternalError );
	    break;
	}/* end SWITCH on current bus phase */

	return;
}/* end fsmPhaseChange */


void CurioSCSIController::fsmStartDataPhase( bool isDataInPhase )
{
	IOReturn	    ioReturn	= kIOReturnSuccess;


	ELG( fCmd, fThisTransferLength, 'fDat', "fsmStartDataPhase" );

		/* DataIn phase is legal if this is a read command:	*/

	if ( isDataInPhase != !fCmdData->isWrite )
	{
	    ioReturn = kIOReturnInternalError;
		PAUSE( fCmd, isDataInPhase, 'Dir-', "fsmStartDataPhase - data direction" );
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    ioReturn = generateCCL();
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	    if ( fThisTransferLength == 0 )
		{
			ioReturn = kIOReturnInternalError;
			PAUSE( fCmd, 0, 'Dat0', "fsmStartDataPhase - no data" );
	    }
	}

	if ( ioReturn == kIOReturnSuccess )
	{
	//	IOSetDBDMACommandPtr(		fDBDMAAddr, fCCLPhysAddr );
		fDBDMAAddr->commandPtrLo = SWAP( fCCLPhysAddr );
		SynchronizeIO();

	//	IOSetDBDMAChannelControl(	fDBDMAAddr, kdbdmaRun * 0x1001 );
	//	fDBDMAAddr->channelControl = SWAP( kdbdmaRun * 0x1001 );
		fDBDMAAddr->channelControl = SWAP( 0x80008000 );	// set RUN bit
		SynchronizeIO();

		WRITE_REGISTER( rXCM, fThisTransferLength >> 8   );
		WRITE_REGISTER( rXCL, fThisTransferLength & 0xFF );
		setCmdReg( cDMAXfer );

	    fBusState = SCS_DMACOMPLETE;
	}

	if ( ioReturn != kIOReturnSuccess )
	{
		PAUSE( 0, 0, 'Dat-', "fsmStartDataPhase - data phase error" );
		fsmStartErrorRecovery( ioReturn );
	}
	return;
}/* end fsmStartDataPhase */


	/* Start error recovery by doing something reasonable for the				*/
	/* current bus phase. This is called when we enter error recovery			*/
	/* (SCS_DEATH_MARCH) from an interrupt or other bus phase action handler.	*/

void CurioSCSIController::fsmStartErrorRecovery( IOReturn status )
{

	ELG( fCmd, 0, 'fSER', "fsmStartErrorRecovery" );
	killActiveCommand( status );
		/* Clean out the SCSI and DBDMA hardware	*/
	setCmdReg( cFlshFFO );
	setCmdReg( cNOP | bDMAEnBit );	// clear TC zero bit
//	IODBDMAReset( fDBDMAAddr );
	fDBDMAAddr->channelControl = SWAP( 0x20002000 );		// set FLUSH bit
		SynchronizeIO();
	fDBDMAAddr->channelControl = SWAP( 0x80000000 );		// clr RUN   bit
		SynchronizeIO();

	if ( fBusBusy == false )
		 fBusState = SCS_DISCONNECTED;
	else fBusState = SCS_DEATH_MARCH;/* We will continue at fsmContinueErrorRecovery	*/

	fCkForAnotherInt = false;
	return;
}/* end fsmStartErrorRecovery */


	/* Manage error recovery by responding to a target interrupt while in		*/
	/* an error recovery state. On exit, the bus state will be as follows:		*/
	/*	SCS_DEATH_MARCH	Still in error recovery. Call fsmContinueErrorRecovery	*/
	/*			to continue processing target requests.							*/
	/*	SCS_DISCONNECTED	The target is finally off the bus. We can restart	*/
	/*			normal operation.												*/
	/* Each bus phase requires a different action:								*/
	/*	Data Out	Clean up after DMA.											*/
	/*	Data In	Clean up after DMA.												*/
	/*	Command	Flush the FIFO													*/
	/*	Status	Drain the FIFO, if we get two bytes, just ACK the message		*/
	/*	Message Out Do nothing													*/
	/*	Message In	Read the message byte, ACK it.								*/

void CurioSCSIController::fsmErrorRecoveryInterruptService()
{
	setCmdReg( cFlshFFO );

	switch ( fCurrentBusPhase )
	{
	case kBusPhaseDATO:
	case kBusPhaseDATI:
	//	IODBDMAReset( fDBDMAAddr );
		fDBDMAAddr->channelControl = SWAP( 0x20002000 );		// set FLUSH bit
			SynchronizeIO();
		fDBDMAAddr->channelControl = SWAP( 0x80000000 );		// clr RUN   bit
			SynchronizeIO();
	    setCmdReg( cNOP | bDMAEnBit );	// clear TC zero bit
	    break;

	case kBusPhaseCMD:
	    break;

	case kBusPhaseSTS:
	    if ( fSaveInterrupt & iFuncComp )
		{
			/* We got both bytes: ACK the command complete	*/
			setCmdReg( cMsgAcep );
	    }
	    break;

	case kBusPhaseMSGO:
	    break;

	case kBusPhaseMSGI:
	    setCmdReg( cMsgAcep );
	    break;

	case kBusPhaseBusFree:	/* Selecting */
	    break;

	default:
	    killActiveCommand( kIOReturnInternalError );
		resetBus();
	    break;
	}
	return;
}/* end fsmErrorRecoveryInterruptService */


	/* Continue to manage error recovery. We are here because the target	*/
	/* is still on the bus and doing strange bus phase things. Follow the	*/
	/* target bus phase (one phase at a time) until the target disconnects.	*/
	/* We just run bit-bucket commands until the target gives up.			*/
	/* (Note that this means that we might send a Command Complete message	*/
	/* to the target. We'll look for this case and send an Abort or			*/
	/* Abort Tag instead.)													*/

void CurioSCSIController::fsmContinueErrorRecovery()
{
	UInt8		msgByte;


	quickCheckForInterrupt();


	if ( fBusBusy )
	{
		interruptPending();
	    if ( fSaveInterrupt & iDisconnect )
			fBusBusy = false;
	}

	if ( fBusBusy == false )
	{
	    fBusState = SCS_DISCONNECTED;
	}
	else
	{
	    fCurrentBusPhase = fSaveStatus & mPhase;

	    switch ( fCurrentBusPhase )
		{
	    case kBusPhaseMSGO:
			msgByte = kSCSIMsgAbort;
			WRITE_REGISTER( rFFO, msgByte );
			setCmdReg( cIOXfer );			/* Start non-DMA xfer	*/
			break;

	    case kBusPhaseDATO:
	    case kBusPhaseCMD:
			WRITE_REGISTER( rXCM, 0xFF );
			WRITE_REGISTER( rXCL, 0xFF );
			setCmdReg( cDMAXferPad );	/* DMA out pad	*/
			break;

	    case kBusPhaseMSGI:
	    case kBusPhaseDATI:
	    case kBusPhaseSTS:
			WRITE_REGISTER( rXCM, 0xFF );
			WRITE_REGISTER( rXCL, 0xFF );
			setCmdReg( cXferPad );	/* non-DMA out pad	*/
			break;

	    default:
			killActiveCommand( kIOReturnInternalError );
			resetBus();
			break;
	    }
	}/* end SWITCH on current phase */

	return;
}/* end fsmContinueErrorRecovery */



IOReturn CurioSCSIController::getReselectionTargetID( UInt8 selectByte )
{
	register UInt8		targetBits = selectByte;
	register UInt8		targetID	= 0;
	register UInt8		bitValue;	/* Supress warning  */


	if ( (targetBits & fInitiatorIDMask) == 0 )
	{
	    PAUSE( 0, selectByte, 'IId-', "getReselectionTargetID -  Reselection failed: initiator ID bit not set" );
		return kIOReturnInternalError;
	}

	targetBits	= selectByte;
	targetBits &= ~fInitiatorIDMask;		/* Remove our bit	    */
	if ( targetBits == 0 )
	{
	    PAUSE( 0, selectByte, 'TId-', "getReselectionTargetID -  Reselection failed: Target ID bit not set" );
		return kIOReturnInternalError;
	}

	bitValue = targetBits;

	if ( bitValue > 0x0F ) { targetID += 4;	bitValue >>= 4; }
	if ( bitValue > 0x03 ) { targetID += 2; bitValue >>= 2; }
	if ( bitValue > 0x01 )	 targetID += 1;

	targetBits  &= ~(1 << targetID);	/* Remove the target mask	*/
	if ( targetBits != 0 )
	{
	    PAUSE( 0, selectByte, 'TTT-', "getReselectionTargetID -  Reselection failed: multiple targets" );
		return kIOReturnInternalError;
	}

	ELG( 0, targetID, '=Tar', "getReselectionTargetID" );

	fCurrentTargetLun.target = targetID;	/* Save the current target	*/
	return kIOReturnSuccess;
}/* end getReselectionTargetID */


void CurioSCSIController::reselectNexus()
{
	fCmd = findCommandWithNexus( fCurrentTargetLun, fTag );
	if ( fCmd )
	{	fCmdData = (PrivCmdData*)fCmd->getCommandData();
		ELG( fCmd, *(UInt16*)&fCurrentTargetLun<<16 | (UInt16)fTag, '=Nex', "reselectNexus" );
	}
	else
	{	if ( fTag != kInvalidTag )
			ELG( 0, *(UInt16*)&fCurrentTargetLun<<16 | (UInt16)fTag, 'Nex-', "reselectNexus" );
	}

    return;
}/* end reselectNexus */


    /* Send a command to the Curio chip. This may cause an interrupt.    */

void CurioSCSIController::setCmdReg( UInt8 command )
{
    ELG( 0, command, '=Cmd', "setCmdReg" );

    fCurioAddr[ rCMD ] = command;	/***** DO IT    *****/
    SynchronizeIO();

    return;
}/* end setCmdReg */



	/* This spin-loop looks for another chip operation - it prevents	*/
	/* exiting the interrupt service routine if the chip presents		*/
	/* another operation within ten microseconds. Note that this		*/
	/* must be called from an IOThread, not from a "real" primary		*/
	/* interrupt service routine.										*/
	/*	@return	true if an interrupt is pending.						*/

#define OVERTIME( now, end )					\
 	(   ((now)->tv_sec  > (end)->tv_sec)		\
	|| (((now)->tv_sec == (end)->tv_sec) && ((now)->tv_nsec > (end)->tv_nsec)) )


bool CurioSCSIController::quickCheckForInterrupt()
{
	mach_timespec_t		thisTime, endTime;


	IOGetTime( &endTime );					// calc the omega
	endTime.tv_nsec	+= 10000;				// 10 mikes from now
	if ( endTime.tv_nsec >= NSEC_PER_SEC )
	{
	    endTime.tv_sec	+= 1;
	    endTime.tv_nsec	-= NSEC_PER_SEC;
	}
		/* whiz til interrupt or time's up:	*/
	do
	{
	    if ( interruptPending() )
			return true;

	    IOGetTime( &thisTime );
	} while ( OVERTIME( &thisTime, &endTime ) == false );

	return false;
}/* end quickCheckForInterrupt */


	/* Determine if SCSI interrupt is pending. If so, store the current	*/
	/* chip state. Note that reading the interrupt register clears the	*/
	/* interrupt source, so this should be done once for each interrupt.*/
	/*	@return	true if an interrupt is pending.						*/

bool CurioSCSIController::interruptPending()
{
	bool	result = false;


		/* mlj - use WHILE to check for double interrupts	*/
		/* and get get most recent conditions.				*/

	while ( (fSaveStatus = fCurioAddr[ rSTA ]) & sIntPend )	// ??? volatile
	{
	    result			= true;
	    fSaveSeqStep	= fCurioAddr[ rSQS ];
	    fSaveInterrupt	= fCurioAddr[ rINT ];	/* this clears rSTA and rSQS	*/
		ELG( fSaveSeqStep, fSaveStatus<<16 | fSaveInterrupt, 'Regs', "interruptPending" );
	}
	return result;
}/* end interruptPending */



void CurioSCSIController::putCDBIntoFIFO()
{
	SCSICDBInfo		scsiCDB;
	UInt32			i;


	fCmd->getCDB( &scsiCDB );
	for ( i = 0; i < scsiCDB.cdbLength; i++ )
		WRITE_REGISTER( rFFO, scsiCDB.cdb[ i ] );

	return;
}/* end putCDBIntoFIFO */


	/* resetCurio - Reset the chip to hardware power on state.	*/

void CurioSCSIController::resetCurio()
{
	ELG( 0, 0, 'ResC', "resetCurio" );
	setCmdReg( cRstSChp );		/* Reset chip		*/
	IODelay( 50 );				/* Chip settle		*/
	setCmdReg( cNOP );			/* Re-enable chip	*/
	setCmdReg( cFlshFFO );		/* In a clean state	*/
	return;
}/* end resetCurio */


	/* resetHardware - Reusable hardware initializer function.	*/
	/* if busReset is true, this includes a SCSI bus reset.		*/

IOReturn CurioSCSIController::resetHardware( bool busReset )
{
	IOReturn		ioReturn = kIOReturnSuccess;
	UInt8			configValue;
	UInt8			clockConversionFactor;
	UInt8			defaultSelectionTimeout;
	UInt8			temp;


		/* Temp for debugging	*/

	fSCSIClockRate = kChipDefaultBusClockMHz;

	ELG( fSCSIClockRate, busReset, 'ResH', "resetHardware" );

		/* First of all, reset interrupts, the SCSI chip, and the DMA engine.	*/

//	resetCurio();							/* Clear out the chip	    */

	temp = fCurioAddr[ rINT ];				/* Clear pending interrupt  */

//	IODBDMAReset( fDBDMAAddr );				/* Stop the DMA engine	    */
	fDBDMAAddr->channelControl = SWAP( 0x20002000 );		// set FLUSH bit
		SynchronizeIO();
	fDBDMAAddr->channelControl = SWAP( 0x80000000 );		// clr RUN   bit
		SynchronizeIO();

		/* Initialize the chip:	*/

	WRITE_REGISTER( rCF2, CF2_EPL );		/* Enable Phase Latching	*/
	setCmdReg(cNOP | bDMAEnBit);			/* DMA Nop					*/

		/* Init state variables:	*/

	fCkForAnotherInt	= false;
	fBusState			= SCS_DISCONNECTED;
	fBusBusy			= false;
	fCurrentBusPhase	= kBusPhaseBusFree;

		/* Smash all active command state (just in case).	*/

	fCmd		= NULL;
	fMsgInState	= kMsgInInit;
	fpMsgOut	= fpMsgPut = fMsgOutBuffer;

    fCurrentTargetLun.target	= kInvalidTarget;
	fCurrentTargetLun.lun		= kInvalidLUN;

		/* Configuration Register 1									*/
		/* Disable interrupt on initiator-instantiated bus reset	*/
		/* (is this correct?)										*/
		/* Enable parity.											*/
		/* Set default bus ID (7) (This should be overriden by the	*/
		/* Inspector)												*/

	fInitiatorID = kDefaultInitiatorID;
	configValue = CF1_SRD | CF1_ENABPAR | fInitiatorID;

#define gOptionExtendTiming	0
	if ( gOptionExtendTiming )
	{		/* Per instance table. This slows down transfers on the bus.	*/
	    configValue |= CF1_SLOW;
	}

	WRITE_REGISTER( rCF1, configValue );

		/* Clock factor and select timeout.	*/

		/* Use the clock frequency (in MHz) to select the clock conversion		*/
		/* factor. According to the NCR53CF94/96 data manual, the conversion	*/
		/* factor is defined by the following table: Currently, we don't allow	*/
		/* the caller to change selection timeout from the ANSI standard		*/
		/* 250 Msec to avoid having to support floating-point register			*/
		/* manipulation															*/

	if ( fSCSIClockRate < 10)	fSCSIClockRate	= 10;
	if ( fSCSIClockRate > 40 )	fSCSIClockRate	= 40;

	if ( fSCSIClockRate <= 10 )
	{
	    clockConversionFactor	= ccf10MHz;
	    defaultSelectionTimeout	= SelTO16Mhz;
	}
	else if ( fSCSIClockRate <= 15 )
	{
	    clockConversionFactor	= ccf11to15MHz;
	    defaultSelectionTimeout	= SelTO16Mhz;
	}
	else if ( fSCSIClockRate <= 20 )
	{
	    clockConversionFactor	= ccf16to20MHz;
	    defaultSelectionTimeout	= SelTO16Mhz;
	}
	else if ( fSCSIClockRate <= 25 )
	{
	    clockConversionFactor	= ccf21to25MHz;
	    defaultSelectionTimeout	= SelTO25Mhz;
	}
	else if ( fSCSIClockRate <= 30 )
	{
	    clockConversionFactor	= ccf26to30MHz;
	    defaultSelectionTimeout	= SelTO33Mhz;
	}
	else if ( fSCSIClockRate <= 35 )
	{
	    clockConversionFactor	= ccf31to35MHz;
	    defaultSelectionTimeout	= SelTO40Mhz;
	}
	else
	{
	    clockConversionFactor	= ccf31to35MHz;
	    defaultSelectionTimeout	= SelTO40Mhz;
	}

	WRITE_REGISTER( rCKF, clockConversionFactor );

	WRITE_REGISTER( rINT, defaultSelectionTimeout);

		/* Configuration Register 2 - enable extended features	*/
		/* - mainly, 24-bit transfer count.						*/

	WRITE_REGISTER( rCF2, CF2_EPL );

		/* Configuration Register 3	*/

	configValue = 0;

#define gOptionFastModeEnable				0
#define gOptionFastModeSupportedByHardware	0

	if ( (gOptionFastModeEnable && gOptionFastModeSupportedByHardware)
	    || fSCSIClockRate > 25 )
	{
	    configValue |= CF3_FASTSCSI;
	}

	WRITE_REGISTER( rCF3, configValue );

		/* Configuration Register 4 - glitch eater, active negation.	*/
		/* Let's not worry about these whizzy features just yet.		*/

	WRITE_REGISTER( rCF4, 0 );

		/* Go to async xfer mode for now.	*/

	WRITE_REGISTER( rFOS, 0 );

		/* Reset SCSI bus, wait, clear possible interrupt.	*/

	if ( busReset )
	{
		setCmdReg( cRstSBus );
	    IOSleep( kSCSIResetDelay );
	    temp = fCurioAddr[ rINT ];		// clear the Interrupt register
	}
	return ioReturn;
}/* end resetHardware */


void CurioSCSIController::releaseHardwareMemoryMaps()
{
	if ( fDBDMAMemoryMap )
	{
	    fDBDMAMemoryMap->release();
	    fDBDMAMemoryMap = NULL;
	}

	if ( fSCSIMemoryMap )
	{
	    fSCSIMemoryMap->release();
	    fSCSIMemoryMap = NULL;
	}

	if ( fCCL )
	{
	    IOFree( fCCL, fCCLSize );
	}

	return;
}/* end releaseHardwareMemoryMaps */


	/* When a legitimate data phase starts, this method			*/
	/* is called to configure the DBDMA Channel Command list.	*/

IOReturn CurioSCSIController::generateCCL()
{
	IODBDMADescriptor	*dp; 					/* current descriptor		*/
	IODBDMADescriptor	*dpMax;					/* past the last descriptor	*/
	UInt32				dbdmaOp;				/* DBDMA opcode				*/
	UInt32				rangeByteCount, actualRanges;
	UInt32				i;
	IOPhysicalSegment	range[ kMaxMemoryCursorSegments ];


	
//	IODBDMAReset( fDBDMAAddr );		/* make sure that the DBDMA is idle.	*/
	fDBDMAAddr->channelControl = SWAP( 0x20002000 );		// set FLUSH bit
		SynchronizeIO();
	fDBDMAAddr->channelControl = SWAP( 0x80000000 );		// clr RUN   bit
		SynchronizeIO();

	if ( fCmdData->isWrite )		/* Select the correct DBDMA:			*/
		 dbdmaOp	= OUTPUT_MORE;
	else dbdmaOp	=  INPUT_MORE;

		/* How many descriptors can we store (need some slop for the	*/
		/* terminator commands).										*/
		/* Get a pointer to the first free descriptor and the total		*/
		/* number of bytes left to transfer in this I/O request.		*/

	dp		= (IODBDMADescriptor*)fCCL;
	dpMax	= (IODBDMADescriptor*)(fCCL + fCCLSize - 256);

		/* fThisTransferLength will contain the actual number	*/
		/* of bytes we intend to transfer in this DMA request,	*/
		/* which is needed when DMA completes to recover the	*/
		/* residual transfer length.							*/
	fThisTransferLength	= 0;

	while ( (dp < dpMax) && (fThisTransferLength < kMaxDMATransfer) )
	{
		actualRanges = fMemoryCursor->getPhysicalSegments(	fCmdData->mdp,
								fCmdData->results.bytesTransferred + fThisTransferLength,
								range,
								(unsigned int)kMaxMemoryCursorSegments,
								0, &rangeByteCount );
		if ( actualRanges == 0 )
			break;

		for ( i = 0; i < actualRanges; i++ )
		{
			ELG( range[ i ].location, range[ i ].length, '=Rng', "generateCCL - range" );
			dp->operation	= SWAP( dbdmaOp | range[ i ].length );
			dp->address		= SWAP(   (UInt32)range[ i ].location );
			dp->cmdDep		= 0;		// no branch address
			dp->result		= 0;
			dp++;
		}/* end FOR each range */

		fThisTransferLength += rangeByteCount;
	}/* end WHILE there is data to xfer */

	if ( (UInt8*)dp > fCCL )
	{
			/* We stored at least one descriptor.							*/
			/* Change the last one so it is an INPUT_LAST or OUTPUT_LAST.	*/

		dp->operation |= SWAP( IO_LAST );
	}

	dp->operation	= SWAP( STOP_CMD );
	dp->address		= 0;
	dp->cmdDep		= 0;
	dp->result		= 0;
	dp++;

	if( dp >= dpMax )
		PAUSE( dpMax, dp, 'Big-', "InitCmdCCL - CCL overrun" );

	flush_dcache( (vm_offset_t)fCCL,	(UInt32)dp - (UInt32)fCCL,	false );

	return kIOReturnSuccess;
}/* end generateCCL */


void CurioSCSIController::killActiveCommand( IOReturn finalStatus )
{
	if ( fCmd )
	{
		fCmdData->results.returnCode = finalStatus;
		completeCommand();
	}
	return;
}/* end killActiveCommand */


void CurioSCSIController::resetBus()
{
	setCmdReg( cRstSBus );
	super::resetOccurred();
	IOSleep( kSCSIResetDelay );
	enableCommands();
	return;
}/* end resetBus */
