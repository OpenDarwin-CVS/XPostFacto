/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc. All Rights
Reserved.

This file contains Original Code and/or Modifications of Original Code
as defined in and that are subject to the Apple Public Source License
Version 2.0 (the 'License'). You may not use this file except in
compliance with the License. Please obtain a copy of the License at
http://www.opensource.apple.com/apsl/ and read it before using this
file.

The Original Code and all software distributed under the License are
distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
Please see the License for the specific language governing rights and
limitations under the License.

*/

#ifndef __VERSRSRC_H__
#define __VERSRSRC_H__

typedef union {
    UInt32 vnum;
    UInt8  bytes[4];
} VERS_version;

typedef enum {
    VERS_development = 0x20,
    VERS_alpha       = 0x40,
    VERS_beta        = 0x60,
    VERS_candidate   = 0x70,  // for interim usage only!
    VERS_release     = 0x80,
    VERS_invalid     = 0xff
} VERS_revision;

#define BCD_combine(l, r)  ( (((l) & 0xf) << 4) | ((r) & 0xf) )
#define BCD_get_left(p)    ( ((p) >> 4) & 0xf )
#define BCD_get_right(p)   ( (p) & 0xf )

#define BCD_illegal  (0xff)   // full byte, 11111111

int VERS_parse_string(const char * vers_string, UInt32 * version_num);
int VERS_string(char * buffer, UInt32 length, UInt32 vers);

#endif _LIBSA_VERS_H_
