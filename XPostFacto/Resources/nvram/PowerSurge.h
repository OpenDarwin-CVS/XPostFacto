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
#
# Modified by ryan.rempel@utoronto.ca
# Disables speculative processing if a G3 or G4 processor is detected
# Removed patch for tftp to save space
# Added instruction cache throttling
##
# Version: 1.2.0  Date: 8-17-2000
use-nvramrc? true
load-base 600000
diag-device 
nvramrc hex\
code ictc! 7E9BFBA6 l, 829F0000 l, 3BFF0004 l, 4E800020 l, c;\
: $G FFFF0000 pvr@ and dup 80000 = swap C0000 = or ;\
: $I $G if ictc! else drop then ;\
: $C $call-method ;\
: $D find-device ;\
: $E device-end ;\
: $x execute ;\
: $F $D " open" $find drop ;\
: $p 0 to my-self property ;\
: $a " /chosen" $D $p $E ;\
: R BRpatch ; : L BLpatch ;\
: q over + ;\
: g code! ;\
: & get-token drop ;\
6ED & $x\
0 value mi\
: mmr " map-range" mi if my-self $C else $call-parent then ;\
89B & ' mmr R\
: mcm -1 to mi $C 0 to mi ;\
8CB & 1E na+ ' mcm L\
: maa -1 to mi 1D swap ;\
8C9 & 5 na+ ' maa L\
8C9 & 134 + ' 1 L\
8CD & 184 + 14 q dup @ 6 << 6 >>a -4 and + R\
8C6 & 7C + ' u< L\
0 value yn\
: y yn 0= if dup @ to yn then ;\
8CB & ' y R\
' y 28 + 8CB & 8 + R\
: z yn ?dup if over ! 0 to yn then ;\
8CC & ' z R\
' z 2C + 8CC & 8 + R\
@startvec BC + @ 40820014 over 88 + g 41820010 swap E0 + g\
0 @startvec 5C + @ 1D8 + g\
dev /packages/mac-parts\
400000 ' load 14 + g\
: m1 400000 do-unmap ;\
' load 8 + ' m1 L\
' load 160 + ' 0 L\
: &r1 4+ dup 8000 alloc-mem 7F00 + swap ! 4+ F8 ;\
' load 2AC - ' &r1 L\
$E\
4180FFF0 ' msr! 44 + g\
dev /packages/xcoff-loader\
: p&+ ['] open 600 - + ;\
: p1 { _a _s } _a -1000 and _a _s + over - FFF ;\
60000000 dup 8 p&+ g C p&+ g\
18 p&+ ' p1 L\
$E\
" enet" $F dup\
1D8 - 24 q ['] or L $x\
248 - @ 6 encode-bytes 2dup\
" local-mac-address" $p\
$E\
" mac-address" $a\
10 buffer: km\
dev kbd\
get-key-map km swap move\
$E\
: ck 0 do swap dup 3 >> km + c@ 1 rot 7 and << and or loop ;\
: bootr 0d word count encode-string " machargs" $a\
0 0 1 ck if 0 and else dup 1 = if 3d 0 1 else f 3d 0 2 then ck if 40 or then then\
40 and if bye else 11 $I 1e 0 do ['] boot catch drop 1f4 ms loop then 0 $I bye ;\
: &SI\
" scsi-int" open-dev\
" open" 2 pick 4+ @ find-method drop\
2c q ['] 2 L 848 -\
8 q 1C q R\
88 q 4 q R\
$x " close" rot $C ;\
&SI\
unselect-dev\
$G if hid0@ 208 or FFFFFF7F and hid0! then
