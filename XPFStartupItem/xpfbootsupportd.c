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

/*

XPostFacto 3 lets you boot from devices that aren't ordinarily bootable, by
setting things up so that the earliest part of the boot process is handled from
a "helper" disk that is bootable, but some other device is used as the root device.
This requires that some of the system software on the root device be copied to the
"helper" disk. If you install a new kernel extension, or a system software update,
it will go onto the root device, and something needs to copy it to the boot device.
So that is the purpose of this little daemon.

What it does is poll two files (/mach_kernel and /System/Library/Extensions.mkext),
as well as the Extensions folder (/System/Library/Extensions). The polling interval
is set by XPF_SECONDS_TO_SLEEP, and I will need to experiment a little to get that 
right.

If there were a reliable way to trigger an action at shutdown or restart time and
make sure that it will be given enough time to complete, then we wouldn't need to
continuously poll. But there doesn't appear to be such a mechanism, so we'll try
it this way.

Basically, we watch for the "modified time" of the files or directory to change.
Then, for the files, we wait until the modified time has stayed the same for
XPF_SECONDS_TO_SLEEP (the idea is to wait until someone is finished writing to the
file). 

For the extensions directory, it is a little more complex. It is quite possible to
change something in a sub-directory of the extensions directory without changing the 
modification time of the extensions directory itself. However, it seems a little
resource-intensive to continuously poll every file in every sub-directory. (I suppose
this may be a premature optimization, but there you are). Furthermore, the convention is 
that installers should "touch" the Extensions directory itself when installing something,
to signal that the Extensions.mkext should be updated. So it seems sufficient to poll
just the Extensions directory itself.

However, once we have detected a change in the modification date of the Extensions
directory, then we need to poll every sub-directory and every file to wait until
things are "stable" (i.e. to wait until the installer is finished). Then we can
do the copy.

The actually copying is being done via calls to ditto at the moment. This is kind of 
lazy, but I can write my own copying code later if it seems to matter. Also, I'm deleting
and recopying the entire Extensions directory at the moment. I really should change this,
as it could be done 

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

#define XPF_SECONDS_TO_SLEEP 5

struct stat 	sb;
struct timespec kernelMod;
struct timespec mkextMod;
struct timespec extfolderMod;	

int kernelChanging;
int mkextChanging;
int extfolderChanging;

int warnings = 0;
CFUserNotificationRef notification;
CFDictionaryRef warningDict;
CFDictionaryRef cancelWarningDict;

char bootDevicePath[255];
char rootDevicePath[255];

int reinitializeNow = 0;
int noSyncRequired = 0;

mach_port_t iokitPort;

void warn_the_user ();
void cancel_warning ();

int initialize_paths ();
void initialize_warning_strings ();
void initialize_everything_else ();

void handle_mkext_change (int hasChanged);
void handle_kernel_change (int hasChanged);
void handle_extensions_folder_change (int hasChanged);

int anything_changed_in_directory_since_time (char *path, struct timespec *sinceTime);
void most_recent_change_in_directory (char *path, struct timespec *changeTime);

void writePID ();
void handle_SIGHUP (int sig);

int getMountPointForOFPath (char *mountPoint, const char *ofPath);

// -----------------
//
// warn_the_user
//
// The idea is to warn the user when synchronization is in progress, so that they
// will not shut down while we're working. The other option would be to write an
// application that we could launch which would actually prevent shutdown. We can't
// actually prevent shutdown from here, but I believe that a GUI app would be able
// to do so. However, launching a GUI app from here is non-trivial (basically, we
// want something like KUNCExecute, but that doesn't exist here. We can write a 
// kernel extension whose sole purpose is to call KUNCExecute, but it seems a little
// silly.) Furthermore, the GUI app would only work if someone is logged into the GUI
// (i.e. not for Darwin). And it is likely that this will work well enough (that is,
// that it will finish before someone is tempted to shut down anyway.) So we'll go
// with it for the moment.
// 
// -------------------

void
warn_the_user ()
{
	if (warnings == 0) {
		SInt32 err;
		notification = CFUserNotificationCreate (NULL, 0, kCFUserNotificationCautionAlertLevel | kCFUserNotificationNoDefaultButtonFlag, &err, warningDict);
	}
	warnings++;
}

void
cancel_warning ()
{
	SInt32 err;

	if (warnings <= 0) return;
	warnings--;

	if (warnings == 0) {
		CFUserNotificationCancel (notification);
		CFRelease (notification);
		
		notification = CFUserNotificationCreate (NULL, 0, kCFUserNotificationNoteAlertLevel | kCFUserNotificationNoDefaultButtonFlag, &err, cancelWarningDict);
		sleep (2);
		CFUserNotificationCancel (notification);
		CFRelease (notification);
	}
}

int 
getMountPointForOFPath (char *mountPoint, const char *ofPath)
{ 
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
	return err;
}

int 
initialize_paths ()
{
	bootDevicePath[0] = 0;
	rootDevicePath[0] = 0;

	io_registry_entry_t options = IORegistryEntryFromPath (iokitPort, "IODeviceTree:/options");
	if (options == NULL) return (1);

	CFMutableDictionaryRef properties;
	IORegistryEntryCreateCFProperties (options, &properties, kCFAllocatorDefault, kNilOptions);
	if (properties == NULL) return (2);

	CFStringRef bootDevice = CFDictionaryGetValue (properties, CFSTR ("boot-device"));
	CFStringRef bootArgs = CFDictionaryGetValue (properties, CFSTR ("boot-args"));
	if (bootDevice == NULL || bootArgs == NULL) return (3);
	
	char bootDeviceOFPath[255], rootDeviceOFPath[255];
	
	CFStringGetCString (bootDevice, bootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	
	CFStringGetCString (bootArgs, rootDeviceOFPath, 255, CFStringGetSystemEncoding ());
	char *pos = strstr (rootDeviceOFPath, "rd=*");
	if (pos) {
		pos += 4;
		char *endpos = strstr (pos, " ");
		if (endpos) *endpos = 0;
		memmove (rootDeviceOFPath, pos, strlen (pos) + 1);
	} else {
		strcpy (rootDeviceOFPath, bootDeviceOFPath);
	}
	
	if (getMountPointForOFPath (rootDevicePath, rootDeviceOFPath)) return (4);
	if (getMountPointForOFPath (bootDevicePath, bootDeviceOFPath)) return (5);
	
	if (strcmp (rootDevicePath, bootDevicePath)) {
		noSyncRequired = 0;
		pos = rootDeviceOFPath;
		while (*pos != 0) {
			if (*pos == ':') *pos = ';';
			pos++;
		}
		strcat (bootDevicePath, "/.XPostFacto/");
		strcat (bootDevicePath, rootDeviceOFPath);
	} else {
		noSyncRequired = 1;
	}
	
	CFRelease (properties);
	IOObjectRelease (options);
	return 0;
}

void
initialize_warning_strings ()
{
	CFStringRef warningDictKeys [] = {kCFUserNotificationAlertHeaderKey, kCFUserNotificationAlertMessageKey};
	CFStringRef warningDictValues [] = {CFSTR ("XPostFacto Synchronization in Progress"), CFSTR ("XPostFacto is synchronizing system software changes with the 'helper' disk used in the boot process. Do not restart your computer until the synchronization process is finished.")};
	
	CFStringRef cancelWarningDictKeys [] = {kCFUserNotificationAlertHeaderKey, kCFUserNotificationAlertMessageKey};
	CFStringRef cancelWarningDictValues [] = {CFSTR ("XPostFacto Synchronization Completed"), CFSTR ("XPostFacto has finished synchronizing your system software with the 'helper' disk used in the boot process.")};

	warningDict = CFDictionaryCreate (NULL, (const void **) warningDictKeys, (const void **) warningDictValues, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	cancelWarningDict = CFDictionaryCreate (NULL, (const void **) cancelWarningDictKeys, (const void **) cancelWarningDictValues, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks); 
}

void
initialize_everything_else ()
{
	int err;
	
	err = initialize_paths ();
	if (err) exit (err);
	
	err = chdir (bootDevicePath);
	if (err) exit (1);
	
	err = stat ("System/Library", &sb);
	if (err) exit (1);
			
	if (!noSyncRequired) {					
		err = stat ("System/Library/Extensions.mkext", &sb);
		if (err) {
			mkextMod.tv_sec = mkextMod.tv_nsec = 0;
		} else {
			mkextMod.tv_sec = sb.st_mtimespec.tv_sec; 
			mkextMod.tv_nsec = sb.st_mtimespec.tv_nsec;
		}
		
		err = stat ("mach_kernel", &sb);
		if (err) {
			kernelMod.tv_sec = kernelMod.tv_nsec = 0;	
		} else {
			kernelMod.tv_sec = sb.st_mtimespec.tv_sec;
			kernelMod.tv_nsec = sb.st_mtimespec.tv_nsec;
		}
		
		err = stat ("System/Library/Extensions", &sb);
		if (err) {
			extfolderMod.tv_sec = extfolderMod.tv_nsec = 0;	
		} else {
			extfolderMod.tv_sec = sb.st_mtimespec.tv_sec;
			extfolderMod.tv_nsec = sb.st_mtimespec.tv_nsec;
		}
	}
	
	kernelChanging = mkextChanging = extfolderChanging = 0;
	reinitializeNow = 0;	
}

void
handle_mkext_change (int hasChanged)
{
	if (mkextChanging && !hasChanged) {
		if (chdir (bootDevicePath)) exit (7);
		if (sb.st_mtimespec.tv_sec == 0) {
			unlink ("System/Library/Extensions.mkext");
		} else {
			char *command;
			asprintf (&command, "/usr/bin/ditto \"%s/System/Library/Extensions.mkext\" System/Library", rootDevicePath);
			system (command);
			free (command);
		}
		mkextChanging = 0;
		cancel_warning ();
	} else if (hasChanged) {
		if (!mkextChanging) warn_the_user ();
		mkextChanging = 1;
	}
	mkextMod.tv_sec = sb.st_mtimespec.tv_sec; 
	mkextMod.tv_nsec = sb.st_mtimespec.tv_nsec;
}

void
handle_kernel_change (int hasChanged)
{
	if (kernelChanging && !hasChanged) {
		if (chdir (bootDevicePath)) exit (8);
		if (sb.st_mtimespec.tv_sec == 0) {
			unlink ("mach_kernel");
		} else {
			char *command;
			asprintf (&command, "/usr/bin/ditto \"%s/mach_kernel\" .", rootDevicePath);
			system (command);
			free (command);
		}
		kernelChanging = 0;
		cancel_warning ();
	} else if (hasChanged) {
		if (!kernelChanging) warn_the_user ();
		kernelChanging = 1;
	}
	kernelMod.tv_sec = sb.st_mtimespec.tv_sec; 
	kernelMod.tv_nsec = sb.st_mtimespec.tv_nsec;
}

int
anything_changed_in_directory_since_time (char *path, struct timespec *sinceTime)
{
	static struct dirent *entry;
	static struct stat buffer;
	DIR *directory;
	
	lstat (path, &buffer);
	
	if ((buffer.st_mtimespec.tv_sec > sinceTime->tv_sec) || 
			((buffer.st_mtimespec.tv_sec = sinceTime->tv_sec) && (buffer.st_mtimespec.tv_nsec > sinceTime->tv_nsec))) {
		sinceTime->tv_sec = buffer.st_mtimespec.tv_sec;
		sinceTime->tv_nsec = buffer.st_mtimespec.tv_nsec;
		return true;
	}

	directory = opendir (path);
	if (directory == NULL) return false;	
	fchdir (directory->dd_fd);

	while ((entry = readdir (directory)) != NULL) {
		if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, "..")) continue;
	
		lstat (entry->d_name, &buffer);
		
		if ((buffer.st_mode & S_IFMT) == S_IFLNK) {
			continue;
		} else if ((buffer.st_mode & S_IFMT) == S_IFDIR) {
			if (anything_changed_in_directory_since_time (entry->d_name, sinceTime)) {
				closedir (directory);
				return true;
			} else {
				fchdir (directory->dd_fd);
			}
		} else if ((buffer.st_mtimespec.tv_sec > sinceTime->tv_sec) || 
				((buffer.st_mtimespec.tv_sec = sinceTime->tv_sec) && (buffer.st_mtimespec.tv_nsec > sinceTime->tv_nsec))) {
			sinceTime->tv_sec = buffer.st_mtimespec.tv_sec;
			sinceTime->tv_nsec = buffer.st_mtimespec.tv_nsec;
			closedir (directory);
			return true;
		}
	}
	
	closedir (directory);
	return false;
}

void
most_recent_change_in_directory (char *path, struct timespec *changeTime)
{
	static struct dirent *entry;
	static struct stat buffer;
	DIR *directory;
	
	lstat (path, &buffer);
	
	if ((buffer.st_mtimespec.tv_sec > changeTime->tv_sec) || 
			((buffer.st_mtimespec.tv_sec = changeTime->tv_sec) && (buffer.st_mtimespec.tv_nsec > changeTime->tv_nsec))) {
		changeTime->tv_sec = buffer.st_mtimespec.tv_sec;
		changeTime->tv_nsec = buffer.st_mtimespec.tv_nsec;
	}

	directory = opendir (path);
	if (directory == NULL) return;
	fchdir (directory->dd_fd);

	while ((entry = readdir (directory)) != NULL) {
		if (!strcmp (entry->d_name, ".") || !strcmp (entry->d_name, "..")) continue;
	
		lstat (entry->d_name, &buffer);
		
		if ((buffer.st_mode & S_IFMT) == S_IFLNK) {
			continue;
		} else if ((buffer.st_mode & S_IFMT) == S_IFDIR) {
			most_recent_change_in_directory (entry->d_name, changeTime);
			fchdir (directory->dd_fd);
		} else if ((buffer.st_mtimespec.tv_sec > changeTime->tv_sec) || 
				((buffer.st_mtimespec.tv_sec = changeTime->tv_sec) && (buffer.st_mtimespec.tv_nsec > changeTime->tv_nsec))) {
			changeTime->tv_sec = buffer.st_mtimespec.tv_sec;
			changeTime->tv_nsec = buffer.st_mtimespec.tv_nsec;
		}
	}

	closedir (directory);
}

void
handle_extensions_folder_change (int hasChanged)
{
	if (extfolderChanging) {
		if (chdir (rootDevicePath)) exit (9);
		if (!anything_changed_in_directory_since_time ("System/Library/Extensions", &extfolderMod)) {
			if (chdir (bootDevicePath)) exit (10);
			system ("/bin/rm -rf System/Library/Extensions");

			char *command;
			asprintf (&command, "/usr/bin/ditto -rsrcFork \"%s/System/Library/Extensions\" System/Library/Extensions", rootDevicePath);
			system (command);
			free (command);

			extfolderMod.tv_sec = sb.st_mtimespec.tv_sec; 
			extfolderMod.tv_nsec = sb.st_mtimespec.tv_nsec;
			extfolderChanging = 0;
			cancel_warning ();
		}
	} else if (hasChanged) {
		warn_the_user ();
		extfolderChanging = 1;
		bzero (&extfolderMod, sizeof (extfolderMod));
		if (chdir (rootDevicePath)) exit (11);
		most_recent_change_in_directory ("System/Library/Extensions", &extfolderMod);
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
	unlink ("/var/run/xpfbootsupportd.pid");
}

void 
handle_SIGHUP (int sig)
{
	reinitializeNow = 1;
}

int
main (int argc, char **argv)
{	
	daemon (0, 0);

	writePID ();
	atexit (deletePID);
	
	IOMasterPort (MACH_PORT_NULL, &iokitPort);
	
	initialize_warning_strings ();
	initialize_everything_else ();
	
	signal (SIGHUP, &handle_SIGHUP);
	
	sigset_t allsigs;
	sigemptyset (&allsigs);
	
	while (1) {			
		sleep (XPF_SECONDS_TO_SLEEP);
		
		while (noSyncRequired) {
			sigsuspend (&allsigs);
			if (reinitializeNow) initialize_everything_else ();
		}

		int err;
		
		if (chdir (rootDevicePath)) exit (1);
		err = stat ("System/Library/Extensions.mkext", &sb);
		if (err) sb.st_mtimespec.tv_sec = sb.st_mtimespec.tv_nsec = 0;
		handle_mkext_change ((mkextMod.tv_sec != sb.st_mtimespec.tv_sec) || (mkextMod.tv_nsec != sb.st_mtimespec.tv_nsec));
	
		if (chdir (rootDevicePath)) exit (1);
		err = stat ("mach_kernel", &sb);
		if (err) sb.st_mtimespec.tv_sec = sb.st_mtimespec.tv_nsec = 0;
		handle_kernel_change ((kernelMod.tv_sec != sb.st_mtimespec.tv_sec) || (kernelMod.tv_nsec != sb.st_mtimespec.tv_nsec));
	
		if (chdir (rootDevicePath)) exit (1);
		err = stat ("System/Library/Extensions", &sb);
		if (err) sb.st_mtimespec.tv_sec = sb.st_mtimespec.tv_nsec = 0;
		handle_extensions_folder_change ((extfolderMod.tv_sec != sb.st_mtimespec.tv_sec) || (extfolderMod.tv_nsec != sb.st_mtimespec.tv_nsec));
	}
	
    return 0;
}
