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

	/* Copyright 1997-1999 Apple Computer Inc. All Rights Reserved.		*/
	/* @author  Martin Minow											*/
	/* Private structures and definitions for Apple 53C96 SCSI driver.	*/



	/*****	For fans of kprintf, IOLog and debugging infrastructure of the	*****/
	/*****	string ilk, please modify the ELG and PAUSE macros or their		*****/
	/*****	associated EvLog and Pause functions to suit your taste. These	*****/
	/*****	macros currently are set up to log events to a wraparound		*****/
	/*****	buffer with minimal performance impact. They take 2 UInt32		*****/
	/*****	parameters so that when the buffer is dumped 16 bytes per line,	*****/
	/*****	time stamps (~1 microsecond) run down the left side while		*****/
	/*****	unique 4-byte ASCII codes can be read down the right side.		*****/
	/*****	Preserving this convention facilitates different maintainers	*****/
	/*****	using different debugging styles with minimal code clutter.		*****/

#define USE_ELG   0					// for debugging
#define kEvLogSize  (4096*16)		// 16 pages = 64K = 4096 events

#if USE_ELG /* (( */
#define ELG(A,B,ASCI,STRING)    EvLog( (UInt32)(A), (UInt32)(B), (UInt32)(ASCI), STRING )
#define PAUSE(A,B,ASCI,STRING)  Pause( (UInt32)(A), (UInt32)(B), (UInt32)(ASCI), STRING )
#else /* ) not USE_ELG: (   */
#define ELG(A,B,ASCI,S)
#define PAUSE(A,B,ASCI,STRING)	IOLog( "Curio: %8x %8x " STRING, (unsigned int)(A), (unsigned int)(B) )
#endif /* USE_ELG )) */


#if USE_ELG
            /* for debugging:   */
//	extern void     EvLog( UInt32 a, UInt32 b, UInt32 ascii, char* str );
//	extern void     Pause( UInt32 a, UInt32 b, UInt32 ascii, char* str );
//	extern void     AllocateEventLog( UInt32 ); // defined in miniMon
#endif /* NotMiniMon */


#ifndef SynchronizeIO
#define SynchronizeIO()     eieio()     /* TEMP */
#endif /* SynchronizeIO */

#define SWAP(x) (UInt32)OSSwapInt32( (UInt32)(x) ) 



	enum	/* Generic numeric parameters.	*/
	{		/* gMaxDMATransfer is set so that we don't have to worry about the	*/
			/* ambiguous "zero" value in the Curio and DBDMA transfer registers	*/
			/* that can mean either 65536 bytes or zero bytes.					*/
		kMaxDMATransfer		= 0xF000,	// round down 1 page
			/* Max number memory segments to fetch from the memory cursor.	*/
		kMaxMemoryCursorSegments	= 16,

		kMaxTargetID	= 8,/* The maximum number of targets and logical units.	*/
		kMaxLUN			= 8,
			/* The default initiator bus ID (needs to be fetched from NVRAM).	*/
		kInitiatorIDDefault	= 7,

		kSCSIResetDelay	= 250,	/*  Delay (in msec) after Bus Reset	*/
			/* Size of buffers used to store message-phase bytes.	*/
		kMessageInBufferLength	= 16,
		kMessageOutBufferLength	= 16
	};


    enum	/* These values represent no active request:	*/
    {   kInvalidTarget	= 0xFF,
        kInvalidLUN		= 0xFF,
		kInvalidTag		= 0xFFFFFFFF
    };

	enum	// Registry stuff:
	{
	    kCurioRegisterBase	= 0,
	    kDBDMARegisterBase	= 1,
	    kNumberRegisters	= 2
	};

	enum	// SCSI Message byte ranges
	{
		kSCSIMsgOneByteMax	= 0x1Fu,
		kSCSIMsgTwoByteMin	= 0x20u,
		kSCSIMsgTwoByteMax	= 0x2Fu
	};



	typedef enum BusPhase	/* These are the real SCSI bus phases		*/
	{
		kBusPhaseDATO	    = 0,
		kBusPhaseDATI,
		kBusPhaseCMD,
		kBusPhaseSTS,
		kBusPhaseReserved1,
		kBusPhaseReserved2,
		kBusPhaseMSGO,
		kBusPhaseMSGI,
		kBusPhaseBusFree
	} BusPhase;


	typedef enum MsgInState
	{
		kMsgInInit = 0,	/*  0 Not reading a message (must be zero)	*/
		kMsgInReading,	/*  1 MSG input state: reading counted data	*/
		kMsgInCounting,	/*  2 MSG input state: reading count byte	*/
		kMsgInReady		/*  3 MSG input state: a msg is now available	*/
	} MsgInState;


		/* Values for the finite-state automaton, stored in the fBusState		*/
		/* instance variable:													*/
		/*	SCS_DISCONNECTED	Normal "bus free" state (we're not processing	*/
		/* 						any commands).									*/
		/*	SCS_SELECTING		Just tried to select a remote target.			*/
		/* 	SCS_RESELECTING		Reselection transistion state					*/
		/* 	SCS_INITIATOR		Normal "processing bus phases" state. Set		*/
		/* 						after correctly responding to an interrupt.		*/
		/* 						Changed to an in-progress state.				*/
		/*	SCS_COMPLETING		In command-complete sequence					*/
		/* 	SCS_WAIT_FOR_BUS_FREE	After disconnect or command complete, before*/
		/* 						seeing disconnect interrupt or reselection		*/
		/* 	SCS_DMACOMPLETE		After starting DMA, waiting for completion		*/
		/* 	SCS_SENDINGMSG		After sending an MSGO byte						*/
		/*	SCS_GETTINGMSG		While getting MSGI bytes						*/
		/*	SCS_SENDINGCMD		While sending CMDO bytes						*/
		/*	SCS_DEATH_MARCH		The target got lost. Follow phases until		*/
		/* 						it disconnects.									*/
	typedef enum
	{
		SCS_DISCONNECTED,		/* disconnected								*/
		SCS_SELECTING,			/* SELECT command issued 					*/
		SCS_RESELECTING,		/* Handle reselection after interrrupt		*/
		SCS_INITIATOR,			/* following target SCSI phase				*/
		SCS_COMPLETING,			/* initiator command complete in progress	*/
		SCS_WAIT_FOR_BUS_FREE,	/* transition after disconnect or complete	*/
		SCS_DMACOMPLETE,		/* dma (in or out) is in progress			*/
		SCS_SENDINGMSG,			/* MSG_OUT phase in progress				*/
		SCS_GETTINGMSG,			/* transfer msg in progress					*/
		SCS_SENDINGCMD,			/* command out in progress					*/
		SCS_DEATH_MARCH			/* recovery from target confusion			*/
	} BusState;


		/* These constants define the DBDMA channel command operations and modifiers:	*/

	enum				/* Command.cmd operations	*/
	{
		OUTPUT_MORE		= 0x00000000,
		OUTPUT_LAST		= 0x10000000,
		INPUT_MORE		= 0x20000000,
		INPUT_LAST		= 0x30000000,
		IO_LAST			= 0x10000000,
		STOP_CMD		= 0x70000000,
	};


		/* Define offsets into the Curio chip:	*/

	typedef enum
	{
		rXCL	= 0x00,		/* Transfer counter LSB							*/
		rXCM	= 0x10,		/* Transfer counter MSB							*/
		rFFO	= 0x20,		/* FIFO											*/
		rCMD	= 0x30,		/* Command										*/
		rSTA	= 0x40,		/* Status (r) or Destination bus ID				*/
		rINT	= 0x50,		/* Interrupt (r) or Select/reselect timeout		*/
		rSQS	= 0x60,		/* Sequence step (r) or Synch Period			*/
		rFOS	= 0x70,		/* FIFO Flags/Sequence Step (r) or Sync Offset	*/
		rCF1	= 0x80,		/* Configuration 1								*/
		rCKF	= 0x90,		/* Clock Conversion Factor						*/
		rTST	= 0xA0,		/* Test											*/
		rCF2	= 0xB0,		/* Configuration 2								*/
		rCF3	= 0xC0,		/* Configuration 3								*/
		rCF4	= 0xD0,		/* Configuration 4								*/
		rTCH	= 0xE0,		/* Transfer counter high/ID						*/
		rDMA	= 0xF0		/* Pseudo-DMA									*/
	} CurioRegisters;

		/* SCSI 53C96 (ASC - Advanced SCSI Controller) Command Set	*/

	enum
	{
		cNOP			= 0x00,		/* NOP command						*/
		cFlshFFO		= 0x01,		/* flush FIFO command				*/
		cRstSChp		= 0x02,		/* reset SCSI chip					*/
		cRstSBus		= 0x03,		/* reset SCSI bus					*/

		cIOXfer			= 0x10,		/* non-DMA Transfer command			*/
		cCmdComp		= 0x11,		/* Initiator Command Complete Seq	*/
		cMsgAcep		= 0x12,		/* Message Accepted					*/
		cXferPad		= 0x18,		/* Transfer pad						*/
		cDisconnect		= 0x23,		/* Disconnect from the SCSI bus		*/
		cSlctNoAtn		= 0x41,		/* Select Without ATN Sequence		*/
		cSlctAtn		= 0x42,		/* Select With ATN Sequence			*/
		cSlctAtnStp		= 0x43,		/* Select With ATN and Stop Seq		*/
		cEnSelResel		= 0x44,		/* Enable Selection/Reselection		*/
		cDsSelResel		= 0x45,		/* Disable Selection/Reselection	*/
		cSlctAtn3		= 0x46,		/* Select with ATN, send 3 byte msg	*/

		cSetAtn			= 0x1A,		/* Set ATN command					*/
		cRstAtn			= 0x1B,		/* Reset ATN command				*/

		bDMAEnBit		= 0x80,		/* DMA command						*/
		bDscCmdState	= 0x40,		/* Disconnected State Group Cmd's	*/

		cDMAXfer		= ( bDMAEnBit | cIOXfer ),		/* DMA Transfer Cmd	*/
		cDMAXferPad		= ( bDMAEnBit | cXferPad ),		/* DMA Transfer Pad	*/
		cDMASelWAtn		= ( bDMAEnBit | cSlctAtn )		/* Sel With ATN use DMA */
	};

		/* (4 - rSTA) Status register bit definitions:	*/

	enum
	{
		sIO			= 0x01,						/* I/O bit					*/
		sCD			= 0x02,						/* C/D bit					*/
		sMsg		= 0x04,						/* MSG bit					*/
		sCmdComp	= 0x08,						/* function complete bit	*/
		sTermCount	= 0x10,						/* bus service bit			*/
		sParityErr	= 0x20,						/* disconnected bit			*/
		sGrossErr	= 0x40,						/* illegal command bit		*/
		sIntPend	= 0x80,						/* SCSI interrupt pending	*/
		mPhase		= (sIO | sCD | sMsg),		/* the phase bitmask		*/
		sTCIntPend	= (sTermCount | sIntPend)	/* TC int pending			*/
	};


		/* (5 - rINT) Interrupt register bit definitions:	*/

	enum
	{
		iSelected		= 0x01,		/* selected bit				*/
		iSelectWAtn		= 0x02,		/* selected w/ ATN bit		*/
		iReselected		= 0x04,		/* reselected bit			*/
		iFuncComp		= 0x08,		/* function complete bit	*/
		iBusService		= 0x10,		/* bus service bit			*/
		iDisconnect		= 0x20,		/* disconnected bit			*/
		iIlegalCmd		= 0x40,		/* illegal command bit		*/
		iResetDetect	= 0x80u		/* SCSI reset detected bit	*/
	};


		/* (7 - rFOS) FIFO Count/Sequence Step register.	*/

	enum
	{
		kFIFOCountMask	= 0x1F	/* mask to get FIFO count		*/
	};


#define INS_STATE_MASK	0x07	/* internal state register.	*/

		/* Clock Conversion Values (based on SCSI chip clock - not CPU clock)	*/

	enum
	{
		ccf10MHz		= 0x02,		/* CLK conv factor 10.0Mhz			*/
		ccf11to15MHz	= 0x03,		/* CLK conv factor 10.01 to 15.0Mhz	*/
		ccf16to20MHz	= 0x04,		/* CLK conv factor 15.01 to 20.0Mhz	*/
		ccf21to25MHz	= 0x05,		/* CLK conv factor 20.01 to 25.0Mhz	*/
		ccf26to30MHz	= 0x06,		/* CLK conv factor 25.01 to 30.0Mhz	*/
		ccf31to35MHz	= 0x07,		/* CLK conv factor 30.01 to 35.0Mhz	*/
		ccf36to40MHz	= 0x00		/* CLK conv factor 35.01 to 40.0Mhz (0 <- 8) */
	};

		/* Select timeout values (these are the values stored in the	*/
		/* chip register).												*/
		/* The "Mhz" value is the SCSI bus speed from the Registry.		*/
	enum
	{
		SelTO16Mhz	= 126,	/* (0x7e)  using the formula: RV (regr value)	*/
							/*   126 =  (16MHz * 250mS)/ (7682 * 4)			*/
							/*	250mS is ANSI standard.						*/
		SelTO25Mhz	= 167,	/* (0xa7)  using the formula: RV (regr value)	*/
							/*   163 =  (25MHz * 250mS)/ (7682 * 5)			*/
							/*	250mS is ANSI standard.						*/
		SelTO33Mhz	= 167,	/* (0xa7)  using the formula: RV (regr value)	*/
							/*   153 =  (33MHz * 250mS)/ (7682 * 7)			*/
							/*	250mS is ANSI standard.						*/
		SelTO40Mhz	= 167	/* (0xa7)  using the formula: RV (regr value)	*/
							/*   163 =  (40MHz * 250mS)/ (7682 * 8)			*/
							/*	250mS is ANSI standard.						*/
	};

		/* (8) Configuration Register 1 bit definition	*/

	enum
	{
		CF1_SLOW		= 0x80,		/* Slow Cable Mode enabled bit				*/
		CF1_SRD			= 0x40,		/* SCSI Reset Reporting Int Disabled bit	*/
		CF1_PTEST		= 0x20,		/* Parity Test Mode bit						*/
		CF1_ENABPAR		= 0x10,		/* Enable Parity Checking bit				*/
		CF1_CHIPTEST	= 0x08,		/* Enable Chip Test Mode bit				*/
		CF1_DEFAULT_ID_MASK	= 0x07	/* The default host SCSI ID mask.			*/
	};

		/* (B) Configuration Register 2 bit definition	*/
	enum
	{
		CF2_RFB			= 0x80,		/* Reserve FIFO Byte			*/
		CF2_EPL			= 0x40,		/* Enable phase latch			*/
		CF2_EBC			= 0x20,		/* Enable Byte Control			*/
		CF2_DREQHIZ		= 0x10,		/* Force DREQ to HI-Z state		*/
		CF2_SCSI2		= 0x08,		/* SCSI-2 features				*/
		CF2_BPA			= 0x04,		/* Target:bad parity abort		*/
		CF2_RPE			= 0x02,		/* Rregister parity enable		*/
		CF2_DPE			= 0x01		/* DMA parity enable			*/
	};

		/* (C) Configuration Register 3 bit definition	*/
	enum
	{
		CF3_MSGID		= 0x80,		/* Check for valid ID message			*/
		CF3_QTAG		= 0x40,		/* Enable 3 byte QTAG messages			*/
		CF3_CDB10		= 0x20,		/* Recognize 10 bytes CDBs				*/
		CF3_FASTCLOCK	= 0x10,		/* Enable fast clock for fast SCSI		*/
		CF3_FASTSCSI	= 0x08,		/* Enable fast SCSI						*/
		CF3_SRB			= 0x04,		/* Save residual byte					*/
		CF3_ALTDMA		= 0x02,		/* Alternate DMA (for threshold 8 only)	*/
		CF3_T8			= 0x01,		/* Force 8-byte DMA caching				*/
		CONFIG_FOR_DMA		= (CF3_SRB | CF3_ALTDMA | CF3_T8),
		CONFIG_FOR_NON_DMA	= (CF3_SRB)
	};


	enum
	{
		kDefaultInitiatorID		= 7,	/* SCSI Bus Initiator ID			*/
		kChipDefaultBusClockMHz	= 25	/* Default 53c96 clock rate in MHz	*/
	};



#define WRITE_REGISTER( r, v )	{ fCurioAddr[ r ] = v; SynchronizeIO(); }




	class CurioSCSIController;

    typedef struct globals      /* Globals for this module (not per instance)   */
    {
        UInt32       evLogFlag; // debugging only
        UInt8       *evLogBuf;
        UInt8       *evLogBufe;
        UInt8       *evLogBufp;
        UInt8       intLevel;

        UInt32		shadowRegs[ 3 ]; // move to per instance??? /* Last Curio register state      */

        UInt32      cclLogAddr,     cclPhysAddr;		// for debugging ease
        UInt32      curioAddr;                   		// for debugging ease
		class CurioSCSIController	*curioInstance;
    } globals;


	typedef struct PrivCmdData	/* command data private to CurioSCSIController	*/
	{
		IOMemoryDescriptor	*mdp;				/* Memory Descriptor Pointer	*/
		UInt32				xferCount;
		UInt32				savedDataPosition;	/* getPosition at disconnect	*/
		SCSIResults			results;
		bool				isWrite;
	} PrivCmdData;



#define super	IOSCSIParallelController


class CurioSCSIController : public IOSCSIParallelController
{
    OSDeclareDefaultStructors( CurioSCSIController )	;/* Constructor & Destructor stuff	*/

private:

    IOService				*fProvider;
    IOSCSIParallelCommand		*fCmd;
    PrivCmdData				*fCmdData;
    IOMemoryMap				*fIOMap;
    IOInterruptEventSource	*fInterruptEvent;


    IOMemoryMap			*fSCSIMemoryMap;
	volatile UInt8		*fCurioAddr;		/* curio registers (logical)		*/
	UInt32				fCurioPhysAddr;		/* curio registers (physical)		*/

    IOMemoryMap			*fDBDMAMemoryMap;
    volatile IODBDMAChannelRegisters	*fDBDMAAddr;  		/* DBDMA registers	*/
	IOPhysicalAddress	fDBDMAAddrPhys;		/* DBDMA registers (physical)		*/

	IOPhysicalAddress	fCCLPhysAddr;		/* Channel Command List (physical)	*/
	UInt8				*fCCL;				/* Channel Command List (logical)	*/
	UInt32				fCCLSize;			/* Channel Command List size		*/
	UInt32				fDBDMADescriptorMax;/* max # Channel Commands			*/

    IOBigMemoryCursor	*fMemoryCursor;		/* ptr to Big-endian memory Cursor	*/

	UInt32			fReadAlignmentCount;	// hack for DBDMA bug at start of
	UInt32			fReadAlignmentIndex;	// Read when buffer is misaligned

	SCSITargetLun	fCurrentTargetLun;
	UInt32       	fTag;					/* Last tag value					*/
	UInt8			fTagType;				/* Last tag type - simple queue...	*/

#define kFlagMsgIn_Reject       0x01
#define kFlagMsgIn_Disconnect   0x02
    UInt8       fMsgInFlag;

#define kFlagMsgOut_SDTR        0x01
#define kFlagMsgOut_Queuing     0x02

    UInt8       fInitiatorID;           /* Our SCSI ID                  */
    UInt8       fInitiatorIDMask;       /* BusID bitmask for selection  */

    UInt8		fSaveStatus;			/* Shadow status register		*/
    UInt8		fSaveSeqStep;			/* Shadow sequence xtep reg.	*/
    UInt8		fSaveInterrupt;			/* Shadow interrupt register	*/
    bool		fCkForAnotherInt;
    bool		fNeedAnotherInterrupt;
    BusState	fBusState;				/* The overall state automaton	*/
    bool		fBusBusy;

    UInt32		fThisTransferLength;	/* Current data transfer count	*/
    UInt8		fCurrentBusPhase;		/* Last known SCSI phase		*/

	SInt32		fMsgInCount;			/* Message bytes still to read	*/
	MsgInState	fMsgInState;			/* How are we handling messages	*/
	UInt32		fMsgInIndex;			/* Free spot in fMsgInBuffer	*/
	UInt8		fMsgInBuffer[ kMessageInBufferLength ];

	UInt8		*fpMsgOut;				/* next MsgO byte to buffer		*/
	UInt8		*fpMsgPut;				/* next MsgO byte fm buffer		*/
	UInt8		fMsgOutBuffer[ kMessageOutBufferLength ];
	UInt8		fMsgOutCount;

    UInt32		fSCSIClockRate;			/* Chip clock in MHz		*/



protected:

	bool		configure( IOService*, SCSIControllerInfo* );
	void		executeCommand(	IOSCSIParallelCommand* );
	void		cancelCommand(	IOSCSIParallelCommand* );
	void		resetCommand(	IOSCSIParallelCommand* );

private:

	IOReturn	initHardware();
	IOReturn	getHardwareMemoryMaps();
    void     	releaseHardwareMemoryMaps();
	IOReturn	allocHdwAndChanMem();
	IOReturn	doHBASelfTest();
	void		initCP();

	void 		interruptOccurred( IOInterruptEventSource *ies, int intCount );
	void		doHardwareInterrupt();
	void		processInterrupt();
	bool		quickCheckForInterrupt();
    bool		interruptPending();
    void		handleReselectionInterrupt();
    IOReturn	getReselectionTargetID( UInt8 selectByte );
    void		reselectNexus();

	bool		startCommand();
	void		completeCommand();

	void		fsmSelecting();
	void		fsmDisconnected();
	void		fsmWaitForBusFree();
	void		fsmInitiator();
	void		fsmCompleting();
	void		fsmDMAComplete();
	void		fsmGettingMsg();
	IOReturn	fsmProcessMessage();
	void		fsmSendingCmd();
	void		fsmReselecting();
	void		fsmPhaseChange();
	void		fsmStartDataPhase( bool isDataInPhase );
	void		fsmStartErrorRecovery( IOReturn status );
	void		fsmErrorRecoveryInterruptService();
	void		fsmContinueErrorRecovery();

	IOReturn	generateCCL();
	void		setCmdReg( UInt8 );
    void		putCDBIntoFIFO();

    void		resetCurio();
    IOReturn	resetHardware( bool busReset );
	void		resetBus();

    void		killActiveCommand( IOReturn finalStatus );
};/* end class CurioSCSIController */
