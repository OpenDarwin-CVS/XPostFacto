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
#include "XPFErrors.h"

#include <StdArg.h>

XPFAuthorization XPFAuthorization::gXPFAuthorization;

XPFAuthorization::XPFAuthorization ()
{	
	fAuthorization = NULL;
}

XPFAuthorization::~XPFAuthorization ()
{
	if (fAuthorization) AuthorizationFree (fAuthorization, kAuthorizationFlagDestroyRights);
}

OSStatus
XPFAuthorization::authenticate ()
{
	const char *path = "";	

	AuthorizationRights rights;
	rights.count = 0;
	rights.items = NULL;
		
	static bool first = true;
	if (first) {
		AuthorizationCreate (&rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &fAuthorization);
		first = false;
	}
	
	AuthorizationItem items[1];
    items[0].name = kAuthorizationRightExecute;
    items[0].value = (char *) path;
    items[0].valueLength = strlen (path);
    items[0].flags = 0;

    rights.count = 1;
    rights.items = items;
	    
	return AuthorizationCopyRights (fAuthorization, &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed, NULL);
}

OSStatus
XPFAuthorization::ExecuteWithPrivileges (const char *pathToTool,
						char * const *arguments,
						FILE **communicationsPipe)
{
	OSErr err = gXPFAuthorization.authenticate ();
	if (err != errAuthorizationSuccess) ThrowException_AC (kCouldNotObtainAuthorization, 0);

	return AuthorizationExecuteWithPrivileges (gXPFAuthorization.fAuthorization, pathToTool, 0, arguments, communicationsPipe);
}
