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
 *	unsigned OWCCacheInit (unsigned)
 *
 *  Initialize the L2CR to the specified value. Returns the new L2CR value.
 *
 */

;			Force a line boundry here
			.align	5
			.globl	EXT(OWCL2CacheInit)

LEXT(OWCL2CacheInit)

; This is largely based on the cacheInit routine from Darwin. But I have simplified
; it somewhat, fixed some bugs, and added "test" code.

; r3 = L2CR to set
; return value is:
;	1 (for unsupported pvr)
;	the new L2CR (for success)
;	how much cache ram tested good (for test mode)

			mfspr	r11,pvr							; get the processor version
			rlwinm	r11,r11,16,16,31				; just keep the most significant part
			
			cmplwi	r11,PROCESSOR_VERSION_750
			beq		doSetup
			cmplwi	r11,PROCESSOR_VERSION_7400
			beq		doSetup
			cmplwi	r11,PROCESSOR_VERSION_7410
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
			
			cmplwi	r11,PROCESSOR_VERSION_750		; only do dssall if appropriate
			beq		cinoDSS
			cmplwi	r11,PROCESSOR_VERSION_750FX
			beq		cinoDSS
			
			dssall									; Stop streams
			sync
			
cinoDSS:

			mtmsr	r5								; Translation and all off
			sync
			isync									; Toss prefetch

			mfspr	r8,l2cr							; Get the L2CR
			rlwinm.	r2,r8,0,l2e,l2e					; see if it is enabled
			beq		ciinvdl2						; if not, no reason to flush
			
;
; flush the L2 Cache
;

ciflushl2:			

												; get the L2 cache size
												; This snippet is copied from start.s
										
			lis		r0,hi16(256*1024)			; Base L2 size
			addis	r10,r8,0x3000				; Hah... Figure this one out...
			rlwinm	r10,r10,4,30,31						 
			slw		r0,r0,r10					; Set 256KB, 512KB, 1MB, or 2MB

			li		r10,1024					; Get the L1 data size. It is always 32K for the moment.
			slwi	r10,r10,5					; i.e. multiply by 32
			add		r0,r0,r10					; add them together
			
			oris	r2,r8,hi16(l2dom)			; Set L2 to data only mode

			b		ciswfl2doa					; Branch to next line...

			.align  5
ciswfl2doc:
			mtspr	l2cr,r2						; Data only mode for L2
			sync
			isync
			b		ciswfl2dod					; It is off, go flush it...
ciswfl2doa:
			b		ciswfl2dob					; Branch to next...
ciswfl2dob:
			sync								; Finish memory stuff
			isync								; Stop speculation
			b		ciswfl2doc					; Jump back up and turn on data only...
			
ciswfl2dod:
			rlwinm	r0,r0,27,5,31					; Get the number of lines
			lis		r10,0xFF80						; Dead recon ROM for now
			mtctr	r0								; Set the number of lines
			
ciswfldl2a:	lwz		r0,0(r10)						; Load something to flush something
			addi	r10,r10,32						; Next line
			bdnz	ciswfldl2a						; Do the lot...

;
;	invalidate L2 cache
;
			
ciinvdl2:	

			rlwinm.	r8,r3,0,l2ts,l2ts				; are we in test mode?
			beq		ciskipdo						; if not, skip setting data-only
			oris	r3,r3,hi16(l2dom)				; make sure dom is set as well
			
ciskipdo:	
			rlwinm	r8,r3,0,l2e+1,31				; Use the desired L2CR and clear the enable bit
			b		cinla							; Branch to next line...

			.align  5
cinlc:		mtspr	l2cr,r8							; Disable L2
			sync
			isync
			b		ciinvl2							; It is off, go invalidate it...
cinla:		b		cinlb							; Branch to next...
cinlb:		sync									; Finish memory stuff
			isync									; Stop speculation
			b		cinlc							; Jump back up and turn off cache...
			
ciinvl2:	sync
			isync

			cmplwi	r3,0							; Should the L2 be all the way off?
			beq		cinol2							; Yes, done with L2

			oris	r2,r8,hi16(l2im)				; Get the invalidate flag set
			sync
			mtspr	l2cr,r2							; Start the invalidate
			sync
			isync
	
ciinvdl2a:	mfspr	r2,l2cr							; Get the L2CR
			rlwinm.	r2,r2,0,l2ip,l2ip				; Is the invalidate still going?
			bne+	ciinvdl2a						; Assume so, this will take a looong time...
											
			sync
			mtspr	l2cr,r8							; Turn off the invalidate request
						
cinol2:				
			cmplwi	r3,0							; Should the L2 be all the way off?
			beq		cinoexit						; Yes, done with L2
			mtspr	l2cr,r3							; Enable it as desired
					
;
;	Execute the test (if desired)
;
	
			rlwinm.	r2,r3,0,l2ts,l2ts				; Are we in test mode?
			beq		cinoexit						; If not, we are done
		
;
;	Write test values to the cache.
;
		
													; get the L2 cache size
													; This snippet is copied from start.s
										
			lis		r0,hi16(256*1024)				; Base L2 size
			addis	r10,r3,0x3000					; Hah... Figure this one out...
			rlwinm	r10,r10,4,30,31						 
			slw		r0,r0,r10						; Set 256KB, 512KB, 1MB, or 2MB
			rlwinm	r0,r0,27,5,31					; Get the number of lines
			lis		r10,0xFF80						; Dead recon ROM for now
			mtctr	r0								; Set the number of lines
			
citestwrl2:	
			dcbz	0,r10							; write zero to the cache line
			sync
			stw		r10,0(r10)						; write the address to the cache line
			sync
			dcbf	0,r10							; flush the write to the L2 cache
			sync
			addi	r10,r10,32						; Next line
			bdnz	citestwrl2						; Do the lot...
			
;
;	Read the test values back from the cache.
;
			
			li		r4,0							; counter for the "successful" lines
			lis		r10,0xFF80						; Dead recon ROM for now
			mtctr	r0								; set the number of lines
			
citestrdl2:
			lwz		r8,0(r10)						; load from the cache (maybe)
			cmpw	r8,r10							; see if we got the right value
			bne		citestfail						; branch around the increment
			addi	r4,r4,1							; increment the "success" line counter
citestfail:	
			sync
			dcbi	0,r10							; invalidate as we go
			sync
			addi	r10,r10,32						; next line
			bdnz	citestrdl2						; do the lot

			slwi	r3,r4,5							; return how much SRAM tested OK (multiply by 32)
		
;
;	Clean up from the test. Basically, just turn off the cache. 
;	Everything should be invalid at this stage.
;

			li		r8,0
			b		ctsla
			
			.align  5
ctslc:		mtspr	l2cr,r8							; Disable L2
			sync
			isync
			b		cinoexit						; It is off
ctsla:		b		ctslb							; Branch to next...
ctslb:		sync									; Finish memory stuff
			isync									; Stop speculation
			b		ctslc							; Jump back up and turn off cache...

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
			
			rlwinm	r8,r8,0,pfL2b+1,pfL2b-1		; show L2 not available
			rlwinm.	r5,r4,0,l2e,l2e				; see if it is enabled now
			beq		cistore	
			
			ori		r8,r8,pfL2					; show L2 available
			
			lis		r0,hi16(256*1024)			; Base L2 size
			addis	r10,r4,0x3000				; Hah... Figure this one out...
			rlwinm	r10,r10,4,30,31						 
			slw		r0,r0,r10					; Set 256KB, 512KB, 1MB, or 2MB
			
			stw		r0,pfl2Size(r2)				; Store the L2 size
			
			lwz		r5,pfl2crOriginal(r2)		; get the original cache setting
			rlwinm.	r5,r5,0,l2e,l2e				; see if it was enabled
			bne		cistore						; skip setting it if it was
			stw		r4,pfl2crOriginal(r2)		; store it
									
cistore:
			stw		r4,pfl2cr(r2)				; store the L2CR
			stw		r8,pfAvailable(r2)			; store Available
			mtsprg	2,r8						; and cache it
			
			blr									; Return...

