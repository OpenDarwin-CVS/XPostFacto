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

#ifndef __OFALIASES_H__
#define __OFALIASES_H__

#ifdef __MACH__
	#define REG_ENTRY_TYPE io_object_t
#else
	#define REG_ENTRY_TYPE RegEntryID*
#endif

class AliasEntry {

	public:
		AliasEntry (const char *key, const char *value);
		~AliasEntry ();
		
		char *abbreviate (TemplateList_AC <char> *wholeName);

	private:
		char *fKey;
		char *fValue;
		TemplateList_AC <char> fComponents;
		
};

class OFAliases {

	public:
	
		static void AliasFor (const REG_ENTRY_TYPE regEntry, char *outAlias, char *shortAlias);

		~OFAliases ();
		
	private:

		static CAutoPtr_AC <OFAliases> sOFAliases;
		static bool sHasBeenInitialized;

		static void Initialize ();

		void aliasFor (const REG_ENTRY_TYPE regEntry, char *outAlias, char *shortAlias);
		
		TemplateList_AC <AliasEntry> fEntries;

		OFAliases ();

#ifdef __MACH__		
		static void processDictionary (const void *key, const void *value, void *context);
#endif
		
};

#endif