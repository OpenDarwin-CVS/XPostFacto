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

#include "XPFProgressWindow.h"

#include "UProgressIndicator.h"
#include "XPFLog.h"

MA_DEFINE_CLASS (XPFProgressWindow);

XPFProgressWindow::XPFProgressWindow (WindowRef itsWMgrWindow, bool canResize, bool canClose, bool inDispose)
	: TWindow (itsWMgrWindow, canResize, canClose, inDispose)
{
}

void 
XPFProgressWindow::DoPostCreate (TDocument* itsDocument)
{
	TWindow::DoPostCreate (itsDocument);
	
	fStatus = (TStaticText *) FindSubView ('stut');
	fProgress = (TProgressIndicator *) FindSubView ('prog');
	fDescription = (TStaticText *) FindSubView ('desc');
}

void
XPFProgressWindow::animate ()
{
	fProgress->HandleIdle (idleContinue);
	updateAllWindows (true);
}

void 
XPFProgressWindow::setDescription (unsigned char* theText)
{
	fDescription->SetText (theText, true);
	updateAllWindows (true);
}

void
XPFProgressWindow::updateAllWindows (bool forceRedraw)
{
	static UInt32 ticks = 0;
	UInt32 newTickCount = TickCount ();
	bool redraw = forceRedraw || (newTickCount > ticks + 6);		
	if (redraw) {
		gApplication->UpdateAllWindows ();	
		ticks = newTickCount;
	}
}
	
void 
XPFProgressWindow::setStatus (unsigned char* theStatus, bool forceRedraw)
{
	static UInt32 ticks = 0;
	UInt32 newTickCount = TickCount ();
	bool redraw = forceRedraw || (newTickCount > ticks + 6);		
	if (redraw) {
		fStatus->SetText (theStatus, true);
		updateAllWindows (forceRedraw);
		ticks = newTickCount;
	}
}

void 
XPFProgressWindow::setProgressMin (ViewCoordinate min)
{
	fProgress->SetMinimum (min, true);
}

void 
XPFProgressWindow::setProgressMax (ViewCoordinate max)
{
	fProgress->SetDeterminate (max != 0);
	fProgress->SetMaximum (max, true);
}

void 
XPFProgressWindow::setProgressValue (ViewCoordinate value, bool forceRedraw)
{
	static UInt32 ticks = 0;
	UInt32 newTickCount = TickCount ();
	bool redraw = forceRedraw || (newTickCount > ticks + 6);
	if (redraw) {
		fProgress->SetValue (value, true);
		updateAllWindows (forceRedraw);
		ticks = newTickCount;
	}
}

void
XPFProgressWindow::setFinished ()
{
	if (fProgress->GetMaximum () == 0) fProgress->SetMaximum (100, false);
	fProgress->SetDeterminate (true);
	fProgress->SetValue (fProgress->GetMaximum (), true);
	updateAllWindows (true);
}
