/*

 Copyright (c) 2001, 2002
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 */

#include "XPFCPUSettings.h"

#define __mtspr(spr, val)  __asm__ volatile("mtspr  " # spr ", %0" : : "r" (val))
#define __mfspr(reg, spr)  __asm__ volatile("mfspr  %0, " # spr : "=r" (reg))
#define mtspr(spr, val) __mtspr(spr, val)
#define mfspr(reg, spr) __mfspr(reg, spr)

#define L2EM		0x80000000
#define L2IM		0x00200000
#define L2RESVM		0x000000FE
#define L2IPM		0x00000001

#define L2CR	1017

#define HID0	1008
#define ICTC	1019
#define SPDM	0x00000200
#define DPMM	0x00100000
#define PVR		287
#define PVR750	0x00080000		// a G3 processor
#define PVR7400	0x000C0000		// a G4 processor

void XPFfixProcessorSettings (void)
{
	// this undoes some stuff that the NVRAMRC does
	// added by ryan.rempel@utoronto.ca
	unsigned pvr, temp;
	mfspr (pvr, PVR);
	pvr &= 0xFFFF0000;
	if ((pvr == PVR750) || (pvr == PVR7400)) {
		mtspr (ICTC, 0);
		mfspr (temp, HID0);
		// these two undo the changs in NVRAMRC
		temp &= ~0x00000208;
		temp |= ~0xFFFFFF7F;
		// and this one turns on dynamic power management
		temp |= DPMM;
		mtspr (HID0, temp);
	}
}

void XPFsetUpL2Cache (void)
{
#if 0
	// This doesn't work yet.
	unsigned int l2crValue;
	int foundArg;
	foundArg = parse_boot_arg ("L2CR", &l2crValue);
	if (foundArg) {
		unsigned oldval, temp, x;
		x = 0;
		mfspr (oldval, L2CR);
		if (oldval & L2EM) return;
		if (! (l2crValue & L2EM)) return;

		l2crValue &= ~L2RESVM;
		printf ("Setting L2CR to: 0x%X\n", l2crValue);

		temp = (l2crValue & ~L2EM) | L2IM;	// we disable, and we invalidate
		printf ("Invalidated value: 0x%X\n", temp);
		__asm__ ("sync");
		__asm__ ("isync");
		mtspr (L2CR, temp);
		__asm__ ("sync");
		__asm__ ("isync");
		__asm__ ("eieio");
		do {		// we check to see when invalidation is finished
			x++;
			mfspr (temp, L2CR);
			__asm__ ("sync");
			__asm__ ("isync");
		} while (temp & L2IPM);
		if (x == 1) FailToBoot (1017);
		mtspr (L2CR, l2crValue); // now set the new value
		__asm__ ("sync");
		__asm__ ("isync");
	}
#endif
}
