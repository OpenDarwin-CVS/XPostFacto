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

use-nvramrc? true
load-base 600000
diag-device 
nvramrc hex\
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
: ck 0 do swap dup 3 >> km + c@ 1 rot 7 and << and or loop ;\
: bootr 0d word count encode-string " machargs" $a\
0 0 1 ck if 0 and else dup 1 = if 3d 0 1 else f 3d 0 2 then ck if 40 or then then\
40 and if bye else 1e 0 do ['] boot catch drop 1f4 ms loop then bye ;\
: myboot boot-command eval ;\
dev scsi\
: $M ['] open 888 - + ;\
: $M1 -E48 $M $X ;\
: $M2 begin 1 ms $M1 1 and -1068 $M $X or until $M1 case 0 of -1 endof 1 of 1 -E08 $M $X false endof dup endcase ;\
: $M3 -F68 $M f over $X $X ;\
: $M4 1 ms ;\
$E\
unselect-dev \
cpoke 0a7 0f3000032 \
cpoke 093 0f3000033 \
cpoke 03e 0f300003a
