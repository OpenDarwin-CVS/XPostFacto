/*

 Copyright (c) 2002, 2003
 Ryan Rempel
 All rights reserved

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer as the first lines of
 each file.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 This software is provided by Ryan Rempel ``as is'' and any express or implied
 warranties, including, but not limited to, the implied warranties of
 merchantability and fitness for a particular purpose are disclaimed. In no event
 shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but not
 limited to, procurement of substitute goods or services; loss of use, data, or
 profits; or business interruption) however caused and on any theory of
 liability, whether in contract, strict liability, or tort (including negligence
 or otherwise) arising in any way out of the use of this software, even if
 advised of the possibility of such damage.

 */
 
#ifndef __OWCCACHEINIT_H__
#define __OWCCACHEINIT_H__
 
#ifdef __cplusplus
extern "C" {
#endif
 
#include <ppc/asm.h>
#include <ppc/proc_reg.h>
 
#ifndef ASSEMBLER
 
unsigned OWCL2CacheInit (unsigned);
unsigned OWCL3CacheInit (unsigned);
 
#endif   // ASSEMBLER
 
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
#define pfNoL2PFNap	0x00008000
#define pfNoL2PFNapb	16
#define pfSlowNap	0x00004000
#define pfSlowNapb	17
#define pfNoMuMMCK	0x00002000
#define pfNoMuMMCKb	18
#define pfLClck		0x00001000
#define pfLClckb	19
#define pfWillNap	0x00000800
#define pfWillNapb	20
#define pfNoMSRir	0x00000400
#define pfNoMSRirb	21
#define pfL3pdet	0x00000200
#define pfL3pdetb	22
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
 
 // from assym.s

#define	pfAvailable	160
#define	pfrptdProc	164
#define	pflineSize	166
#define	pfl1iSize	168
#define	pfl1dSize	172
#define	pfl2cr	176
#define	pfl2Size	180
#define	pfl3cr	184
#define	pfl3Size	188
#define	pfHID0	192
#define	pfHID1	196
#define	pfHID2	200
#define	pfHID3	204
#define	pfMSSCR0	208
#define	pfMSSCR1	212
#define	pfICTRL	216
#define	pfLDSTCR	220
#define	pfLDSTDB	224
#define	pfl2crOriginal	228
#define	pfl3crOriginal	232
#define	pfBootConfig	236

/* Defines for PVRs */
#define PROCESSOR_VERSION_601		1
#define PROCESSOR_VERSION_603		3
#define PROCESSOR_VERSION_604		4
#define PROCESSOR_VERSION_603e		6
#define PROCESSOR_VERSION_750		8
#define PROCESSOR_VERSION_750FX		0x7000  /* ? */
#define PROCESSOR_VERSION_604e		9
#define PROCESSOR_VERSION_604ev		10	/* ? */
#define PROCESSOR_VERSION_7400		12	/* ? */
#define PROCESSOR_VERSION_7410		0x800C	/* ? */
#define PROCESSOR_VERSION_7450		0x8000	/* ? */
#define PROCESSOR_VERSION_7455		0x8001	/* ? */

/* Defines for L2 and L3 cache settings */
#define L2CLK_1		0x02000000	
#define L2CLK_15	0x04000000
#define L2CLK_35	0x06000000	// G4 only
#define L2CLK_2		0x08000000
#define L2CLK_25	0x0A000000
#define L2CLK_3		0x0C000000
#define L2CLK_4		0x0E000000	// G4 only
 
#define L2SIZ_2048	0x00000000  // G4 only
#define L2SIZ_256	0x10000000
#define L2SIZ_512	0x20000000
#define L2SIZ_1024	0x30000000
 
#define L2RAM_FT	0x00000000
#define L2RAM_PB	0x01000000
#define L2RAM_LW	0x01800000

#define L2OH_0		0x00000000
#define L2OH_1		0x00010000
#define L2OH_2		0x00020000		// only G4
#define L2OH_3		0x00030000		// only G4

#define l2em		0x80000000
#define l2tsm		0x00040000
#define l2slm		0x00008000
#define l2dfm		0x00004000
#define l2clkm		0x0E000000

#define l3em		0x80000000
#define l3sizm		0x10000000
#define l3clkenm	0x08000000
#define l3dxm		0x04000000
#define l3clkm		0x03800000
#define l3iom		0x00400000
#define l3ckspm		0x00030000
#define l3pspm		0x0000E000
#define l3repm		0x00001000
#define l3hwfm		0x00000800
#define l3im		0x00000400
#define	l3rtm		0x00000300
#define l3drom		0x00000100 
#define l3cyam		0x00000080
#define l3donlym	0x00000040
#define l3dmemm		0x00000004
#define l3dmsizm	0x00000001

#define L3CLK_6		0x00000000
#define L3CLK_5		0x03800000
#define L3CLK_4		0x03000000
#define L3CLK_35	0x02800000
#define L3CLK_3		0x02000000
#define L3CLK_25	0x01800000
#define L3CLK_2		0x01000000

#define L3CKSP_2	0x00000000
#define L3CKSP_3	0x00010000
#define L3CKSP_4	0x00020000
#define L3CKSP_5	0x00030000

#define L3PSP_0		0x00000000
#define L3PSP_1		0x00002000
#define L3PSP_2		0x00004000
#define L3PSP_3		0x00006000
#define L3PSP_4		0x00008000
#define L3PSP_5		0x0000A000

#define L3RT_DDR	0x00000000
#define L3RT_LW		0x00000100
#define L3RT_PB		0x00000300
 
#ifdef __cplusplus
}
#endif
 
#endif