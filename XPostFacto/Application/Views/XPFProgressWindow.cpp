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

#include "XPFProgressWindow.h"

#include "UProgressIndicator.h"
#include "XPFLog.h"

MA_DEFINE_CLASS (XPFProgressWindow);

XPFProgressWindow::XPFProgressWindow (WindowRef itsWMgrWindow, bool canResize, bool canClose, bool inDispose)
	: TWindow (itsWMgrWindow, canResize, canClose, inDispose)
{
	fSetDescription = false;
	fSetStatus = false;
	fSetMin = false;
	fSetMax = false;
	fSetValue = false;
	fFinished = false;
	fHasException = false;
	fThread = NULL;
	
	gLogFile << "XPFProgressWindow::XPFProgressWindow called\n" << endl_AC;
}

void 
XPFProgressWindow::DoPostCreate (TDocument* itsDocument)
{
	TWindow::DoPostCreate (itsDocument);
	
	fStatus = (TStaticText *) FindSubView ('stut');
	fCancel = (TButton *) FindSubView ('canc');
	fProgress = (TProgressIndicator *) FindSubView ('prog');
	fDialog = GetDialogBehavior ();
	
	SetIdleFreq (1);
}

void
XPFProgressWindow::setThread (CCooperativeThread_AC *thread)
{
	fThread = thread;
}

void 
XPFProgressWindow::setDescription (unsigned char* theText)
{
	fSetDescription = true;
	fDescriptionText.CopyFrom (theText);
	YieldToThread (kApplicationThreadID);
}
	
void 
XPFProgressWindow::setStatus (unsigned char* theStatus)
{
	fSetStatus = true;
	fStatusText.CopyFrom (theStatus);
	YieldToThread (kApplicationThreadID);
}

void 
XPFProgressWindow::setProgressMin (ViewCoordinate min)
{
	fSetMin = true;
	fMin = min;
}

void 
XPFProgressWindow::setProgressMax (ViewCoordinate max)
{
	fSetMax = true;
	fMax = max;
	YieldToThread (kApplicationThreadID);
}

void 
XPFProgressWindow::setProgressValue (ViewCoordinate value)
{
	fSetValue = true;
	fValue = value;
}

void
XPFProgressWindow::setFinished ()
{
	fFinished = true;
	fThread = NULL;
}

void 
XPFProgressWindow::displayException (CException_AC& ex)
{
	fException = ex;
	fHasException = true;
}

bool 
XPFProgressWindow::DoIdle (IdlePhase phase)
{
	if (phase != idleContinue) return TWindow::DoIdle (phase);
	
	if (fFinished) {
		fFinished = false;
		if (fProgress->GetMaximum () == 0) fProgress->SetMaximum (100, false);
		fProgress->SetDeterminate (true);
		fProgress->SetValue (fProgress->GetMaximum (), true);
		if (fDialog) fDialog->Dismiss ('cncl', false);
	}
	
	if (fHasException) {
		fHasException = false;
		ErrorAlert (fException.GetError (), fException.GetExceptionMessage ());
		if (fDialog) fDialog->Dismiss ('cncl', false);
	}
	
	if (fSetDescription) {
		this->SetTitle (fDescriptionText);
		fSetDescription = false;
	}
	
	if (fSetStatus) {
		fStatus->SetText (fStatusText, true);
		fSetStatus = false;
	}
	
	if (fSetMin) {
		fProgress->SetMinimum (fMin, true);
		fSetMin = false;
	}
	
	if (fSetMax) {
		fProgress->SetDeterminate (fMax != 0);
		fProgress->SetMaximum (fMax, true);
		fSetMax = false;
	}
	
	if (fSetValue) {
		fProgress->SetValue (fValue, true);
		fSetValue = false;
	}
	
	if (fThread && fThread->IsAlive ()) fThread->YieldTo ();
	
	return TWindow::DoIdle (phase);
}