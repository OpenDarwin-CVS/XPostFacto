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
#include <IOKit/IOMessage.h>
#include <ApplicationServices/ApplicationServices.h>

#include "XPFSyncPaths.h"

#define XPF_SECONDS_TO_SLEEP 12

// ================
// Global variables
// ================

typedef struct XPFBootFile {
	char* path;
	struct timespec mtime;
} XPFBootFile;

typedef struct XPFDeviceFiles {
	XPFBootFile kernel;
	XPFBootFile extensions;
	XPFBootFile extensionsCache;
} XPFDeviceFiles;

XPFDeviceFiles rootDeviceFiles;
XPFDeviceFiles bootDeviceFiles;

int reinitializeNow = 0;
int noSyncRequired = 0;

// ===================
// Function prototypes
// ===================

void initialize_everything ();

void writePID ();
void deletePID ();
void handle_SIGHUP (int sig);
void restartProcess ();

void pollForChanges ();
void restartWhenSignalled ();

void statDeviceFiles (XPFDeviceFiles *deviceFiles);
void statBootFile (XPFBootFile *bootFile);
void checkForChangesBetween (XPFBootFile *bf1, XPFBootFile *bf2);

void exitWithError (OSStatus err, int code);
void turnOffSleepIfUnsupported ();
void setupRestartInMacOS9 ();

void
setupRestartInMacOS9 ()
{
	if (noSyncRequired) return;

	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_registry_entry_t options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	if (options) {
		CFTypeRef keys[3] = {CFSTR ("boot-device"), CFSTR ("boot-command"), CFSTR ("boot-file")};
		CFTypeRef values[3] = {CFSTR ("/AAPL,ROM"), CFSTR ("boot"), CFSTR ("")};
		CFDictionaryRef dict = CFDictionaryCreate (kCFAllocatorDefault, keys, values, 3, 
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (dict) {
			IORegistryEntrySetCFProperties (options, dict);
			CFRelease (dict);
			noSyncRequired = true;
		}
		IOObjectRelease (options);
	}
	
	CFURLRef bundle = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem"), kCFURLPOSIXPathStyle, true);

	CFURLRef icon = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem/Resources/XPFIcons.icns"), kCFURLPOSIXPathStyle, false);
	
	CFUserNotificationDisplayNotice (0, kCFUserNotificationCautionAlertLevel, 
		icon, NULL, bundle, 
		CFSTR ("XPF Synchronization Required"),
		CFSTR ("XPFSyncMessage"),
		NULL);
		
	if (bundle) CFRelease (bundle);
	if (icon) CFRelease (icon);
}

void
turnOffSleepIfUnsupported ()
{
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_service_t platformDevice = IOServiceGetMatchingService (iokitPort, IOServiceMatching ("IOPlatformExpertDevice"));
	if (platformDevice) {
		io_iterator_t iterator;
		IORegistryEntryGetChildIterator (platformDevice, kIOServicePlane, &iterator);
		if (iterator) {
			io_object_t child;
			while ((child = IOIteratorNext (iterator))) {
				if (IOObjectConformsTo (child, "ApplePowerSurgePE") || IOObjectConformsTo (child, "ApplePowerStarPE")) system ("/usr/bin/pmset sleep 0");
				IOObjectRelease (child);
			}
			IOObjectRelease (iterator);
		}
		IOObjectRelease (platformDevice);
	}
}

void 
exitWithError (OSStatus err, int code)
{
	if (err) {
		syslog (LOG_ERR, "Error %d at code %d", err, code);
		exit (err);
	}
}

void
initialize_everything ()
{
	int err;
	char rootDevicePath[256], bootDevicePath[256];
	
	err = getPaths (rootDevicePath, bootDevicePath); if (err) exitWithError (err, 1);
	
	noSyncRequired = !strcmp (rootDevicePath, bootDevicePath);
	
	syslog (LOG_INFO, "rootDevicePath = %s", rootDevicePath);
	syslog (LOG_INFO, "bootDevicePath = %s", bootDevicePath);
	
	// Fill in the paths to each of the files/folders we want to track
	
	asprintf (&rootDeviceFiles.kernel.path, "%s/mach_kernel", rootDevicePath);
	asprintf (&rootDeviceFiles.extensions.path, "%s/System/Library/Extensions", rootDevicePath);
	asprintf (&rootDeviceFiles.extensionsCache.path, "%s/System/Library/Extensions.mkext", rootDevicePath);
	
	asprintf (&bootDeviceFiles.kernel.path, "%s/mach_kernel", bootDevicePath);
	asprintf (&bootDeviceFiles.extensions.path, "%s/System/Library/Extensions", bootDevicePath);
	asprintf (&bootDeviceFiles.extensionsCache.path, "%s/System/Library/Extensions.mkext", bootDevicePath);
	
	// Fill in the "helper" stat stuff. When someone writes to the helper, it's their 
	// responsiblity to restart us.
	
	statDeviceFiles (&bootDeviceFiles);
	
	reinitializeNow = 0;	
}

void
statDeviceFiles (XPFDeviceFiles *deviceFiles)
{
	statBootFile (&deviceFiles->kernel);
	statBootFile (&deviceFiles->extensionsCache);
	statBootFile (&deviceFiles->extensions);
}

void
statBootFile (XPFBootFile *bootFile)
{
	struct stat sb;
	int err = stat (bootFile->path, &sb);
	if (err) {
		bootFile->mtime.tv_sec = 0;
	} else {
		bootFile->mtime.tv_sec = sb.st_mtimespec.tv_sec;
	}
}

void
writePID ()
{
	FILE *fp;

	fp = fopen ("/var/run/xpfbootsupportd.pid", "w");
	if (fp != NULL) {
		fprintf (fp, "%d\n", getpid ());
		fclose (fp);
	}
}

void
deletePID ()
{
	if (rootDeviceFiles.kernel.path) free (rootDeviceFiles.kernel.path);
	if (rootDeviceFiles.extensions.path) free (rootDeviceFiles.extensions.path);
	if (rootDeviceFiles.extensionsCache.path) free (rootDeviceFiles.extensionsCache.path);
	
	if (bootDeviceFiles.kernel.path) free (bootDeviceFiles.kernel.path);
	if (bootDeviceFiles.extensions.path) free (bootDeviceFiles.extensions.path);
	if (bootDeviceFiles.extensionsCache.path) free (bootDeviceFiles.extensionsCache.path);

	unlink ("/var/run/xpfbootsupportd.pid");
}

void 
handle_SIGHUP (int sig)
{
	reinitializeNow = 1;
}

void
restartProcess ()
{
	execl ("/Library/StartupItems/XPFStartupItem/xpfbootsupportd", "xpfbootsupportd", NULL);
	// this shouldn't return, so error if it does
	syslog (LOG_ERR, "Error from execl: %m");
	exit (errno);
}

void
restartWhenSignalled ()
{
	sigset_t allsigs;
	sigemptyset (&allsigs);
		
	while (!reinitializeNow) sigsuspend (&allsigs);
	restartProcess ();
}

void
checkForChangesBetween (XPFBootFile *bf1, XPFBootFile *bf2)
{
	if (bf1->mtime.tv_sec != bf2->mtime.tv_sec) setupRestartInMacOS9 ();
}

void
pollForChanges ()
{
	while (!noSyncRequired) {			
		sleep (XPF_SECONDS_TO_SLEEP);
		
		if (reinitializeNow) restartProcess ();
		
		statDeviceFiles (&rootDeviceFiles);
		
		checkForChangesBetween (&rootDeviceFiles.kernel, &bootDeviceFiles.kernel);
		checkForChangesBetween (&rootDeviceFiles.extensions, &bootDeviceFiles.extensions);
		checkForChangesBetween (&rootDeviceFiles.extensionsCache, &bootDeviceFiles.extensionsCache);
	}
}

int
main (int argc, char **argv)
{	
	turnOffSleepIfUnsupported ();

	daemon (0, 0);
	writePID ();
	atexit (deletePID);
		
	openlog ("xpfbootsupportd", 0, LOG_USER);
	
	initialize_everything ();
	
	signal (SIGHUP, &handle_SIGHUP);
	
	pollForChanges ();
	
	restartWhenSignalled ();
	
    return 0;
}
