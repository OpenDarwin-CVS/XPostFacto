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

#ifndef __XPFPROGRESSWINDOW_H__
#define __XPFPROGRESSWINDOW_H__

#include "UWindow.h"
#include "CCooperativeThread_AC.h"

class TProgressIndicator;

class XPFProgressWindow : public TWindow
{
	MA_DECLARE_CLONE;

public:

	XPFProgressWindow (WindowRef itsWMgrWindow = NULL,
					  bool canResize = true,
					  bool canClose = true,
					  bool inDispose = false);
	virtual void DoPostCreate (TDocument* itsDocument);
	bool DoIdle (IdlePhase phase);
	
	void setThread (CCooperativeThread_AC *thread);
						
	// The following are all meant to be called from a thread.
						
	void setDescription (unsigned char* theText);
	void setStatus (unsigned char* theStatus);
	void setProgressMin (ViewCoordinate min);
	void setProgressMax (ViewCoordinate max);
	void setProgressValue (ViewCoordinate value);

	void displayException (CException_AC& ex);	
	void setFinished ();
	
private:

	TStaticText *fDescription;
	TStaticText *fStatus;
	TButton *fOkay;
	TButton *fCancel;
	TProgressIndicator *fProgress;
	
	CStr255_AC fDescriptionText;
	CStr255_AC fStatusText;
	ViewCoordinate fMin, fMax, fValue;
	CException_AC fException;
	
	bool fSetDescription, fSetStatus, fSetMin, fSetMax, fSetValue;
	bool fFinished;
	bool fHasException;
	
	CCooperativeThread_AC *fThread;
};

#endif