/*

    Copyright (c) 2002
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

#include "HFSPlusArchive.h"
#include "MoreFilesExtras.h"

#include <Carbon/Carbon.h>

#include <fstream>

int main (int argc, char **argv)
{
	OSStatus result;
	
	if (argc != 3) return 1;
	
	FSRef theSourceRef;
	result = FSPathMakeRef ((UInt8 *) argv[1], &theSourceRef, NULL);
	if (result) return result;

	FSSpec theSource;
	result = FSGetCatalogInfo (&theSourceRef, kFSCatInfoNone, NULL, NULL, &theSource, NULL);
	if (result) return result;
	
	FSRef theTargetDirRef;
	Boolean isDirectory;
	result = FSPathMakeRef ((UInt8 *) argv[2], &theTargetDirRef, &isDirectory);
	if (result) return result;
	if (!isDirectory) return 1;
		
	char theName[255];
	p2cstrcpy (theName, theSource.name);
	char *pos = strstr (theName, ".");
	if (pos) *pos = '\0';
	strcat (theName, ".hfs");
	
	char thePath[255];
	snprintf (thePath, 255, "%s/%s", argv[2], theName);

	fstream theStream (thePath, fstream::out);
	
	try {
		HFSPlusArchive theArchive (&theStream);
		OSErr err = theArchive.addToArchive (&theSource);
		if (err) return err;
		theArchive.flushWrites ();
	}
	catch (...) {
		return 1;
	}
	return 0;
}