/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as 
defined in and that are subject to the Apple Public Source License Version 2.0 
(the 'License').  You may not use this file except in compliance with the License.  
Please obtain a copy of the License at http://www.opensource.apple.com/apsl/ and 
read it before using this file. 

The Original Code and all software distributed under the License are distributed 
on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND 
APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT 
OR NON-INFRINGEMENT. Please see the License for the specific language governing 
rights and limitations under the License.

*/

#include "PatchedBlockStorageDriver.h"

#include <IOKit/IOLib.h>

static int VERS_parse_string (const char *vers_string, UInt32 *version_num);

#define super IOBlockStorageDriver

OSDefineMetaClassAndStructors(PatchedBlockStorageDriver, IOBlockStorageDriver);

IOService*
PatchedBlockStorageDriver::probe (IOService *provider, SInt32 *score)
{
	kmod_info_t *scsiAMF = kmod_lookupbyname ("com.apple.iokit.IOSCSIArchitectureModelFamily");
	if (!scsiAMF) return NULL;
	
	UInt32 targetVersion, installedVersion;
	VERS_parse_string ("1.3.3", &targetVersion);
	VERS_parse_string (scsiAMF->version, &installedVersion);
	if (installedVersion >= targetVersion) return NULL;
	
	return super::probe (provider, score);
}

IOReturn	
PatchedBlockStorageDriver::checkForMedia (void)
{
    IOReturn result;
    bool currentState;
    bool changed;
    
    IOLockLock (_mediaStateLock);    

    result = getProvider()->reportMediaState (&currentState, &changed);
	changed = currentState ? !_mediaPresent : _mediaPresent;
	
    if (result != kIOReturnSuccess) {		/* the poll operation failed */
        IOLog("%s[IOBlockStorageDriver]::checkForMedia; err '%s' from reportMediaState\n", getName(), stringFromReturn (result));
    } else if (changed) {	/* the poll succeeded, media state has changed */
        result = mediaStateHasChanged (currentState ? kIOMediaStateOnline : kIOMediaStateOffline);
    }

    IOLockUnlock (_mediaStateLock);
    return (result);
}

#undef super

#define super IOCDBlockStorageDriver

OSDefineMetaClassAndStructors(PatchedCDBlockStorageDriver, IOCDBlockStorageDriver);

IOService*
PatchedCDBlockStorageDriver::probe (IOService *provider, SInt32 *score)
{
	kmod_info_t *scsiAMF = kmod_lookupbyname ("com.apple.iokit.IOSCSIArchitectureModelFamily");
	if (!scsiAMF) return NULL;
	
	UInt32 targetVersion, installedVersion;
	VERS_parse_string ("1.3.3", &targetVersion);
	VERS_parse_string (scsiAMF->version, &installedVersion);
	if (installedVersion >= targetVersion) return NULL;
	
	return super::probe (provider, score);
}

IOReturn	
PatchedCDBlockStorageDriver::checkForMedia (void)
{
    IOReturn result;
    bool currentState;
    bool changed;
    
    IOLockLock (_mediaStateLock);    

    result = getProvider()->reportMediaState (&currentState, &changed);
	if (!changed) {
		changed = currentState ? !_mediaPresent : _mediaPresent;
		if (changed) setProperty ("Bug fix applied", "");
	}
	
    if (result != kIOReturnSuccess) {		/* the poll operation failed */
        IOLog("%s[IOBlockStorageDriver]::checkForMedia; err '%s' from reportMediaState\n", getName(), stringFromReturn (result));
    } else if (changed) {	/* the poll succeeded, media state has changed */
        result = mediaStateHasChanged (currentState ? kIOMediaStateOnline : kIOMediaStateOffline);
    }

    IOLockUnlock (_mediaStateLock);
    return (result);
}

// ****************
// Version checking
// ****************

#include <libkern/OSTypes.h>

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

static int VERS_parse_string(const char * vers_string, UInt32 * version_num);

#include <libkern/OSByteOrder.h>

static int isdigit(char c) {
    return (c == '0' ||
        c == '1' || c == '2' || c == '3' ||
        c == '4' || c == '5' || c == '6' ||
        c == '7' || c == '8' || c == '9');
}

static int isspace(char c) {
    return (c == ' ' ||
        c == '\t' ||
        c == '\r' ||
        c == '\n');
}

static int isreleasestate(char c) {
    return (c == 'd' || c == 'a' || c == 'b' || c == 'f');
}

static UInt8 BCD_digit_for_char(char c) {
    switch (c) {
      case '0': return 0; break;
      case '1': return 1; break;
      case '2': return 2; break;
      case '3': return 3; break;
      case '4': return 4; break;
      case '5': return 5; break;
      case '6': return 6; break;
      case '7': return 7; break;
      case '8': return 8; break;
      case '9': return 9; break;
      default:  return BCD_illegal; break;
    }
    return BCD_illegal;
}

static VERS_revision VERS_revision_for_string(const char ** string_p) {
    const char * string;

    if (!string_p || !*string_p) {
        return VERS_invalid;
    }

    string = *string_p;

    if (isspace(string[0]) || string[0] == '\0') {
        return VERS_release;
    } else {
        switch (string[0]) {
          case 'd':
              if (isdigit(string[1])) {
                  *string_p = &string[1];
                  return VERS_development;
              }
              break;
          case 'a':
              if (isdigit(string[1])) {
                  *string_p = &string[1];
                  return VERS_alpha;
              }
              break;
          case 'b':
              if (isdigit(string[1])) {
                  *string_p = &string[1];
                  return VERS_beta;
              }
              break;
          case 'f':
              if (isdigit(string[1])) {
                  *string_p = &string[1];
                  return VERS_candidate;
              } else if (string[1] == 'c' && isdigit(string[2])) {
                  *string_p = &string[2];
                  return VERS_candidate;
              } else {
                  return VERS_invalid;
              }
              break;
          default:
              return VERS_invalid;
              break;
        }
    }

    return VERS_invalid;
}


static int VERS_parse_string(const char * vers_string, UInt32 * version_num) {
    int result = 1;
    VERS_version vers;
    const char * current_char_p;
    UInt8  scratch;

    if (!vers_string || *vers_string == '\0') {
        return 0;
    }

    vers.vnum = 0;

    current_char_p = &vers_string[0];


   /*****
    * Check for an initial digit of the major release number.
    */
    vers.bytes[0] = BCD_digit_for_char(*current_char_p);
    if (vers.bytes[0] == BCD_illegal) {
        return 0;
    }

    current_char_p++;


   /*****
    * Check for a second digit of the major release number.
    */
    if (*current_char_p == '\0') {
        vers.bytes[2] = VERS_release;
        vers.bytes[3] = 0xff;
        goto finish;
    } else if (isdigit(*current_char_p)) {
        scratch = BCD_digit_for_char(*current_char_p);
        if (scratch == BCD_illegal) {
            return 0;
        }
        vers.bytes[0] = BCD_combine(vers.bytes[0], scratch);
        current_char_p++;

        if (*current_char_p == '\0') {
            vers.bytes[2] = VERS_release;
            vers.bytes[3] = 0xff;
            goto finish;
        } else if (isreleasestate(*current_char_p)) {
            goto release_state;
        } else if (*current_char_p == '.') {
            current_char_p++;
        } else {
            return 0;
        }
    } else if (isreleasestate(*current_char_p)) {
        goto release_state;
    } else if (*current_char_p == '.') {
        current_char_p++;
    } else {
        return 0;
    }


   /*****
    * Check for the minor release number.
    */
    if (*current_char_p == '\0') {
        vers.bytes[2] = VERS_release;
        vers.bytes[3] = 0xff;
        goto finish;
    } else if (isdigit(*current_char_p)) {
        vers.bytes[1] = BCD_digit_for_char(*current_char_p);
        if (vers.bytes[1] == BCD_illegal) {
            return 0;
        }

        // Make sure its the first nibble of byte 1!
        vers.bytes[1] = BCD_combine(vers.bytes[1], 0);

        current_char_p++;

        if (*current_char_p == '\0') {
            vers.bytes[2] = VERS_release;
            vers.bytes[3] = 0xff;
            goto finish;
        } else if (isreleasestate(*current_char_p)) {
            goto release_state;
        } else if (*current_char_p == '.') {
            current_char_p++;
        } else {
            return 0;
        }
    } else {
        return 0;
    }


   /*****
    * Check for the bugfix number.
    */
    if (*current_char_p == '\0') {
        vers.bytes[2] = VERS_release;
        vers.bytes[3] = 0xff;
        goto finish;
    } else if (isdigit(*current_char_p)) {
        scratch = BCD_digit_for_char(*current_char_p);
        if (scratch == BCD_illegal) {
            return 0;
        }

        /* vers.bytes[1] has its left nibble set already */
        vers.bytes[1] = vers.bytes[1] | scratch;

        current_char_p++;

        if (*current_char_p == '\0') {
            vers.bytes[2] = VERS_release;
            vers.bytes[3] = 0xff;
            goto finish;
        } else if (isreleasestate(*current_char_p)) {
            goto release_state;
        } else {
            return 0;
        }
    } else {
        return 0;
    }


release_state:

   /*****
    * Check for the release state.
    */
    if (*current_char_p == '\0') {
        vers.bytes[2] = VERS_release;
        vers.bytes[3] = 0xff;
        goto finish;
    } else {
        vers.bytes[2] = VERS_revision_for_string(&current_char_p);
        if (vers.bytes[2] == VERS_invalid) {
            return 0;
        }
    }


   /*****
    * Get the nonrelease revision number (0..255).
    */
    if (vers.bytes[2] != VERS_release) {
        UInt32 revision_num = 0;
        int    i;

        if (*current_char_p == '\0' || !isdigit(*current_char_p)) {
            return 0;
        }
        for (i = 0; i < 3 && *current_char_p != '\0'; i++, current_char_p++) {
            UInt8 scratch_digit;
            scratch_digit = BCD_digit_for_char(*current_char_p);
            if (scratch_digit == BCD_illegal) {
                return 0;
            }
            revision_num *= 10;
            revision_num += scratch_digit;
        }
        if (isdigit(*current_char_p) || revision_num > 255) {
            return 0;
        }
        vers.bytes[3] = (UInt8)revision_num;
    }

    if (vers.bytes[2] == VERS_release) {
        vers.bytes[3] = 0xff;
    } else {
        if (vers.bytes[2] == VERS_candidate) {
            if (vers.bytes[3] == 0) {
                return 0;
            } else {
                vers.bytes[2] = VERS_release;
                vers.bytes[3]--;
            }
        }
    }

finish:
    *version_num = OSSwapBigToHostInt32(vers.vnum);
    return result;
}


