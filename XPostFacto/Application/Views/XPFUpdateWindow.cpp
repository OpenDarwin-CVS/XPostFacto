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

#include "XPFUpdateWindow.h"
#include "XPostFacto.h"
#include "CLongRect_AC.h"
#include "XPFStrings.h"
#include "vers_rsrc.h"
#include "UThemeEnvironment.h"

MA_DEFINE_CLASS(XPFUpdateWindow);

void 
XPFUpdateWindow::setUpdateItemList (XPFUpdateItemList *list)
{
	CStr255_AC theWordDelete ((ResNumber) kXPFStringsResource, kDeleteWord);
	CStr255_AC theWordInstall ((ResNumber) kXPFStringsResource, kInstallWord);

	TView *listView = FindSubView ('list');
	listView->SetDrawingEnvironment (new TThemeWhiteEnvironment, false);

	CViewPoint offset (0, 0);
	
	for (XPFUpdateListIterator iter (list); iter.Current (); iter.Next ()) {
		if (iter->getAction () == kActionNone) continue;
	
		TView *itemView = TViewServer::fgViewServer->DoCreateViews (GetDocument (), listView, kUpdateItem, offset);
		
		TStaticText *name = (TStaticText *) itemView->FindSubView ('item');
		TStaticText *installed = (TStaticText *) itemView->FindSubView ('inst');
		TStaticText *available = (TStaticText *) itemView->FindSubView ('avai');
		TStaticText *action = (TStaticText *) itemView->FindSubView ('acti');
		
		name->SetText (iter->getResourceName (), false);
		
		char buffer [64];
		
		parseVersion (buffer, iter->getInstalledVersion ());
		installed->SetText (buffer, false);
		
		parseVersion (buffer, iter->getAvailableVersion ());
		available->SetText (buffer, false);
		
		if (!iter->getIsQualified ()) available->SetText ("n/a", false);
		
		switch (iter->getAction ()) {
	
			case kActionDelete:
				theWordDelete.CopyTo (buffer);
				break;
			
			case kActionInstall:
				theWordInstall.CopyTo (buffer);
				break;
				
			default:
				buffer[0] = 0;
				break;
				
		}
		
		action->SetText (buffer, false);	
		
		offset.v += itemView->GetSize().v;
	}	
}

void
XPFUpdateWindow::parseVersion (char *buffer, UInt32 version)
{	
	buffer[0] = 0;
	if (version) {
		VERS_string (buffer, 32, version);
	} else {
		strcpy (buffer, "n/a");
	}
}
