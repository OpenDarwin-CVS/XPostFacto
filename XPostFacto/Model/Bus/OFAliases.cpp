/*

Copyright (c) 2001, 2002
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

#include "OFAliases.h"
#include "XPFNameRegistry.h"
#include <iostream.h>
#include <string.h>
#include <stdio.h>
#include <PCI.h>

CAutoPtr_AC <OFAliases> OFAliases::sOFAliases = NULL;
bool OFAliases::sHasBeenInitialized = false;

void
OFAliases::Initialize ()
{
	if (sHasBeenInitialized) return;
	sHasBeenInitialized = true;
	sOFAliases = new OFAliases;
}

OFAliases::OFAliases ()
{
	ThrowIfOSErr_AC (RegistryEntryIDInit (&fAliasEntry));	
	ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree:aliases", &fAliasEntry));
}

void
OFAliases::AliasFor (const RegEntryID* regEntry, char *outAlias) {
	if ((Ptr) RegistryEntryIDInit == (Ptr) kUnresolvedCFragSymbolAddress) return;
	Initialize ();
	return sOFAliases->aliasFor (regEntry, outAlias);
}

void
OFAliases::aliasFor (const RegEntryID* regEntry, char *outAlias) {
	// First, we get the path to this RegEntry
	RegPathNameSize pathSize;
	ThrowIfOSErr_AC (RegistryEntryToPathSize (regEntry, &pathSize));
	RegCStrPathName *pathName = (RegCStrPathName *) malloc (pathSize);
	ThrowIfNULL_AC (pathName);
		
	try {
		ThrowIfOSErr_AC (RegistryCStrEntryToPath (regEntry, pathName, pathSize));
		
		// replace all the ':' with '/'
		char *iter = pathName;
		while (*iter != 0) {
			if (*iter == ':') *iter = '/';
			iter++;
		}
		
		// now, we get rid of the "Devices/device-tree/"
		iter = pathName;
		if (!memcmp (iter, "Devices/device-tree", 19)) iter += 19; 
		
		// now, we try to abbreviate it as is
		if (!OFAliases::abbreviate (iter, outAlias)) {

			// it didn't abbreviate as is. So we expand the first node & try again
			
			RegEntryID deviceTreeEntry;
			ThrowIfOSErr_AC (RegistryEntryIDInit (&deviceTreeEntry));
			ThrowIfOSErr_AC (RegistryCStrEntryLookup (NULL, "Devices:device-tree", &deviceTreeEntry));

			char expandedName[255];
			expandedName[0] = 0;
			RegEntryID iterEntry, parentEntry;
			ThrowIfOSErr_AC (RegistryEntryIDInit (&iterEntry));
			ThrowIfOSErr_AC (RegistryEntryIDInit (&parentEntry));
			ThrowIfOSErr_AC (RegistryEntryIDCopy (regEntry, &iterEntry));
			
			Boolean finished = false;
			char nameComponent [255];
			char workString[255];
			while (!finished) {
				ThrowIfOSErr_AC (RegistryCStrEntryToName (&iterEntry, &parentEntry, nameComponent, &finished));
				if (finished) break;
				if (RegistryEntryIDCompare (&parentEntry, &deviceTreeEntry)) {
					finished = true;
					RegPropertyValueSize propSize;
					OSErr err = RegistryPropertyGetSize (&iterEntry, "reg", &propSize);
					if (err == noErr) {
						Ptr reg = NewPtr (propSize + 1);
						ThrowIfNULL_AC (reg);
						ThrowIfOSErr_AC (RegistryPropertyGet (&iterEntry, "reg", reg, &propSize));
						reg[propSize] = '\0';
						snprintf (workString, 255, "/%s@%X%s", nameComponent, * (unsigned *) reg, expandedName);
						strcpy (expandedName, workString);
						DisposePtr (reg);	
					} else {			
						snprintf (workString, 255, "/%s%s", nameComponent, expandedName);
						strcpy (expandedName, workString);
					}
					RegistryEntryIDDispose (&iterEntry);
					RegistryEntryIDDispose (&parentEntry);
				} else {
					RegistryEntryIDDispose (&iterEntry);
					ThrowIfOSErr_AC (RegistryEntryIDCopy (&parentEntry, &iterEntry));
					RegistryEntryIDDispose (&parentEntry);
					snprintf (workString, 255, "/%s%s", nameComponent, expandedName);
					strcpy (expandedName, workString);
				}
			}
			if (!OFAliases::abbreviate (expandedName, outAlias)) {
				// OK, that didn't work either. So just return the plain name.
				strcpy (outAlias, iter);
			}
			RegistryEntryIDDispose (&deviceTreeEntry);	
		}
	}
	catch (...) {
		free (pathName);
		throw;
	}
	free (pathName);
}

bool
OFAliases::abbreviate (const char *inPath, char *outAlias)
{
	// If I knew what I was doing, I would cache this stuff. But it will
	// hardly be performance critical. The idea, in general, is to reduce the
	// amount of NVRAM we need by shortening our device names.

	RegPropertyIter cookie;
	Boolean done = false;
	int currentLength = 0;
	
	ThrowIfOSErr_AC (RegistryPropertyIterateCreate (&fAliasEntry, &cookie));
	while (!done) {
		RegPropertyNameBuf property;
		ThrowIfOSErr_AC (RegistryPropertyIterate (&cookie, property, &done));
		if (!done) {
			char *value;
			RegPropertyValueSize size;
			ThrowIfOSErr_AC (RegistryPropertyGetSize (&fAliasEntry, property, &size));
			value = NewPtr (size + 1);
			try {
				ThrowIfOSErr_AC (RegistryPropertyGet (&fAliasEntry, property, value, &size));
				value[size] = '\0';
				
				// OK. Now we have an alias name and a value. We need to check whether the
				// inPath starts with our value, with a '/ at the end. If so, we can substitute our name.
				// And we want to use the longest property that matches.
				if (strlen (value) > currentLength) {
					if (strstr (inPath, value) == inPath) {
						if ((inPath[strlen (value)] == '/') || (inPath[strlen (value)] == '\0')) {
							strcpy (outAlias, property);
							strcat (outAlias, inPath + strlen (value));
							currentLength = strlen (value);
						}
					}
				}
			}
			catch (...) {
				DisposePtr (value);
				throw;
			}
			DisposePtr (value);
		}
	}
	RegistryPropertyIterateDispose(&cookie);
	return currentLength != 0;
}

OFAliases::~OFAliases ()
{
	RegistryEntryIDDispose (&fAliasEntry);
}