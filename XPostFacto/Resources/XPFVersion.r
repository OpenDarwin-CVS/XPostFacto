/*

Copyright (c) 2002, 2003
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


#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif


//--------------------------------------------------------------------------------------------------
//  Version resources
//--------------------------------------------------------------------------------------------------

// The revision of this particular file

RESOURCE 'vers' (1,
#if qNames
"Application Version",
#endif
	purgeable) {
	0x03,
	0x00,
	beta,
	0x7,
	verUs,
	"3.0b7",
	"3.0b7, © 2001 - 2004 Other World Computing"
};

RESOURCE 'vers' (3,
#if qNames
"BootX Version",
#endif
	purgeable) {
	0x03,
	0x00,
	final,
	0xFF,
	verUs,
	"3.0",
	"3.0, © 2001 - 2004 Other World Computing"
};

RESOURCE 'vers' (4,
#if qNames
"Register Version",
#endif
	purgeable) {
	0x03,
	0x0,
	final,
	0x0,
	verUs,
	"3.0",
	"3.0, © 2001 - 2004 Other World Computing"
};

