/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */
 
// From <osfmk/ppc/exception.h>

/* Miscellaneous constants and structures used by the exception
 * handlers
 */

#ifndef _PPC_EXCEPTION_H_
#define _PPC_EXCEPTION_H_

#ifndef ASSEMBLER

#define NCPUS 1

//#include <cpus.h>
//#include <mach_kdb.h>
//#include <mach_kdp.h>

#include <mach/machine/vm_types.h>
#include <mach/boolean.h>
#include <pexpert/pexpert.h>
#include <IOKit/IOInterrupts.h>
#include <ppc/machine_routines.h>

/*	Per processor CPU features */
struct procFeatures {
	unsigned int	Available;
#define pfFloat		0x80000000
#define pfFloatb	0
#define pfAltivec	0x40000000
#define pfAltivecb	1
#define pfAvJava	0x20000000
#define pfAvJavab	2
#define pfSMPcap	0x10000000
#define pfSMPcapb	3
#define pfCanSleep	0x08000000
#define pfCanSleepb	4
#define pfCanNap	0x04000000
#define pfCanNapb	5
#define pfCanDoze	0x02000000
#define pfCanDozeb	6
#define pfThermal	0x01000000
#define pfThermalb	7
#define pfThermInt	0x00800000
#define pfThermIntb	8
#define pfL23lck	0x00001000
#define pfL23lckb	19
#define pfWillNap	0x00000800
#define pfWillNapb	20
#define pfNoMSRir	0x00000400
#define pfNoMSRirb	21
#define pfL1nnc		0x00000200
#define pfL1nncb	22
#define pfL1i		0x00000100
#define pfL1ib		23
#define pfL1d		0x00000080
#define pfL1db		24
#define pfL1fa		0x00000040
#define pfL1fab		25
#define pfL2		0x00000020
#define pfL2b		26
#define pfL2fa		0x00000010
#define pfL2fab		27
#define pfL2i		0x00000008
#define pfL2ib		28
#define pfL3		0x00000004
#define pfL3b		29
#define pfL3fa		0x00000002
#define pfL3fab		30
#define pfValid		0x00000001
#define pfValidb	31
	unsigned short	rptdProc;
	unsigned short	lineSize;
	unsigned int	l1iSize;
	unsigned int	l1dSize;
	unsigned int	l2cr;
	unsigned int	l2Size;
	unsigned int	l3cr;
	unsigned int	l3Size;
	unsigned int	pfHID0;
	unsigned int	pfHID1;
	unsigned int	pfHID2;
	unsigned int	pfHID3;
	unsigned int	pfMSSCR0;
	unsigned int	pfMSSCR1;
	unsigned int	pfICTRL;
	unsigned int	pfLDSTCR;
};

typedef struct procFeatures procFeatures;

struct thrmControl {
	unsigned int	maxTemp;			/* Maximum temprature before damage */
	unsigned int	throttleTemp;		/* Temprature at which to throttle down */
	unsigned int	lowTemp;			/* Interrupt when temprature drops below */
	unsigned int	highTemp;			/* Interrupt when temprature exceeds this */
	unsigned int	thrm3val;			/* Value for thrm3 register */
	unsigned int	rsvd[3];			/* Pad to cache line */
};

typedef struct thrmControl thrmControl;

/* When an exception is taken, this info is accessed via sprg0 */
/* We should always have this one on a cache line boundary */
struct per_proc_info {
	unsigned short	cpu_number;
	unsigned short	cpu_flags;			/* Various low-level flags */
	vm_offset_t  	istackptr;
	vm_offset_t  	intstack_top_ss;

	vm_offset_t  	debstackptr;
	vm_offset_t  	debstack_top_ss;

	unsigned int	tempwork1;			/* Temp work area - monitor use carefully */
	unsigned int	save_exception_type;
	unsigned int	old_thread;

	/* PPC cache line boundary here - 020 */

	unsigned int	active_kloaded;		/* pointer to active_kloaded[CPU_NO] */
	unsigned int	cpu_data;			/* pointer to cpu_data[CPU_NO] */
	unsigned int	need_ast;			/* pointer to need_ast[CPU_NO] */
/*
 *	Note: the following two pairs of words need to stay in order and each pair must
 *	be in the same reservation (line) granule 
 */
	unsigned int	FPU_thread;			/* Thread owning the FPU on this cpu.*/
	unsigned int	FPU_vmmCtx;			/* Owing virtual machine context */
	unsigned int 	VMX_thread;			/* Thread owning the VMX on this cpu */
	unsigned int 	VMX_vmmCtx;			/* Owing virtual machine context */
	unsigned int	active_stacks;		/* pointer to active_stacks[CPU_NO] */

	/* PPC cache line boundary here - 040 */
	unsigned int 	quickfret;			/* Pointer to savearea for exception exit to free */
	unsigned int 	Lastpmap;			/* Last user pmap loaded  */
	unsigned int	userspace;			/* Last loaded user memory space ID  */
	unsigned int	userpmap;			/* User pmap - real address */
	unsigned int 	liveVRSave;			/* VRSave assiciated with live vector registers */
	unsigned int 	spcFlags;			/* Special thread flags */
	unsigned int	liveFPSCR;			/* FPSCR which is for the live context */
	unsigned int	ppbbTaskEnv;		/* BlueBox Task Environment */

	/* PPC cache line boundary here - 060 */
	boolean_t		interrupts_enabled;
	unsigned int	rsrvd064;
	IOInterruptHandler	interrupt_handler;
	void *			interrupt_nub;
	unsigned int	interrupt_source;
	void *			interrupt_target;
	void *			interrupt_refCon;
	unsigned int	savedSave;			/* Savearea saved across sleep - must be 0 at boot */

	/* PPC cache line boundary here - 080 */
	unsigned int	MPsigpStat;			/* Signal Processor status (interlocked update for this one) */
#define MPsigpMsgp		0xC0000000		/* Message pending (busy + pass) */
#define MPsigpBusy		0x80000000		/* Processor area busy, i.e., locked */
#define MPsigpPass		0x40000000		/* Busy lock passed to receiving processor */
#define MPsigpSrc		0x000000FF		/* Processor that owns busy, i.e., the ID of */
										/*   whomever set busy. When a busy is passed, */
										/*   this is the requestor of the function. */
#define MPsigpFunc		0x0000FF00		/* Current function */
#define MPsigpIdle		0x00			/* No function pending */
#define MPsigpSigp		0x04			/* Signal a processor */
#define SIGPast		0					/* Requests an ast on target processor */
#define SIGPcpureq	1					/* Requests CPU specific function */
#define SIGPdebug	2					/* Requests a debugger entry */
#define SIGPwake	3					/* Wake up a sleeping processor */
#define CPRQtemp	0					/* Get temprature of processor */
#define CPRQtimebase	1					/* Get timebase of processor */
	unsigned int	MPsigpParm0;		/* SIGP parm 0 */
	unsigned int	MPsigpParm1;		/* SIGP parm 1 */
	unsigned int	MPsigpParm2;		/* SIGP parm 2 */
	cpu_id_t		cpu_id;
	vm_offset_t		start_paddr;
	unsigned int	ruptStamp[2];		/* Timebase at last interruption */

	/* PPC cache line boundary here - 0A0 */
	procFeatures 	pf;					/* Processor features */
	
	/* PPC cache line boundary here - 0E0 */
	thrmControl		thrm;				/* Thermal controls */
	
	/* PPC cache line boundary here - 100 */
	unsigned int	napStamp[2];		/* Time base when we napped */
	unsigned int	napTotal[2];		/* Total nap time in ticks */
	unsigned int	numSIGPast;			/* Number of SIGP asts recieved */
	unsigned int	numSIGPcpureq;		/* Number of SIGP cpu requests recieved */
	unsigned int	numSIGPdebug;		/* Number of SIGP debugs recieved */
	unsigned int	numSIGPwake;		/* Number of SIGP wakes recieved */
	
	/* PPC cache line boundary here - 120 */
	unsigned int	spcTRc;				/* Special trace count */
	unsigned int	spcTRp;				/* Special trace buffer pointer */
	unsigned int 	Uassist;			/* User Assist Word */
	unsigned int	rsrvd12C[5];		/* Reserved slots */
	
	/* PPC cache line boundary here - 140 */
	time_base_enable_t	time_base_enable;
	unsigned int	rsrvd140[7];		/* Reserved slots */
	
	/* PPC cache line boundary here - 160 */
	unsigned int	rsrvd160[8];		/* Reserved slots */
	
	/* PPC cache line boundary here - 180 */
	unsigned int	rsrvd180[8];		/* Reserved slots */
	
	/* PPC cache line boundary here - 1A0 */
	unsigned int	rsrvd1A0[8];		/* Reserved slots */
	
	/* PPC cache line boundary here - 1C0 */
	unsigned int	rsrvd1C0[8];		/* Reserved slots */
	
	/* PPC cache line boundary here - 1E0 */
	double			emfp0;				/* Copies of floating point registers */
	double			emfp1;				/* Used for emulation purposes */
	double			emfp2;
	double			emfp3;

	double			emfp4;				
	double			emfp5;
	double			emfp6;
	double			emfp7;

	double			emfp8;
	double			emfp9;
	double			emfp10;
	double			emfp11;
	
	double			emfp12;
	double			emfp13;
	double			emfp14;
	double			emfp15;
	
	double			emfp16;
	double			emfp17;
	double			emfp18;
	double			emfp19;

	double			emfp20;
	double			emfp21;
	double			emfp22;
	double			emfp23;
	
	double			emfp24;
	double			emfp25;
	double			emfp26;
	double			emfp27;
	
	double			emfp28;
	double			emfp29;
	double			emfp30;
	double			emfp31;

/*								   - 2E0 */
	unsigned int 	emfpscr_pad;
	unsigned int 	emfpscr;
	unsigned int	empadfp[6];
	
/*								   - 300 */
	unsigned int	emvr0[4];			/* Copies of vector registers used both */
	unsigned int	emvr1[4];			/* for full vector emulation or */
	unsigned int	emvr2[4];			/* as saveareas while assisting denorms */
	unsigned int	emvr3[4];
	unsigned int	emvr4[4];
	unsigned int	emvr5[4];
	unsigned int	emvr6[4];
	unsigned int	emvr7[4];
	unsigned int	emvr8[4];
	unsigned int	emvr9[4];
	unsigned int	emvr10[4];
	unsigned int	emvr11[4];
	unsigned int	emvr12[4];
	unsigned int	emvr13[4];
	unsigned int	emvr14[4];
	unsigned int	emvr15[4];
	unsigned int	emvr16[4];
	unsigned int	emvr17[4];
	unsigned int	emvr18[4];
	unsigned int	emvr19[4];
	unsigned int	emvr20[4];
	unsigned int	emvr21[4];
	unsigned int	emvr22[4];
	unsigned int	emvr23[4];
	unsigned int	emvr24[4];
	unsigned int	emvr25[4];
	unsigned int	emvr26[4];
	unsigned int	emvr27[4];
	unsigned int	emvr28[4];
	unsigned int	emvr29[4];
	unsigned int	emvr30[4];
	unsigned int	emvr31[4];
	unsigned int	emvscr[4];			
	unsigned int	empadvr[4];			
/*								   - 520 */

	unsigned int	patcharea[56];
/*								   - 600 */

};


extern struct per_proc_info per_proc_info[NCPUS];

#endif  // #ifndef ASSEMBLER

#endif