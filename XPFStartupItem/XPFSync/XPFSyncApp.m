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

#import "XPFSyncApp.h"
#include "XPFSyncPaths.h"
#include "FSRefCopying.h"

#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>

pascal Boolean 
CopyFilterGlue (void *refCon, const FSRef *src, Boolean preflight)
{
	[(XPFSyncApp *) refCon copyFilter:src preflight:preflight];
}

@implementation XPFSyncApp

- (Boolean) copyFilter:(const FSRef *)src preflight:(Boolean)preflight
{
	if (preflight) {
		fItemsToCopy++;
		[fProgressIndicator setMaxValue:fItemsToCopy];
	} else {
		fItemsCopied++;
		[fProgressIndicator setDoubleValue:fItemsCopied];
		HFSUniStr255 unistring;
		FSGetCatalogInfo (src, kFSCatInfoNone, NULL, &unistring, NULL, NULL);
		NSString *fileName = [[NSString alloc] initWithCharactersNoCopy:unistring.unicode length:unistring.length freeWhenDone:NO];
		NSString *copyMessage = [[NSString alloc] initWithFormat:@"%@ %@", fCopyWord, fileName];
		[fCopyMessage setStringValue:copyMessage];
		[copyMessage release];
		[fileName release];
	}
	return true;
}

- (void) applicationDidFinishLaunching: (NSNotification *) aNotification
{
	openlog ("XPFSync", 0, LOG_USER);
	syslog (LOG_ERR, "finished launching");
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication *) sender;
{
	return [self synchronizeWithHelper];
}

- (void) sayTerminateOKNow
{
	[NSApp replyToApplicationShouldTerminate:YES];
}

- (void) awakeFromNib
{
	[fCopyMessage setStringValue:@""];
	fItemsToCopy = 0;
	fItemsCopied = 0;
	NSBundle *thisBundle = [NSBundle bundleForClass: [self class]];
	fCopyWord = [thisBundle localizedStringForKey:@"Copying" value:nil table:nil];
	[fCopyWord retain];
}

- (void) dealloc
{
	[fCopyWord release];
}

- (void) beginAlertSheetWithTitle:(NSString *)title defaultButton:(NSString *)defaultButton
	alternateButton:(NSString *)alternateButton otherButton:(NSString *)otherButton
	didDismissSelector:(SEL)didDismissSelector msg:(NSString *)msg
{
	NSBundle *thisBundle = [NSBundle bundleForClass: [self class]];
	NSBeginAlertSheet (
		[thisBundle localizedStringForKey:title value:nil table:nil], 
		[thisBundle localizedStringForKey:defaultButton value:nil table:nil], 
		[thisBundle localizedStringForKey:alternateButton  value:nil table:nil], 
		[thisBundle localizedStringForKey:otherButton value:nil table:nil], 
		fMainWindow, self, NULL, didDismissSelector, nil,
		[thisBundle localizedStringForKey:msg value:nil table:nil], 
		nil
	);
}

- (int) copyFile:(char *)fromPath toPath:(char *)toPath
{
	syslog (LOG_ERR, "Copying from %s to %s", fromPath, toPath);

	int err;
	FSRef fromRef, toRef;
	err = FSPathMakeRef (fromPath, &fromRef, NULL);
	if (err) return err;
	err = FSPathMakeRef (toPath, &toRef, NULL);
	if (err) return err;

	return FSRefFileCopy (&fromRef, &toRef, NULL, NULL, NULL, false);
}

- (int) copyFolder:(char *)fromPath toPath:(char *)toPath
{
	syslog (LOG_ERR, "Copying from %s to %s", fromPath, toPath);

	int err;
	FSRef fromRef, toRef;
	err = FSPathMakeRef (fromPath, &fromRef, NULL);
	if (err) return err;
	err = FSPathMakeRef (toPath, &toRef, NULL);
	if (err) return err;

	return FSRefFilteredDirectoryCopy (&fromRef, &toRef, NULL, NULL, 0, true, NULL, &CopyFilterGlue, self);
}

- (NSApplicationTerminateReply) synchronizeWithHelper
{
	char rootDevicePath[256], bootDevicePath[256];
	getPaths (rootDevicePath, bootDevicePath);
	
	syslog (LOG_ERR, "rootDevice = %s", rootDevicePath);
	syslog (LOG_ERR, "bootDevice = %s", bootDevicePath);
	
	if (!strcmp (rootDevicePath, bootDevicePath)) {
		syslog (LOG_ERR, "synchronization is not necessary");
		return NSTerminateNow;	
	}
	
	int err = seteuid (0);
	if (err) {
		[self 	beginAlertSheetWithTitle:@"Privileges Error" 
				defaultButton:@"OK" alternateButton:nil otherButton:nil
				didDismissSelector:@selector(sayTerminateOKNow) 
				msg:@"InsufficientPrivileges"];
				
		return NSTerminateLater;
	}
	
	char *rootPath = NULL, *bootPath = NULL;
	fItemsToCopy = 2;

	// Copy the extensions
	asprintf (&rootPath, "%s/System/Library/Extensions", rootDevicePath);
	asprintf (&bootPath, "%s/System/Library", bootDevicePath);
	err = [self copyFolder:rootPath toPath:bootPath];
	free (rootPath); // we'll free the bootPath below after we copy the Extensions.mkext
	
	// Copy the Extensions.mkext (or delete if not there)
	if (err == noErr) {
		asprintf (&rootPath, "%s/System/Library/Extensions.mkext", rootDevicePath);
		err = [self copyFile:rootPath toPath:bootPath];
		free (rootPath);
	}
	
	free (bootPath);
	
	// Copy the mach_kernel file
	if (err == noErr) {
		asprintf (&rootPath, "%s/mach_kernel", rootDevicePath);
		err = [self copyFile:rootPath toPath:bootDevicePath];
		free (rootPath);
	}

	if (err) {
		syslog (LOG_ERR, "Error %d while copying files", err);
		[self 	beginAlertSheetWithTitle:@"Copy Error" 
				defaultButton:@"OK" alternateButton:nil otherButton:nil
				didDismissSelector:@selector(sayTerminateOKNow) 
				msg:@"CopyError"];		
		seteuid (getuid ());
		return NSTerminateLater;
	}
	
	// Tell the startup item to start checking things again
	system ("/Library/StartupItems/XPFStartupItem/XPFStartupItem restart");
	
	seteuid (getuid ());
	
	return NSTerminateNow;
}

@end
