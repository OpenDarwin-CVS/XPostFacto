/*

 Copyright (c) 2003
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 */

#include "XPFSyncPaths.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

int 
getMountPointForOFPath (char *mountPoint, const char *ofPath)
{ 
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	
	int err = 1;
	char deviceBSDName[255];
	strcpy (deviceBSDName, "/dev/");
	IOServiceOFPathToBSDName (iokitPort, ofPath, deviceBSDName + 5);

	mountPoint[0] = 0;
	int numFS = getfsstat (NULL, 0, MNT_NOWAIT);
	struct statfs *fs = (struct statfs *) malloc (numFS * sizeof (struct statfs));
	getfsstat (fs, numFS * sizeof (struct statfs), MNT_NOWAIT);
	int x;
	for (x = 0; x < numFS; x++) {
		if (!strcmp (fs[x].f_mntfromname, deviceBSDName)) {
			strcpy (mountPoint, fs[x].f_mntonname);
			err = 0;
			break;
		}
	}		
	free (fs);
	
	if (err) syslog (LOG_ERR, "Could not find mountpoint for %s", ofPath);
	
	return err;
}

int
getPaths (char *rootDevicePath, char *bootDevicePath)
{
	char bootDeviceOFPath[256], rootDeviceOFPath[256];
	CFMutableDictionaryRef properties;
	io_registry_entry_t options;
	mach_port_t iokitPort;
	int err;
	
	// Get the boot-device and boot-args
	
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	if (!options) return 1;
	
	IORegistryEntryCreateCFProperties (options, &properties, kCFAllocatorDefault, kNilOptions);
	if (!properties) return 2;
	
	CFStringRef bootDevice = CFDictionaryGetValue (properties, CFSTR ("boot-device"));
	CFStringRef bootArgs = CFDictionaryGetValue (properties, CFSTR ("boot-args"));
	
	CFStringGetCString (bootDevice, bootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	CFStringGetCString (bootArgs, rootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	
	// The root-device is either found in the rd=* argument, or is equal to the boot-device
	
	char *pos = strstr (rootDeviceOFPath, "rd=*");
	if (pos) {
		pos += 4;
		char *endpos = strstr (pos, " ");
		if (endpos) *endpos = 0;
		memmove (rootDeviceOFPath, pos, strlen (pos) + 1);
	} else {
		strcpy (rootDeviceOFPath, bootDeviceOFPath);
	}
	
	// Get the mount point that corresponds to the root-device and boot-device
	
	err = getMountPointForOFPath (rootDevicePath, rootDeviceOFPath); if (err) return err;
	err = getMountPointForOFPath (bootDevicePath, bootDeviceOFPath); if (err) return err;
	
	// See if any synchronization is required. If so, construct the path to the helper files
	// on the boot device
	
	if (strcmp (rootDevicePath, bootDevicePath)) {
		pos = rootDeviceOFPath;
		while (*pos != 0) {
			if (*pos == ':') *pos = ';';
			pos++;
		}
		strcat (bootDevicePath, "/.XPostFacto/");
		strcat (bootDevicePath, rootDeviceOFPath);
	}
	
	CFRelease (properties);
	IOObjectRelease (options);
	
	return 0;
}