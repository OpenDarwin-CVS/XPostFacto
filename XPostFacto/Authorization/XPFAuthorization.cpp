/*

Copyright (c) 2002
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


#include "XPFAuthorization.h"

#include "CCFBundle_AC.h"
#include "CCFURL_AC.h"
#include "CCFString_AC.h"
#include <Files.h>

#include <Sound.h>
#include <StdArg.h>

XPFAuthorization::XPFAuthorization ()
{	
	FSRef systemLibraryFrameworksFolder, securityRef, systemRef;
	UniChar securityName[] = {'S', 'e', 'c', 'u', 'r', 'i', 't', 'y', '.', 'f', 'r', 'a', 'm', 'e', 'w', 'o', 'r', 'k'};
	UniChar systemFramework[] = {'S', 'y', 's', 't', 'e', 'm', '.', 'f', 'r', 'a', 'm', 'e', 'w', 'o', 'r', 'k'};

	ThrowIfOSErr_AC (FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &systemLibraryFrameworksFolder));
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&systemLibraryFrameworksFolder,
			sizeof (securityName) / sizeof (UniChar), securityName, kTextEncodingUnknown, &securityRef));
	ThrowIfOSErr_AC (FSMakeFSRefUnicode (&systemLibraryFrameworksFolder,
			sizeof (systemFramework) / sizeof (UniChar), systemFramework, kTextEncodingUnknown, &systemRef));

	CAutoCFURL_AC securityURL (&securityRef);	
	fSecurityFramework = __CFBundle::Create(NULL, securityURL);
	if (!fSecurityFramework->LoadExecutable()) throw false;
	
	CAutoCFURL_AC systemURL (&systemRef);	
	fSystemFramework = __CFBundle::Create(NULL, systemURL);
	if (!fSystemFramework->LoadExecutable()) throw false;
	
	fCreate = (CreatePtr) fSecurityFramework->GetFunctionPointerForName (CFSTR ("AuthorizationCreate"));
	fFree = (FreePtr) fSecurityFramework->GetFunctionPointerForName (CFSTR ("AuthorizationFree"));
	fCopyRights = (CopyRightsPtr) fSecurityFramework->GetFunctionPointerForName (CFSTR ("AuthorizationCopyRights"));
	fExecute = (ExecuteWithPrivilegesPtr) fSecurityFramework->GetFunctionPointerForName (CFSTR ("AuthorizationExecuteWithPrivileges"));

	fvfprintf = (VFPrintFPtr) fSystemFramework->GetFunctionPointerForName (CFSTR ("vfprintf"));
	ffclose = (FClosePtr) fSystemFramework->GetFunctionPointerForName (CFSTR ("fclose"));

	ThrowIfNULL_AC (fCreate);
	ThrowIfNULL_AC (fFree);
	ThrowIfNULL_AC (fCopyRights);
	ThrowIfNULL_AC (fExecute);
	ThrowIfNULL_AC (fvfprintf);
	ThrowIfNULL_AC (ffclose);
}

XPFAuthorization::~XPFAuthorization ()
{
	(*fFree)(fAuthorization, kAuthorizationFlagDestroyRights);
	fSecurityFramework->UnloadExecutable ();
	fSystemFramework->UnloadExecutable ();
}

OSStatus
XPFAuthorization::Authenticate ()
{
	const char *path = "";	

	AuthorizationRights rights;
	rights.count = 0;
	rights.items = NULL;
	
	fAuthorization = NULL;
	
	(*fCreate)(&rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &fAuthorization);

	AuthorizationItem items[1];
    items[0].name = kAuthorizationRightExecute;
    items[0].value = (char *) path;
    items[0].valueLength = strlen (path);
    items[0].flags = 0;

    rights.count = 1;
    rights.items = items;
	    
	return (*fCopyRights)(fAuthorization, &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed, NULL);
}

OSStatus
XPFAuthorization::ExecuteWithPrivileges (const char *pathToTool,
						char * const *arguments,
						FILE **communicationsPipe)
{
	return (*fExecute)(fAuthorization, pathToTool, 0, arguments, communicationsPipe);
}
 
int 
XPFAuthorization::fprintf(FILE *stream, const char *format, ...)
{
	va_list varargs;
	va_start (varargs, format);
	int retVal = (*fvfprintf) (stream, format, varargs);	
	va_end (varargs);
	return retVal;
}

int 
XPFAuthorization::fclose(FILE *stream)
{	
	return (*ffclose)(stream);
}