/*

    Copyright (c) 2003, 2005
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
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/attr.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

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
	XPFBootFile mountPoint;
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
int setupRestartInMacOS9OldWorld ();
int setupRestartInMacOS9NewWorld ();

void reblessMacOS9SystemFolder ();
bool isOldWorld ();
bool isMacOS9SystemFolder (char *path);
int asprintfcompat (char **buffer, const char *format, char *string);

int getPaths (char *rootDevicePath, char *bootDevicePath);
int getMountPointForOFPath (char *mountPoint, const char *ofPath);

struct volinfobuf {
  uint32_t info_length;
  uint32_t finderinfo[8];
}; 

struct fileinfobuf {
  uint32_t info_length;
  uint32_t finderinfo[8];
}; 

struct objectinfobuf {	
	uint32_t info_length;
	fsobj_id_t dirid;
};

// asprintfcompat because 10.1 doesn't have asprintf

int
asprintfcompat (char **buffer, const char *format, char *string)
{
	int retVal;
	char temp[1024];
	temp[0] = 0;
	retVal = sprintf (temp, format, string);
	*buffer = malloc (strlen (temp) + 1);
	strcpy (*buffer, temp);
	return retVal;
}

bool
isOldWorld ()
{
	static bool retVal = true;
	static bool cached = false;
	
	if (cached) return retVal;
	
	cached = true;
	bool isEmulatingNewWorld = false;

	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_service_t patchedAppleNVRAM = NULL; 
	io_iterator_t iter = NULL;
	
	IOServiceGetMatchingServices (iokitPort, IOServiceMatching ("IONVRAMController"), &iter);
	if (iter) {
		patchedAppleNVRAM = IOIteratorNext (iter);
		IOObjectRelease (iter);
	}
	
	if (patchedAppleNVRAM) {
		CFTypeRef emulatingNewWorld = IORegistryEntryCreateCFProperty (patchedAppleNVRAM, CFSTR("EmulatingNewWorld"), NULL, 0);
		if (emulatingNewWorld) {
			isEmulatingNewWorld = true;
			retVal = true;
			CFRelease (emulatingNewWorld);
		}
		IOObjectRelease (patchedAppleNVRAM);
	}

	if (!isEmulatingNewWorld) {
		UInt32 epoch;
		size_t epochlen = sizeof (epoch);
		int err = sysctlbyname ("hw.epoch", &epoch, &epochlen, NULL, NULL);
		if (err == noErr) retVal = (epoch == 0);
	}
	
	return retVal;
}

bool
isMacOS9SystemFolder (char *path)
{
	if (chdir (path)) return false;
	bool hasSystem = false;
	bool hasFinder = false;
	struct dirent *entry;
	DIR *dir = opendir (".");
	if (!dir) return false;
	
	while ((entry = readdir (dir)) && (!hasSystem || !hasFinder)) {
		struct attrlist	alist;
		struct fileinfobuf finfo;
		int err;
	
		alist.bitmapcount = 5;
		alist.reserved = 0;
		alist.commonattr = ATTR_CMN_FNDRINFO;
		alist.volattr = 0;
		alist.dirattr = 0;
		alist.fileattr = 0;
		alist.forkattr = 0;

		err = getattrlist (entry->d_name, &alist, &finfo, sizeof (finfo), 0);
		if (err) continue;
		
		if ((finfo.finderinfo[0] == 'FNDR') && (finfo.finderinfo[1] == 'MACS')) hasFinder = true;
		if ((finfo.finderinfo[0] == 'zsys') && (finfo.finderinfo[1] == 'MACS')) hasSystem = true;
	}

    closedir (dir);
	return hasSystem && hasFinder;
}

unsigned
getMacOS9SystemFolderNodeID (unsigned hint)
{
	unsigned retVal = 0;
	struct dirent *entry;
	DIR *dir = opendir ("/");
	
	while ((entry = readdir (dir)) && (retVal == 0)) {
		chdir ("/");
		if (isMacOS9SystemFolder (entry->d_name)) {
			chdir ("/");
			
			struct attrlist alist;
			struct objectinfobuf attrbuf;

			alist.bitmapcount = 5;
			alist.reserved = 0;
			alist.commonattr = ATTR_CMN_OBJID;
			alist.volattr = 0;
			alist.dirattr = 0;
			alist.fileattr = 0;
			alist.forkattr = 0;

			OSErr err = getattrlist (entry->d_name, &alist, &attrbuf, sizeof (attrbuf), 0);
			if (err == noErr) retVal = attrbuf.dirid.fid_objno;
			
			if (hint && (hint == retVal)) break;
		}
	}

    closedir (dir);
	return retVal;	
}

void
reblessMacOS9SystemFolder ()
{
	// If we're not on an Old World system, then we don't want to unbless CoreServices,
	// since Mac OS X actually uses the BootX from CoreServices on New World
	if (!isOldWorld ()) return;

    struct attrlist alist;
    struct volinfobuf vinfo;
	int err;

    alist.bitmapcount = 5;
    alist.reserved = 0;
    alist.commonattr = ATTR_CMN_FNDRINFO;
    alist.volattr = ATTR_VOL_INFO;
    alist.dirattr = 0;
    alist.fileattr = 0;
    alist.forkattr = 0;

	err = getattrlist ("/", &alist, &vinfo, sizeof (vinfo), 0);
    if (err) {
		syslog (LOG_INFO, "Could not get finder info");
        return;
    }
	
	syslog (LOG_INFO, "FinderInfo [0]: %lu [3]: %lu [5]: %lu", vinfo.finderinfo[0], vinfo.finderinfo[3], vinfo.finderinfo[5]);

	// Documentation of the meaning of the finderinfo values is here:
	// http://developer.apple.com/technotes/tn/tn1150.html

	// We only want to deal with cases where Mac OS X has deblessed the Mac OS 9 system folder
	// In those cases, Mac OS X sets finderinfo[5] equal to finderinfo[0]
	if (vinfo.finderinfo[0] != vinfo.finderinfo[5]) return;
	
	// We look for a system folder. We provide finderinfo[3] as a hint.
	
	UInt32 systemFolder = getMacOS9SystemFolderNodeID (vinfo.finderinfo[3]);
	
	// If we can't find a system folder, then we should set finderinfo[0] equal to
	// finderinfo[3]. That way, we do no harm if our routine for detecting system folders
	// misses one for some reason (since the previous system folder setting should still be
	// in finderinfo[3]). This is probably better than leaving finderinfo[0] with
	// the CoreServices directory id, because that won't ever do any good on Old World machines,
	// and can be problematic in some cases if you try to reboot after zapping PRAM.
		
	if (!systemFolder) systemFolder = vinfo.finderinfo[3];

	syslog (LOG_INFO, "Reblessing Mac OS 9 System Folder: %lu", systemFolder);
	vinfo.finderinfo[0] = systemFolder;
	vinfo.finderinfo[3] = systemFolder;
	err = setattrlist ("/", &alist, &vinfo.finderinfo, sizeof (vinfo.finderinfo), 0);
	if (err) syslog (LOG_INFO, "Error setting finder info");
}

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
	CFStringRef rootCommand = CFDictionaryGetValue (properties, CFSTR ("boot-args"));
	
	if (bootDevice) CFStringGetCString (bootDevice, bootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	if (rootCommand) CFStringGetCString (rootCommand, rootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	
	// We break the boot-device path at a comma-backslash, in case there is a file specifier in the path
	// This would be true on New World machines, which specify a path to BootX
	
	char *comma = strstr (bootDeviceOFPath, ",\\");
	if (comma) *comma = 0;
	
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

	if (err == noErr) {
		if (strcmp (rootDevicePath, bootDevicePath)) {
			pos = rootDeviceOFPath;
			while (*pos != 0) {
				if (*pos == ':') *pos = ';';
				pos++;
			}
			strcat (bootDevicePath, "/.XPostFacto/");
			strcat (bootDevicePath, rootDeviceOFPath);
		}
	}
	
	if (properties) CFRelease (properties);
	
	if (options) IOObjectRelease (options);
	
	return err;
}

int
setupRestartInMacOS9OldWorld ()
{
	// On Old World, we rewrite the NVRAM to boot in Mac OS 9. Unfortunately, we can't specify
	// which Mac OS 9 disk to boot from. (The mechanism for doing so requires talking to the
	// Mac OS 9 drivers, which of course we can't do.)
	
	// The alternative would be to find a different Mac OS X disk to boot from (presumably one
	// that doesn't require a helper, or is sync'd up). But we can generally rely on there being
	// a bootable Mac OS 9 volume around, whereas a second bootable Mac OS X volume is less likely.
	// And the user interface works well in XPostFacto when we've switched to OS 9 (since it will
	// automatically switch back to OS X).
	
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_registry_entry_t options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	
	if (options) {
		CFTypeRef keys[3] = {
			CFSTR ("boot-device"), 
			CFSTR ("boot-command"), 
			CFSTR ("boot-file")
		};
		CFTypeRef values[3] = {
			CFSTR ("/AAPL,ROM"), 
			CFSTR ("boot"), 
			CFSTR ("")
		};
		
		CFDictionaryRef dict = CFDictionaryCreate (kCFAllocatorDefault, keys, values, 3, 
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (dict) {
			IORegistryEntrySetCFProperties (options, dict);
			CFRelease (dict);
		}
		IOObjectRelease (options);
	}
	
	syslog (LOG_INFO, "Setting reboot to Mac OS 9 because synchronization required.");	
	
	CFURLRef bundle = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem"), kCFURLPOSIXPathStyle, true);
	CFURLRef icon = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem/Contents/Resources/XPFIcons.icns"), kCFURLPOSIXPathStyle, false);
	
	CFUserNotificationDisplayNotice (0, kCFUserNotificationCautionAlertLevel, 
			icon, NULL, bundle, 
			CFSTR ("XPF Synchronization Required"),
			CFSTR ("XPostFacto has noticed changes in progress to your system files. Once these changes have completed, you can launch XPostFacto synchronize the changes with your helper disk. Otherwise, your computer will restart in Mac OS 9 to avoid a mismatch between the system files on your root disk and helper disk."),
			NULL);
		
	if (bundle) CFRelease (bundle);
	if (icon) CFRelease (icon);

	return 0;
}

int
setupRestartInMacOS9NewWorld ()
{
	// On New World machines, we need to find a volume that can boot Mac OS 9.
	// Unlike on Old World, we need to specify a particular volume and bless the System Folder.
	
	// FIXME -- need to actually write this! For the moment, I'm simply warning the user and
	// not actually changing anything.
	
	syslog (LOG_INFO, "XPostFacto synchronization required.");	
	
	CFURLRef bundle = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem"), kCFURLPOSIXPathStyle, true);
	CFURLRef icon = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR ("/Library/StartupItems/XPFStartupItem/Contents/Resources/XPFIcons.icns"), kCFURLPOSIXPathStyle, false);
	
	CFUserNotificationDisplayNotice (0, kCFUserNotificationCautionAlertLevel, 
			icon, NULL, bundle, 
			CFSTR ("XPF Synchronization Required"),
			CFSTR ("XPostFacto has noticed changes in progress to your system files. Once these changes have completed, you can launch XPostFacto synchronize the changes with your helper disk. If you cannot launch XPostFacto, then reboot from a different volume (or in Mac OS 9) and run XPostFacto from there."),
			NULL);
		
	if (bundle) CFRelease (bundle);
	if (icon) CFRelease (icon);

	return -1;
}

void
setupRestartInMacOS9 ()
{
	if (noSyncRequired) return;
	noSyncRequired = true;
	
	if (isOldWorld ()) {
		setupRestartInMacOS9OldWorld ();
	} else {
		setupRestartInMacOS9NewWorld ();
	}
}

void
turnOffSleepIfUnsupported ()
{
	mach_port_t iokitPort;
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	io_service_t platformDevice = NULL; 
	io_iterator_t iter = NULL;
	
	IOServiceGetMatchingServices (iokitPort, IOServiceMatching ("IOPlatformExpertDevice"), &iter);
	if (iter) {
		platformDevice = IOIteratorNext (iter);
		IOObjectRelease (iter);
	}

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
		asprintfcompat (&rootDeviceFiles.kernel.path, "%s/mach_kernel", rootDevicePath);
		asprintfcompat (&rootDeviceFiles.extensions.path, "%s/System/Library/Extensions", rootDevicePath);
		asprintfcompat (&rootDeviceFiles.extensionsCache.path, "%s/System/Library/Extensions.mkext", rootDevicePath);
		asprintfcompat (&rootDeviceFiles.mountPoint.path, "%s", rootDevicePath);
		
		asprintfcompat (&bootDeviceFiles.kernel.path, "%s/mach_kernel", bootDevicePath);
		asprintfcompat (&bootDeviceFiles.extensions.path, "%s/System/Library/Extensions", bootDevicePath);
		asprintfcompat (&bootDeviceFiles.extensionsCache.path, "%s/System/Library/Extensions.mkext", bootDevicePath);
		asprintfcompat (&bootDeviceFiles.mountPoint.path, "%s", bootDevicePath);
		
		// Fill in the "helper" stat stuff. When someone writes to the helper, it's their 
		// responsiblity to restart us.		
		statDeviceFiles (&bootDeviceFiles);
	}
	
	reinitializeNow = 0;	
}

void
statDeviceFiles (XPFDeviceFiles *deviceFiles)
{
	statBootFile (&deviceFiles->mountPoint);
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
	if (bf1->mtime.tv_sec != bf2->mtime.tv_sec) {
		syslog (LOG_INFO, "%s mtime %lu differs from %s mtime %lu\n", bf1->path, bf1->mtime.tv_sec, bf2->path, bf2->mtime.tv_sec);
		setupRestartInMacOS9 ();
	}
}

void
pollForChanges ()
{
	while (!noSyncRequired) {			
		sleep (XPF_SECONDS_TO_SLEEP);
		
		if (reinitializeNow) restartProcess ();
		
		statDeviceFiles (&rootDeviceFiles);
		
		// If the (proposed) root device is not mounted, then we skip the rest of the changes
		// We keep checking, though, in case it is remounted 
		if (rootDeviceFiles.mountPoint.mtime.tv_sec != 0) {
			checkForChangesBetween (&rootDeviceFiles.kernel, &bootDeviceFiles.kernel);
			checkForChangesBetween (&rootDeviceFiles.extensions, &bootDeviceFiles.extensions);
			checkForChangesBetween (&rootDeviceFiles.extensionsCache, &bootDeviceFiles.extensionsCache);
		}
	}
}

int
main (int argc, char **argv)
{	
	daemon (0, 0);
	writePID ();
	atexit (deletePID);
		
	openlog ("xpfbootsupportd", 0, LOG_USER);
	
	turnOffSleepIfUnsupported ();
	reblessMacOS9SystemFolder ();

	initialize_everything ();
	
	signal (SIGHUP, &handle_SIGHUP);
	signal (SIGUSR1, &handle_SIGUSR1);

	pollForChanges ();
	
	restartWhenSignalled ();
	
    return 0;
}
