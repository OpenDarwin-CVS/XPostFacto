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

#include "XPFFSRef.h"
#include "XPFAuthorization.h"
#include "FSRefCopying.h"
#include "MoreFilesExtras.h"

#ifdef BUILDING_XPF
	#include "XPFApplication.h"
#endif

#include <UnicodeConverter.h>

// String constants

char systemName[] =  "System";
char libraryName[] = "Library";
char extensionsName[] = "Extensions"; 
char extensionsCacheName[] = "Extensions.mkext";
char privateName[] = "private";
char tmpName[] = "tmp";
char kernelName[] = "mach_kernel";
char coreServicesName[] = "CoreServices";
char bootXName[] = "BootX";
char installName[] = "XPFInstall";
char XPFName[] = ".XPostFacto";
char startupItemsName[] = "StartupItems";

// Constructor for getting the right permissions and ownership

class XPFCatalogInfo : public FSCatalogInfo {

	public:

		XPFCatalogInfo (UInt32 mode, bool isDirectory = true, UInt32 uid = 0, UInt32 gid = 0) {
			permissions[0] = uid;
			permissions[1] = gid;
			permissions[2] = mode | (isDirectory ? 040000 : 0100000);
			permissions[3] = 0;
		}
};

// Methods to get or create directories or other FSRefs (with proper permissions and ownership)

OSErr
XPFFSRef::getFSRef (FSRef *rootDirectory, char *path, FSRef *result)
{
	OSErr err;
	ByteCount uniLength, converted;
	UniChar uniChars[256];
	TextToUnicodeInfo converter;	

	ThrowIfOSErr_AC (CreateTextToUnicodeInfoByEncoding (
		CreateTextEncoding (kTextEncodingMacRoman, kTextEncodingDefaultVariant, kUnicode16BitFormat),
		&converter));
		
	err = ConvertFromTextToUnicode (converter, strlen (path), path, 0, 0, NULL, 0, NULL, 
			256 * sizeof (UniChar), &converted, &uniLength, uniChars);

	DisposeTextToUnicodeInfo (&converter);

	if (err) return err;

	return FSMakeFSRefUnicode (rootDirectory, uniLength / sizeof (UniChar), uniChars, NULL, result);
}

OSErr
XPFFSRef::getOrCreateDirectory (FSRef *rootDirectory, char *path, UInt32 mode, FSRef *result, bool create)
{
	OSErr err;
	ByteCount uniLength, converted;
	UniChar uniChars[256];
	TextToUnicodeInfo converter;

	ThrowIfOSErr_AC (CreateTextToUnicodeInfoByEncoding (
		CreateTextEncoding (kTextEncodingMacRoman, kTextEncodingDefaultVariant, kUnicode16BitFormat),
		&converter));
		
	err = ConvertFromTextToUnicode (converter, strlen (path), path, 0, 0, NULL, 0, NULL, 
			256 * sizeof (UniChar), &converted, &uniLength, uniChars);

	DisposeTextToUnicodeInfo (&converter);

	if (err) return err;

	XPFCatalogInfo catInfo (mode);
	XPFSetUID myUID (0);
	return FSGetOrCreateDirectoryUnicode (rootDirectory, 
			uniLength / sizeof (UniChar), uniChars, kFSCatInfoPermissions, &catInfo, 
			result, NULL, NULL, create);
}

OSErr
XPFFSRef::getOrCreateXPFDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err = getOrCreateDirectory (rootDirectory, XPFName, 0755, result, create);
	if (err == noErr) {
		FSSpec spec;
		FSCatalogInfo catInfo;
		err = FSGetCatalogInfo (result, kFSCatInfoFinderInfo, &catInfo, NULL, &spec, NULL);
		if (err == noErr) {
			FolderInfo *info = (FolderInfo *) catInfo.finderInfo;
			XPFSetUID myUID (0);
#ifdef BUILDING_XPF
			if (((XPFApplication *) gApplication)->getDebugOptions () & kVisibleHelperFiles) {			
				if ((info->finderFlags & kIsInvisible)) FSpClearIsInvisible (&spec);				
			} else {
				if (!(info->finderFlags & kIsInvisible)) FSpSetIsInvisible (&spec);
			}
#endif
		}
	}
	return err;
}

OSErr
XPFFSRef::getOrCreateSystemLibraryDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err;
	FSRef systemRef;
	
	err = getOrCreateDirectory (rootDirectory, systemName, 0755, &systemRef, create);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemRef, libraryName, 0755, result, create);
}

OSErr
XPFFSRef::getOrCreateSystemLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err;
	FSRef systemLibraryRef;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &systemLibraryRef, create);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemLibraryRef, extensionsName, 0755, result, create);
}

OSErr
XPFFSRef::getOrCreateCoreServicesDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err;
	FSRef systemLibraryRef;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &systemLibraryRef, create);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&systemLibraryRef, coreServicesName, 0755, result, create);
}

OSErr
XPFFSRef::getOrCreateLibraryDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	return getOrCreateDirectory (rootDirectory, libraryName, 0775, result, create);
}

OSErr
XPFFSRef::getOrCreateStartupDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err;
	FSRef libraryRef;
	
	err = getOrCreateLibraryDirectory (rootDirectory, &libraryRef, create);
	if (err != noErr) return err;
	
	return getOrCreateDirectory (&libraryRef, startupItemsName, 0755, result, create); 
}

OSErr
XPFFSRef::getOrCreateLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result, bool create)
{
	OSErr err;
	FSRef libraryRef;
	
	err = getOrCreateLibraryDirectory (rootDirectory, &libraryRef, create);
	if (err != noErr) return err;
				
	return getOrCreateDirectory (&libraryRef, extensionsName, 0755, result, create);
}

OSErr
XPFFSRef::getKernelFSRef (FSRef *rootDirectory, FSRef *result)
{
	return getFSRef (rootDirectory, kernelName, result);
}

OSErr
XPFFSRef::getExtensionsCacheFSRef (FSRef *rootDirectory, FSRef *result, bool create)
{
	FSRef libraryRef;
	OSErr err;
	
	err = getOrCreateSystemLibraryDirectory (rootDirectory, &libraryRef, create);
	if (err != noErr) return err;
	
	return getFSRef (&libraryRef, extensionsCacheName, result);
}

OSErr
XPFFSRef::getBootXFSRef (FSRef *rootDirectory, FSRef *result, bool create)
{
	FSRef coreServicesRef;
	OSErr err;
	
	err = getOrCreateCoreServicesDirectory (rootDirectory, &coreServicesRef, create);
	if (err != noErr) return err;
	
	return getFSRef (&coreServicesRef, bootXName, result);
}