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


#include "XPFAboutBox.h"
#include "XPFApplication.h"
#include <iostream.h>
#include <InternetConfig.h>
#include <Sound.h>
#include "XPostFacto.h"

MA_DEFINE_CLASS (XPFAboutBox);

XPFAboutBox::~XPFAboutBox()
{
	DisposeIfHandle_AC (fCopyrightText);
}

void 
XPFAboutBox::DoPostCreate (TDocument* itsDocument)
{
	TWindow::DoPostCreate (itsDocument);
	fURL = (TStaticText *) this->FindSubView ('urlT');
	fCopyrightNotice = (TTEView *) this->FindSubView ('Copy');
	
	fCopyrightText = GetResource ('TEXT', kCopyrightID);
	if (fCopyrightText) {
		DetachResource (fCopyrightText);
		for (int x = GetHandleSize (fCopyrightText) - 1; x >= 0; x--) {
			if ((*fCopyrightText)[x] == 10) (*fCopyrightText)[x] = 13;
		}
		fCopyrightNotice->StuffText (fCopyrightText);
	}
}

void 
XPFAboutBox::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	TWindow::DoEvent (eventNumber, source, event);
	if ((eventNumber == mStaticTextHit) && (source == fURL)) {
		CStr255_AC theURL = fURL->GetText ();
		((XPFApplication *) gApplication)->launchURL (theURL);
	} 
}
