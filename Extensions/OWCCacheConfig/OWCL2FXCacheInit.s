/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as defined 
in and that are subject to the Apple Public Source License Version 2.0 (the 'License').  
You may not use this file except in compliance with the License.  Please obtain a copy 
of the License at http://www.opensource.apple.com/apsl/ and read it before using this file. 

The Original Code and all software distributed under the License are distributed on an 
'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND APPLE HEREBY 
DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see the License 
for the specific language governing rights and limitations under the License.

*/

/*

 Copyright (c) 2002, 2003
 Other World Computing
 All rights reserved

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer as the first lines of
 each file.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or implied
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

#define ASSEMBLER 1

#include "OWCCacheInit.h"

/*  Initialize L2 Cache
 *
 *	unsigned OWCL2FXCacheInit (void)
 * 
 */

;			Force a line boundry here
			.align	5
			.globl	EXT(OWCL2FXCacheInit)

LEXT(OWCL2FXCacheInit)

			mfspr	r11,pvr							; get the processor version
			rlwinm	r11,r11,16,16,31				; just keep the most significant part
			
			cmplwi	r11,PROCESSOR_VERSION_750FX
			beq		doSetup
		
			li		r3,1		; ok, we do not support this
			blr
						
doSetup:

			mfspr	r9,hid0							; Get the current power-saving mode
			mfmsr	r7								; Get the current MSR
			
			rlwinm	r4,r9,0,dpm+1,doze-1			; Clear all possible power-saving modes (also disable DPM)	
			rlwinm	r7,r7,0,MSR_FP_BIT+1,MSR_FP_BIT-1	; Force floating point off
			rlwinm	r7,r7,0,MSR_VEC_BIT+1,MSR_VEC_BIT-1	; Force vectors off
			rlwinm	r5,r7,0,MSR_DR_BIT+1,MSR_DR_BIT-1	; Turn off translation		
			rlwinm	r5,r5,0,MSR_EE_BIT+1,MSR_EE_BIT-1	; Turn off interruptions
			
			mtspr	hid0,r4							; Set up the HID
			isync
									
			mtmsr	r5								; Translation and all off
			sync
			isync									; Toss prefetch
			
;
; turn on the L2 cache if not already on
;

			mfspr	r8,l2cr							; get the L2CR
			rlwinm.	r8,r8,0,l2e,l2e					; see if it was enabled
			bne		cinoexit						; get out if so
			
			lis		r2,hi16(l2im)					; Get the invalidate flag set
			sync
			mtspr	l2cr,r2							; Start the invalidate
			sync
			isync
	
ciinvdl2a:	mfspr	r2,l2cr							; Get the L2CR
			rlwinm.	r2,r2,0,l2ip,l2ip				; Is the invalidate still going?
			bne+	ciinvdl2a						; Assume so, this will take a looong time...
			
			lis		r8,hi16(l2em)					
			sync
			mtspr	l2cr,r8							; and turn on the cache
			sync
			isync
						
;
;	Restore entry conditions and leave
;

cinoexit:	
			sync
			mtspr	hid0,r9							; restore entry conditions
			sync
			isync
			mtmsr	r7								; Restore MSR to entry
			sync
			isync
			
;
; 	Update the per_proc_info
;

			mfsprg	r2,0						; get the per_proc_info
			mfsprg	r8,2						; get the features
			
			mfspr	r4,l2cr						; get the new L2CR
			stw		r4,pfl2cr(r2)				; store the L2CR
			
			rlwinm	r8,r8,0,pfL2b+1,pfL2b-1		; show L2 not available
			rlwinm.	r5,r4,0,l2e,l2e				; see if it is enabled now
			beq		ciupdateav					; skip to end if not
			
			lis		r10,hi16(512*1024)			; Base L2 size
			stw		r10,pfl2Size(r2)			; store the L2 size
				
			ori		r8,r8,pfL2					; show L2 available

ciupdateav:
			stw		r8,pfAvailable(r2)			; store Available
			mtsprg	2,r8						; and cache it
			
			blr									; Return...
