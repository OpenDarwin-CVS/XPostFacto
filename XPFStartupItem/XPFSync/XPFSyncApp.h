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

#import <Cocoa/Cocoa.h>

@interface XPFSyncApp : NSObject
{
	IBOutlet NSTextField *fCopyMessage;
	IBOutlet NSWindow *fMainWindow;
	IBOutlet NSProgressIndicator *fProgressIndicator;
	IBOutlet NSButton *fSychronizeButton;
	
	unsigned fItemsToCopy;
	unsigned fItemsCopied;
	NSString *fCopyWord;
}

- (void) applicationDidFinishLaunching: (NSNotification *) aNotification;
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication *) sender;
- (NSApplicationTerminateReply) synchronizeWithHelper;
- (void) awakeFromNib;
- (void) sayTerminateOKNow;
- (int) copyFile:(char *)fromPath toPath:(char *)toPath;
- (int) copyFolder:(char *)fromPath toPath:(char *)toPath;
- (Boolean) copyFilter:(const FSRef *)src preflight:(Boolean)preflight;

- (void) beginAlertSheetWithTitle:(NSString *)title defaultButton:(NSString *)defaultButton
	alternateButton:(NSString *)alternateButton otherButton:(NSString *)otherButton
	didDismissSelector:(SEL)didDismissSelector msg:(NSString *)msg;
	
- (void) dealloc;

@end