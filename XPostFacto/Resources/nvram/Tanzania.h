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
use-nvramrc? true
load-base 600000
diag-device 
nvramrc hex\
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
10 buffer: km\
dev kbd\
get-key-map km swap move\
$E\
: ck 0 do swap dup 3 >> km + c@ 1 rot 7 and << and or loop ;\
: bootr 0d word count encode-string " machargs" $a\
0 0 1 ck if 0 and else dup 1 = if 3d 0 1 else f 3d 0 2 then ck if 40 or then then\
40 and if bye else 1e 0 do ['] boot catch drop 3c8 ms loop then bye ;\
: myboot boot-command eval ;\
unselect-dev
