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

#include "XPFOpenURLBehavior.h"
#include "XPFURLs.h"
#include "InternetConfig.h"

struct URLResource {
	IDType idType;
	short index;
};

MA_DEFINE_CLASS (XPFOpenURLBehavior);

#define Inherited TBehavior

XPFOpenURLBehavior::XPFOpenURLBehavior (IDType itsIdentifier)
	: TBehavior (itsIdentifier)
{
}

void 
XPFOpenURLBehavior::DoEvent (EventNumber eventNumber, TEventHandler* source, TEvent* event)
{
	Inherited::DoEvent (eventNumber, source, event);
	if (eventNumber == mStaticTextHit) {
		if (fURL != "") {
			if ((Ptr) ICStart == (Ptr) kUnresolvedCFragSymbolAddress) return;
			long start = 0, end = fURL[0];
			ICInstance inst;
			ThrowIfOSErr_AC (ICStart (&inst, 'usuX'));
			#ifndef __MACH__
				ThrowIfOSErr_AC (ICFindConfigFile (inst, 0, NULL));
			#endif
			ThrowIfOSErr_AC (ICLaunchURL (inst, "\p", &fURL[1], fURL[0], &start, &end));
			ICStop (inst);
		}
	}
}

void
XPFOpenURLBehavior::DoPostCreate (TDocument* itsDocument)
{	
	Inherited::DoPostCreate (itsDocument);
	
	Handle urls = GetResource ('urls', kURLResource);
	if (urls) {
		CTempHandleLock_AC lock (urls);
		URLResource *r = (URLResource *) *urls;
		unsigned handleLength = GetHandleSize (urls) / sizeof (URLResource);
		for (unsigned x = 0; x < handleLength; x++) {
			if (r[x].idType == fOwner->GetIdentifier ()) {
				fURL.CopyFrom (kURLResource, r[x].index, 255);
				break;
			}
		}
	}
}
