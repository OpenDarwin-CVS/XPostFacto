/*

Copyright (c) 2004
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


#include "XPFBusPopup.h"
#include "XPostFacto.h"
#include "MountedVolume.h"
#include "XPFVolumeInspectorWindow.h"

//========================================================================================
// CLASS XPFBusPopup
//========================================================================================

#undef Inherited
#define Inherited TPopup

MA_DEFINE_CLASS (XPFBusPopup);

void 
XPFBusPopup::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);	
	
	fVolume = NULL;

	CreateMenu ();
	
#ifdef __MACH__
	AddItem ("n/a");
	EnableItem (1, false);
	SetActiveState (false, false);
#endif

	DoUpdate (cSetVolumeInspectorVolume, GetWindow (), ((XPFVolumeInspectorWindow *) GetWindow ())->getVolume (), NULL);
	GetWindow ()->AddDependent (this);
}

void 
XPFBusPopup::DoEvent (EventNumber eventNumber, TEventHandler* source, TEvent* event)
{
	if (eventNumber == mPopupHit) {
		short itemOffset = GetCurrentItem ();
		fVolume->setBus ((XPFBus *) fVolume->getBusList()->At (itemOffset));
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

void
XPFBusPopup::DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace)
{
#ifdef __MACH__
	Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
#else
	CVoidList_AC *list = NULL;

	switch (theChange) {
		case cSetVolumeInspectorVolume:
			if (!changeData) return;
			if (fVolume) fVolume->RemoveDependent (this);
			fVolume = (MountedVolume *) changeData;
			fVolume->AddDependent (this);
			
			DeleteAll ();
			list = fVolume->getBusList ();
			SetActiveState (list, true);
			if (list) {
				for (int x = 1; x <= list->GetSize (); x++) {
					XPFBus *bus = (XPFBus *) (list->At (x));
					AddItem (bus->getOpenFirmwareName (false));
					if (bus == fVolume->getDefaultBus ()) SetItemStyle (x, underline);
				}
			} else {
				AddItem ("n/a");
				EnableItem (1, false);
			}
			
			DoUpdate (cSetBus, changedObject, changeData, dependencySpace);
			break;
			
		case cSetBus:
			XPFBus *bus = fVolume->getBus ();
			if (bus) {
				list = fVolume->getBusList ();
				for (int x = 1; x <= list->GetSize (); x++) {
					if (list->At (x) == bus) {
						SetCurrentItem (x, true);
						break;
					}
				}
			} else {
				SetCurrentItem (1, true);
			}
			break;
	}
#endif
}
