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

/*  Initialize L3 Cache
 *
 *	unsigned XPFL3CacheInit (unsigned)
 *
 *  Initialize the L3CR to the specified value. Returns the new L3CR value.
 *
 */

;			Force a line boundry here
			.align	5
			.globl	EXT(OWCL3CacheInit)

LEXT(OWCL3CacheInit)

			mfspr	r11,pvr							; get the processor version
			rlwinm	r11,r11,16,16,31				; just keep the most significant part
			
			cmplwi	r11,PROCESSOR_VERSION_7450
			beq		doSetup
			cmplwi	r11,PROCESSOR_VERSION_7455
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
						
			dssall									; Stop streams
			sync
			
			mtmsr	r5								; Translation and all off
			sync
			isync									; Toss prefetch
			
;
; turn on the L2 cache if not already on
;

			mfspr	r8,l2cr							; get the L2CR
			rlwinm.	r8,r8,0,l2e,l2e					; see if it was enabled
			bne		cil3							; move on to L3 if so
			
			lis		r2,hi16(l2im)					; Get the invalidate flag set
			sync
			mtspr	l2cr,r2							; Start the invalidate
			sync
			isync
	
ciinvdl2a:	mfspr	r2,l2cr							; Get the L2CR
			rlwinm.	r2,r2,0,l2i,l2i					; Is the invalidate still going?
			bne+	ciinvdl2a						; Assume so, this will take a looong time...
											
			lis		r8,hi16(l2em)					
			sync
			mtspr	l2cr,r8							; and turn on the cache
			sync
			isync

cil3:
			
;
;			Flush and enable the level 3
;

			mfspr	r8,l3cr							; Get the L3CR
			rlwinm.	r0,r8,0,l3e,l3e					; Was the L3 enabled?
			beq		ciinvdl3						; No, skip flush
			
ciflushl3:
			sync									; 7450 book says do this even though not needed
			mr		r10,r8							; Take a copy now
						
			oris	r10,r10,hi16(l3iom)				; Set instruction-only
			ori		r10,r10,lo16(l3donlym)			; Set data-only
			sync
			mtspr	l3cr,r10						; Lock out the cache
			sync
			isync
			
cinol3lck:	ori		r10,r10,lo16(l3hwfm)			; Request flush
			sync									; Make sure everything is done
			
			mtspr	l3cr,r10						; Request flush
			
cihwfl3:	mfspr	r10,l3cr						; Get back the L3CR
			rlwinm.	r10,r10,0,l3hwf,l3hwf			; Is the flush over?
			bne+	cihwfl3							; Nope, keep going...
			
;
;	invalidate the L3
;

ciinvdl3:	rlwinm	r8,r3,0,l3e+1,31				; Use desired L3CR value and clear the enable bit
			sync									; Make sure of life, liberty, and justice
			mtspr	l3cr,r8							; Disable L3
			sync

			cmplwi	r3, 0							; Should the L3 be all the way off?
			beq	cinoexit							; Yes, done with L3
			rlwinm. r2,r3,0,l3dmem,l3dmem			; Are we in test mode?
			bne 	citestL3						; Skip the invalidate

			ori		r8,r8,lo16(l3im)				; Get the invalidate flag set

			mtspr	l3cr,r8							; Start the invalidate

ciinvdl3b:	mfspr	r8,l3cr							; Get the L3CR
			rlwinm.	r8,r8,0,l3i,l3i					; Is the invalidate still going?
			bne+	ciinvdl3b						; Assume so...
			sync

			b		cil3pdet						; Not in test mode, or we would have branched above
;
;	Setup for test mode
;

citestL3:
			lis		r10,0xFFE0					; Dead recon ROM for now
			rlwinm	r8,r10,28,4,15				; compensate for extended addressing off
			sync
			mtspr	983,r8						; set things up for test mode
			sync
			isync

cil3pdet:
			mfspr	r10, pir						; ?
			rlwinm.	r10, r10, 24, 28, 31			; ?
			beq	ciinvdl3nopdet						; ?
			
			mfspr	r8,l3pdet						; ?
			srw		r2, r8, r10						; ?
			rlwimi	r2, r8, 0, 24, 31				; ?
			subfic	r10, r10, 32					; ?
			li		r8, -1							; ?
			ori		r2, r2, 0x0080					; ?
			slw		r8, r8, r10						; ?
			or		r8, r2, r8						; ?
			mtspr	l3pdet, r8						; ?
			isync

ciinvdl3nopdet:
			mfspr	r8,l3cr							; Get the L3CR
			rlwinm	r8,r8,0,l3clken+1,l3clken-1		; Clear the clock enable bit
			mtspr	l3cr,r8							; Disable the clock

			li		r2,128							; ?
ciinvdl3c:	addi	r2,r2,-1						; ?
			cmplwi	r2,0							; ?
			bne+	ciinvdl3c

			mfspr	r10,msssr0						; ?
			rlwinm	r10,r10,0,vgL3TAG+1,vgL3TAG-1	; ?
			mtspr	msssr0,r10						; ?
			sync

			mtspr	l3cr,r3							; Enable it as desired
			sync
			
;
;	Execute the test (if desired)
;
	
			rlwinm.	r2,r3,0,l3dmem,l3dmem			; Are we in test mode?
			beq		cinoexit						; If not, we are done
		
;
;	Write test values to the cache.
;

			lis		r0,hi16(1024*1024)			; Base L3 size
			rlwinm	r10,r3,4,31,31				; Get size multiplier
			slw		r0,r0,r10					; Set 1 or 2MB
			srwi	r0,r0,5						; get the number of lines
			mtctr	r0							; set the number of lines

			lis		r10,0xFFE0					; Dead recon ROM for now
													
citestwrl2:	
			stw		r10,0(r10)						; write the address to the cache line
			stw		r10,4(r10)
			stw 	r10,8(r10)
			stw		r10,12(r10)
			stw		r10,16(r10)
			stw		r10,20(r10)
			stw		r10,24(r10)
			stw		r10,28(r10)
			dcbf	0,r10							; flush the write to the L3 cache
			addi	r10,r10,32						; Next line
			bdnz	citestwrl2						; Do the lot...
			
;
;	Read the test values back from the cache.
;
			
			li		r4,0							; counter for the "successful" lines
			lis		r10,0xFFE0						; Dead recon ROM for now
			mtctr	r0								; set the number of lines
			
citestrdl2:
			lwz		r8,0(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,4(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,8(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,12(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,16(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,20(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,24(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			lwz		r8,28(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			
			addi	r4,r4,1							; increment the "success" line counter
citestfail:	
			addi	r10,r10,32						; next line
			bdnz	citestrdl2						; do the lot

			sync
			slwi	r3,r4,5							; return how much SRAM tested OK (multiply by 32)
		
;
;	Clean up from the test. Basically, just turn off the cache. 
;	Everything should be invalid at this stage.
;

			li		r8,0
			mtspr	l3cr,r8							; Disable L3
						
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
			beq		ciupdatel3					; skip to L3 if not
			
			lis		r10,hi16(256*1024)			; Base L2 size
			rlwinm	r0,r4,22,12,13				; Convert to 256k, 512k, or 768k
			add		r10,r10,r0					; Add in minimum

			stw		r10,pfl2Size(r2)			; store the L2 size
				
			ori		r8,r8,pfL2					; show L2 available
									
ciupdatel3:
			mfspr	r4,l3cr						; get the new L3CR
			stw		r4,pfl3cr(r2)				; store it

			rlwinm	r8,r8,0,pfL3b+1,pfL3b-1		; show L3 not available
			rlwinm.	r5,r4,0,l3e,l3e				; see if it is enabled now
			beq		ciupdateav					; skip to end if not

			lis		r10,hi16(1024*1024)			; Base L3 size
			rlwinm	r0,r4,4,31,31				; Get size multiplier
			slw		r10,r10,r0					; Set 1 or 2MB
		
			stw		r10,pfl3Size(r2)
			
			ori		r8,r8,pfL3					; show L3 available
			
ciupdateav:
			stw		r8,pfAvailable(r2)			; store Available
			mtsprg	2,r8						; and cache it
			
			blr									; Return...
