##
# Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
# Reserved.  This file contains Original Code and/or Modifications of
# Original Code as defined in and that are subject to the Apple Public
# Source License Version 1.0 (the 'License').  You may not use this file
# except in compliance with the License.  Please obtain a copy of the
# License at http://www.apple.com/publicsource and read it before using
# this file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
# License for the specific language governing rights and limitations
# under the License."
# 
# @APPLE_LICENSE_HEADER_END@
##
# Version: 1.2.0  Date: 8-17-2000
use-nvramrc? true
load-base 600000
diag-device 
nvramrc hex\
code ictc! 7E9BFBA6 l, 829F0000 l, 3BFF0004 l, 4E800020 l, c;\
: $G FFFF0000 pvr@ and dup 80000 = swap C0000 = or ;\
: $I $G if ictc! else drop then ;\
: $D find-device ;\
: $E device-end ;\
: $L BLpatch ; : $R BRpatch ;\
: $X execute ;\
: $Xd execute-device-method ;\
: $p 0 to my-self property ;\
: $a " /chosen" $D $p $E ;\
10 buffer: km\
dev kbd\
get-key-map km swap move\
$E\
: patch " screen" " nopen" execute-device-method drop ;
: ck 0 do swap dup 3 >> km + c@ 1 rot 7 and << and or loop ;\
: bootr patch 0d word count encode-string " machargs" $a\
0 0 1 ck if 0 and else dup 1 = if 3d 0 1 else f 3d 0 2 then ck if 40 or then then\
40 and if bye else 11 $I 1e 0 do ['] boot catch ['] init-program catch ['] go catch\
drop 1f4 ms loop then 0 $I bye ;\
: myboot boot-command eval ;\
dev /packages/mac-parts\
: $M -7E9408 $X 8000 alloc-mem 7F00 + 4 -7E9408 $X ;\
' load 268 - ' $M $L\
' load 168 + ' 0 $L\
$E\
dev ata\
d encode-int 2 encode-int encode+ " AAPL,interrupts" $p\
$E\
: $s " screen" ;\
dev screen \
: $NN 280 1e0 over 14 - char-width / over 14 - char-height / fb8-install ;\
: $EE -7d78e0 $X 280 1e0 * ff filldefault-font set-font ;\
: $RR 280 #columns char-width * - 2/ to window-left 1e0 #lines char-height * - 2/ to window-top ; \
: $SP frame-buffer-adr encode-int " address" property \
280 encode-int " width" property \
1e0 encode-int " height" property \
8 encode-int " line-bytes" property ; \
: $WW -efff000 $X dup dup 280 1e0 * w___ do-map to frame-buffer-adr ['] $EE catch \
['] $NN catch \
['] $RR catch drop ['] $SP catch drop ; \
: nopen $WW 0 4B00 do-unmap ; \
$E\
unselect-dev\
: fix $s " $WW" $Xd ;
