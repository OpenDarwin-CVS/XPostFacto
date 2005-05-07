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

// The purpose of this class is to encapsulate XPF's knowledge of the 
// platform. That is, which machine are we running on, and what special
// things do we need to do for that machine.

#ifndef __XPFPLATFORM_H__
#define __XPFPLATFORM_H__

class XPFPlatform {

public:

	~XPFPlatform ();
	
	static XPFPlatform* GetPlatform ();

	bool getIsNewWorld () {return fIsNewWorld;}
	char* getCompatible () {return fCompatible;}
	bool getCanPatchNVRAM () {return fNVRAMPatch != NULL;}
	void patchNVRAM ();

	static void getCompatibleFromDeviceTree (char **compatible);

private:

	static XPFPlatform *gPlatform;

	XPFPlatform ();

	char *fNVRAMPatch;
	char *fCompatible;
	bool fIsNewWorld;
	
	void loadNVRAMPatch (char *compatible);
	void processOFVariable (char *name, char *value);
	void parsePatchStream (CStream_AC *patches);
	
};

#endif