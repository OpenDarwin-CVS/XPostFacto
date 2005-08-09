/*

Copyright (c) 2001 - 2004
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

#include "XPFHelpBehavior.h"
#include "XPFHelpStrings.h"
#include "XPFLog.h"

MA_DEFINE_CLASS (XPFHelpBehavior);

#define Inherited TBehavior

XPFHelpBehavior::XPFHelpBehavior (IDType itsIdentifier)
	: TBehavior (itsIdentifier),
		fWindowContentUPP (NULL)
{
}

XPFHelpBehavior::~XPFHelpBehavior ()
{
	if (fWindowContentUPP) DisposeHMWindowContentUPP (fWindowContentUPP);
}

TView* 
XPFHelpBehavior::DeepestSubviewWithHelp (TView *view, CViewPoint &vp)
{
	TView *retVal = NULL;
	
	for (CSubViewIterator iter (view); iter.Current (); iter.Next ()) {
		if (iter->IsShown () && iter->GetFrame().Contains (vp)) {
			CViewPoint subviewpt = iter->SuperViewToLocal (vp);
			retVal = DeepestSubviewWithHelp (iter.Current (), subviewpt);
			break;	
		}
	}
		
	if (!retVal && (view->GetHelpID () != kNoResource)) retVal = view;

	return retVal;
}

pascal OSStatus 
XPFHelpBehavior::HelpTagCallback (WindowRef inWindow, Point inGlobalMouse, HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent)
{
	*outContentProvided = kHMContentNotProvided;
	
	if (inRequest == kHMSupplyContent) {
		TWindow *window = TWindow::WMgrToWindow (inWindow);
		if (!window) return noErr;
		
		CPoint_AC mouse (inGlobalMouse);
		GlobalToLocal (&mouse);
		CViewPoint viewPoint = window->QDToView (mouse);

		TView *helpView = DeepestSubviewWithHelp (window, viewPoint);
		if (!helpView) return noErr;

		CViewRect global (0, 0, 0, 0);

		TView *sv = helpView->GetSuperView ();
		if (sv) global = window->LocalToGlobal (sv->LocalToRootView (helpView->GetFrame ()));
			
		*outContentProvided =  kHMContentProvided;
		ioHelpContent->absHotRect = global.AsRect ();
		ioHelpContent->tagSide = kHMOutsideBottomLeftAligned;
		ioHelpContent->content[0].contentType = kHMStringResContent;
		ioHelpContent->content[0].u.tagStringRes.hmmResID = helpView->GetHelpID ();
		ioHelpContent->content[0].u.tagStringRes.hmmIndex = helpView->GetHelpIndex ();
		ioHelpContent->content[1].contentType = kHMNoContent;
	} 
	
	return noErr;
}

void
XPFHelpBehavior::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	TWindow *owner = dynamic_cast_AC (TWindow*, fOwner);
	
#ifdef __MACH__
	if (owner) {
		fWindowContentUPP = NewHMWindowContentUPP (HelpTagCallback);
		HMInstallWindowContentCallback (owner->GetWindowRef (), fWindowContentUPP);
	} 
#endif
	
}
