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

#ifndef __XPFFSREF_H__
#define __XPFFSREF_H__

class XPFFSRef {

public:

	static OSErr getOrCreateDirectory (FSRef *rootDirectory, char *path, UInt32 mode, FSRef *result, bool create = true);
	static OSErr getOrCreateXPFDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateSystemLibraryDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateSystemLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateCoreServicesDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateLibraryDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateStartupDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getOrCreateLibraryExtensionsDirectory (FSRef *rootDirectory, FSRef *result, bool create = true);

	static OSErr getFSRef (FSRef *rootDirectory, char *path, FSRef *result);
	static OSErr getKernelFSRef (FSRef *rootDirectory, FSRef *result);
	static OSErr getExtensionsCacheFSRef (FSRef *rootDirectory, FSRef *result, bool create = true);
	static OSErr getBootXFSRef (FSRef *rootDirectory, FSRef *result, bool create = true);
	
};

#endif