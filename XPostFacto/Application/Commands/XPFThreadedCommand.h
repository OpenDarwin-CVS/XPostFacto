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

#include "UnicodeConverter.h"

class XPFPrefs;
class MountedVolume;
class XPFProgressWindow;
class XPFUpdate;

class XPFThreadedCommand : public TCommand {

	public:
		
		XPFThreadedCommand (XPFUpdate *update);
		
		void DoIt ();											
		virtual void DoItInProgressWindow () = 0;

		XPFProgressWindow* getProgressWindow () {return fProgressWindow;}
			
		static pascal Boolean CopyFilterGlue (void *refCon, const FSRef *src, Boolean preflight);
		static pascal Boolean ArchiveFilterGlue (void *refCon, const FSRef *src, Boolean preflight);
	
	protected:
	
		Boolean copyFilter (const FSRef *src, Boolean preflight);
		Boolean archiveFilter (const FSRef *src, Boolean preflight);
		
		void updateExtensionsCacheForRootDirectory (FSRef *rootDirectory);
		void installExtensionsWithRootDirectory (FSRef *rootDirectory, bool deleteUnqualified = true);
		void installSecondaryExtensionsWithRootDirectory (FSRef *rootDirectory);
		void installStartupItemWithRootDirectory (FSRef *rootDirectory);
		void synchronizeWithHelper (bool deleteFirst = false);
		
		void copyHFSArchivesTo (ResType type, FSRef *directory, bool deleteUnqualified = true);
				
		void setStatusMessage (unsigned char* message, bool forceRedraw);
		void setCopyingFile (unsigned char* fileName, bool forceRedraw);
		void setDescription (unsigned char* description);
		
		XPFPrefs *fPrefs;
		MountedVolume *fRootDisk;
		MountedVolume *fBootDisk;
		
		XPFUpdate *fUpdate;
		
		XPFProgressWindow *fProgressWindow;
		
		CStr63_AC fCopyWord;
		
		UInt32 fDebugOptions;
		
		unsigned fItemsToCopy;
		unsigned fItemsCopied;
		
		unsigned fProgressMin;
		unsigned fProgressMax;
};

#endif

