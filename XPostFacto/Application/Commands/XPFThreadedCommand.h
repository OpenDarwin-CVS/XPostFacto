/*

Copyright (c) 2003
Other World Computing
All rights reserved

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer as the first lines of
   each file.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

3. Redistributions in binary form must retain the link to Other World
   Computing's web site in the application's "About Box."

This software is provided by Other World Computing ``as is'' and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed. In no event
shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or
profits; or business interruption) however caused and on any theory of
liability, whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

*/

#ifndef __XPFTHREADEDCOMMAND_H__
#define __XPFTHREADEDCOMMAND_H__

#include "CThreadRunner_AC.h"
#include "CThreadYielder_AC.h"
#include "UnicodeConverter.h"
#include "CCooperativeThread_AC.h"

class XPFThreadedCommand;

class XPFCommandThread : public CCooperativeThread_AC {

	public:
		
		XPFCommandThread (XPFThreadedCommand *theCommand);
		
		virtual void Run ();
		
	protected:
										
		XPFThreadedCommand *fCommand;
		
};	

class XPFPrefs;
class MountedVolume;
class XPFProgressWindow;

class XPFThreadedCommand : public TCommand {

	public:
		
		XPFThreadedCommand (XPFPrefs *prefs);
		~XPFThreadedCommand ();
		
		void DoIt ();											
		virtual void DoItThreaded () = 0;

		XPFProgressWindow* getProgressWindow () {return fProgressWindow;}
			
		static pascal Boolean CopyFilterGlue (void *refCon, const FSRef *src, Boolean preflight);
		static pascal Boolean ArchiveFilterGlue (void *refCon, const FSRef *src, Boolean preflight);
	
	protected:
	
		Boolean copyFilter (const FSRef *src, Boolean preflight);
		Boolean archiveFilter (const FSRef *src, Boolean preflight);

		OSErr getOrCreateDirectory (FSRef *rootDirectory, char *path, UInt32 mode, FSRef *result);
		OSErr getOrCreateXPFDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateSystemLibraryDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateSystemLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateCoreServicesDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateLibraryDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateStartupDirectory (FSRef *rootDirectory, FSRef *result);
		OSErr getOrCreateLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result);

		OSErr getFSRef (FSRef *rootDirectory, char *path, FSRef *result);
		OSErr getKernelFSRef (FSRef *rootDirectory, FSRef *result);
		OSErr getExtensionsCacheFSRef (FSRef *rootDirectory, FSRef *result);
		OSErr getBootXFSRef (FSRef *rootDirectory, FSRef *result);
		
		void turnOffIgnorePermissionsForVolume (MountedVolume *volume);
		
		void updateExtensionsCacheForRootDirectory (FSRef *rootDirectory);
		void installExtensionsWithRootDirectory (FSRef *rootDirectory);
		void installSecondaryExtensionsWithRootDirectory (FSRef *rootDirectory);
		void installStartupItemWithRootDirectory (FSRef *rootDirectory);

		void copyHFSArchivesTo (ResType type, FSRef *directory);
				
		void setStatusMessage (unsigned char* message);
		void setCopyingFile (unsigned char* fileName);
		void setDescription (unsigned char* description);
		
		MountedVolume *fTargetDisk;
		MountedVolume *fHelperDisk;
		
		TextToUnicodeInfo fConverter;
		
		XPFProgressWindow *fProgressWindow;
		CThreadYielder_AC fYielder;
		CThreadRunner_AC fRunner;
		
		CStr63_AC fCopyWord;
		
		UInt32 fDebugOptions;
		
		unsigned fItemsToCopy;
		unsigned fItemsCopied;
		
		unsigned fProgressMin;
		unsigned fProgressMax;
};

#endif

