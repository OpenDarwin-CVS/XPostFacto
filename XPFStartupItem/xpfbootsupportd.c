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
void handle_SIGUSR1 (int sig);
void restartProcess ();

void pollForChanges ();
void restartWhenSignalled ();

void statDeviceFiles (XPFDeviceFiles *deviceFiles);
void statBootFile (XPFBootFile *bootFile);
void checkForChangesBetween (XPFBootFile *bf1, XPFBootFile *bf2);

void exitWithError (OSStatus err, int code);
void turnOffSleepIfUnsupported ();
void setupRestartInMacOS9 ();

int getPaths (char *rootDevicePath, char *bootDevicePath);
int getMountPointForOFPath (char *mountPoint, const char *ofPath);

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
	CFMutableDictionaryRef properties = NULL;
	io_registry_entry_t options = NULL;
	mach_port_t iokitPort;
	int err;
	
	bootDeviceOFPath[0] = 0;
	rootDeviceOFPath[0] = 0;
	
	// Get the boot-device and boot-args
	
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	if (!options) return 1;
	
	IORegistryEntryCreateCFProperties (options, &properties, kCFAllocatorDefault, kNilOptions);
	if (!properties) return 2;
	
	CFStringRef bootDevice = CFDictionaryGetValue (properties, CFSTR ("boot-device"));
	CFStringRef rootCommand = CFDictionaryGetValue (properties, CFSTR ("boot-command"));
	
	if (bootDevice) CFStringGetCString (bootDevice, bootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	if (rootCommand) CFStringGetCString (rootCommand, rootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	
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
	
	err = getMountPointForOFPath (rootDevicePath, rootDeviceOFPath); 
	if (err == noErr) err = getMountPointForOFPath (bootDevicePath, bootDeviceOFPath); 
	if (err) return err;
	
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
	
	if (properties) CFRelease (properties);
	if (rootCommand) CFRelease (rootCommand);
	if (bootDevice) CFRelease (bootDevice);
	
	if (options) IOObjectRelease (options);
	
	return 0;
}

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
	rootDevicePath[0] = 0;
	bootDevicePath[0] = 0;
	
	err = getPaths (rootDevicePath, bootDevicePath); 
	if (err) {
		noSyncRequired = true;
		syslog (LOG_INFO, "Could not get device paths");
	} else {
		noSyncRequired = !strcmp (rootDevicePath, bootDevicePath);
		syslog (LOG_INFO, "rootDevicePath = %s", rootDevicePath);
		syslog (LOG_INFO, "bootDevicePath = %s", bootDevicePath);
	}
			
	if (!noSyncRequired) {	
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
	}
	
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
handle_SIGUSR1 (int sig)
{
	noSyncRequired = true;
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
	signal (SIGUSR1, &handle_SIGUSR1);
	
	pollForChanges ();
	
	restartWhenSignalled ();
	
    return 0;
}
