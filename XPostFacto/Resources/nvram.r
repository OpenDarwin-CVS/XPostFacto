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

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif

Read 'OFpt' (128, "PowerSurge") ":nvram:PowerSurge.h";
Read 'OFpt' (129, "Kanga") ":nvram:Kanga.h";
Read 'OFpt' (130, "Hooper") ":nvram:Hooper.h";
Read 'OFpt' (131, "Alchemy") ":nvram:Alchemy.h";
Read 'OFpt' (132, "PowerExpress") ":nvram:PowerExpress.h";
Read 'OFpt' (133, "Silk") ":nvram:Silk.h";
Read 'OFpt' (134, "Gossamer") ":nvram:Gossamer.h";
Read 'OFpt' (135, "Mainstreet") ":nvram:Mainstreet.h";
Read 'OFpt' (136, "Wallstreet") ":nvram:Wallstreet.h";


type 'OFtc' {
	array {
		cstring[36];
		integer;
	};
};

Resource 'OFtc' (128) {
	{
		"AAPL,7300", 128,
		"AAPL,????", 128,
		"AAPL,7500", 128,
		"AAPL,8500", 128,
		"AAPL,9500", 128,
		"AAPL,3500", 129,
		"AAPL,3400/2400", 130,
		"AAPL,e407", 131,
		"AAPL,e411", 131,
		"AAPL,9700", 132,
		"AAPL,Gossamer", 134,
		"AAPL,PowerMac G3", 133,
		"AAPL,PowerBook1998", 136
	}
};