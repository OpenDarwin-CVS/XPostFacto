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

#include "XPFHelpBehavior.h"
#include "XPFHelpStrings.h"

struct HelpStringResource {
	IDType idType;
	short index;
};

MA_DEFINE_CLASS (XPFHelpBehavior);

#define Inherited TBehavior

XPFHelpBehavior::XPFHelpBehavior (IDType itsIdentifier)
	: TBehavior (itsIdentifier),
		fHelpText (NULL)
{
}

bool 
XPFHelpBehavior::DoSetCursor (const CViewPoint& localPoint, RgnHandle cursorRegion)
{
	if (fHelpText) fHelpText->SetText (fHelpString, true);
	return true;
}

bool 
XPFHelpBehavior::DoUnsetCursor ()
{
	if (fHelpText) fHelpText->SetText ("", false);
	return true;
}

void
XPFHelpBehavior::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
	
	fHelpText = (TStaticText *) fOwner->GetWindow ()->FindSubView ('hlpt');
	
	Handle helpStrings = GetResource ('hlps', kHelpStringsResource);
	if (helpStrings) {
		CTempHandleLock_AC lock (helpStrings);
		HelpStringResource *r = (HelpStringResource *) *helpStrings;
		unsigned handleLength = GetHandleSize (helpStrings) / sizeof (HelpStringResource);
		for (unsigned x = 0; x < handleLength; x++) {
			if (r[x].idType == fOwner->GetIdentifier ()) {
				fHelpString = CString_AC (kHelpStringsResource, r[x].index);
				break;
			}
		}
	}
}
