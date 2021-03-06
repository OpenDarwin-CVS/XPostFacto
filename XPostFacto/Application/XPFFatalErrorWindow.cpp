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

3. Redistributions in binary form must retain unchanged the elements of the application 
   which credit Other World Computing (such as the splash screen and the "about box").

4. Redistributions in binary form must not require payment (or be bundled with items 
   requiring payment).

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


#include "XPFFatalErrorWindow.h"

//========================================================================================
// CLASS XPFFatalErrorWindow
//========================================================================================

MA_DEFINE_CLASS(XPFFatalErrorWindow);

void 
XPFFatalErrorWindow::Close()
{
	TWindow::Close ();
	if (!gApplication->GetDone ()) gApplication->DoMenuCommand (cQuit);
}

void 
XPFFatalErrorWindow::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	TWindow::DoEvent (eventNumber, source, event);
	if (eventNumber == mButtonHit) {
		gApplication->DoMenuCommand (cQuit);
	}
}

void 
XPFFatalErrorWindow::SetText (CStr255_AC text)
{
	TStaticText *textView = (TStaticText *) FindSubView ('Erro');
	textView->SetText (text, true);
}
