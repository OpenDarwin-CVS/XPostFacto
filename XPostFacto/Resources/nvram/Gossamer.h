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
: $p 0 to my-self property ;\
: $a " /chosen" $D $p $E ;\
: &c " ata-enable" $call-parent ;\
10 buffer: km\
dev kbd\
get-key-map km swap move\
$E\
: ck 0 do swap dup 3 >> km + c@ 1 rot 7 and << and or loop ;\
: bootr 0d word count encode-string " machargs" $a\
0 0 1 ck if 0 and else dup 1 = if 3d 0 1 else f 3d 0 2 then ck if 40 or then then\
40 and if bye else 11 $I 1e 0 do ['] boot catch drop 1f4 ms loop then 0 $I bye ;\
: myboot boot-command eval ;\
dev enet\
' open constant $M\
: $M2 $M 710 - $X ;\
: rl@ -7D9D40 $X ;\
: chstat begin $M2 $M 14f8 - $X -7D6C20 $X rl@ 400 and 0= until ;\
: bmstat begin $M2 $M 13F0 - $X rl@ 100 and until ;\
: xmt1 get-msecs $M 720 - ! chstat $M A00 - $X bmstat chstat ;\
' xmt1 ' WRITE 10 + l!\
62 ' READ 7 - c!\
: READ { _p _n ; _a } begin _p _n bead -> _a _a 2+\
if _p c@ 80 and 0= else 1 then until _a ;\
$E\
dev /packages/obp-tftp\
: $M over + ['] noop $L ;\
: $O ['] open + ;\
: $M1 dup 24 - -1720 $O $X 6 move 14 + ;\
-5BC $O ' $M1 $L\
0 $O E8 $M EC $M F0 $M F4 $M F8 + ' true $L\
$E\
dev /packages/mac-parts\
: $M -7E89E0 $X 8000 alloc-mem 7F00 + 4 -7E89E0 $X ;\
' load 268 - ' $M $L\
' load 160 + ' 0 $L\
$E\
dev ide0\
: open use-ata-interface 0 &c -1 ;\
: set-device-ID set-drive-select ;\
: reset-atapi-bus reset-ata-bus ;\
' reset-ata-bus 2c + ' 2 $L\
$E\
dev ide1\
: open use-ata-interface 0 &c -1 ;\
: set-device-ID set-drive-select ;\
: reset-atapi-bus reset-ata-bus ;\
' reset-ata-bus 2c + ' 2 $L\
$E\
dev scsi\
: $M ['] do-cmd + ;\
: $M2 5 us -5f0 $M $X ;\
: $M3 -710 $M f over $X $X ;\
: $M4 1 ms ;\
-1AC $M ' $M2 $L\
100 $M ' $M3 $L\
120 $M ' $M4 $L\
124 $M ' 1 $L\
$E\
ff000000 dup dup 400 28 do-map 4+ w@ 10 and 0=\
if 90b7 f3000032 w! then\
unselect-dev
