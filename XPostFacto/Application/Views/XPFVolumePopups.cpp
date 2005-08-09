/*

Copyright (c) 2003 - 2004
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


#include "XPFVolumePopups.h"
#include "XPFApplication.h"
#include "XPFStrings.h"

#include "MountedVolume.h"
#include "XPFPrefs.h"
#include "XPostFacto.h"
#include "XPFLog.h"
#include "XPFVolumeInspectorWindow.h"

//========================================================================================
// CLASS XPFVolumePopup
//========================================================================================

#undef Inherited
#define Inherited TPopup

void 
XPFVolumePopup::DoPostCreate(TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
	
	fApp = (XPFApplication *) gApplication;
	fPrefs = (XPFPrefs *) GetDocument ();
	
	// Create the initial menu items. We'll catch changes through the "update" thingy.
	
	CreateMenu ();
	
	if (fUseNoneItem) {
		AddItem ("None");
		AddSeparator ();
	}
	
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		DoUpdate (cNewMountedVolume, iter.Current (), iter.Current (), NULL);
	}
	
	fApp->AddDependent (this);

	if (fPrefs) {
		DoUpdate (cSetRebootInMacOS9, fPrefs, NULL, NULL);
		fPrefs->AddDependent (this);
	}
}

void 
XPFVolumePopup::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	if (eventNumber == mPopupHit) {
		short itemOffset = GetCurrentItem ();
		if (fUseNoneItem) itemOffset -= 2;
		if (itemOffset < 0) {
			setMountedVolume (NULL);
		} else {
			setMountedVolume (fVolumeList.At (itemOffset));
		}
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

void
XPFVolumePopup::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	ArrayIndex_AC index;
	
	switch (theChange) {
	
		case cDeleteMountedVolume:
			index = fVolumeList.GetIdentityItemNo (volume);
			if (index) {
				if (fUseNoneItem) index += 2;
				fVolumeList.Delete (volume);
				DeleteItem (index);
			}
			break;
			
		case cNewMountedVolume:
			if (useVolumeInMenu (volume)) {
				fVolumeList.InsertLast (volume);
				AddItem (volume->getVolumeName ());
				volume->AddDependent (this);
				ForceRedraw ();
			}
			break;
		
		case cSetVolumeName:
			index = fVolumeList.GetIdentityItemNo (volume);
			if (index) {
				if (fUseNoneItem) index += 2;
				SetMenuItemText (index, volume->getVolumeName ());
			}
			break;
			
		case cSetRebootInMacOS9:
			if (fPrefs) SetActiveState (!fPrefs->getRebootInMacOS9 (), false);
			break;
			
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

// =================
// XPFInstallCDPopup
// =================

#undef Inherited
#define Inherited XPFVolumePopup

MA_DEFINE_CLASS (XPFInstallCDPopup);

void 
XPFInstallCDPopup::DoPostCreate (TDocument* itsDocument)
{
	fUseNoneItem = false;
	Inherited::DoPostCreate (itsDocument);
	DoUpdate (cSetInstallCD, fPrefs, fPrefs->getInstallCD (), NULL);
}

bool 
XPFInstallCDPopup::useVolumeInMenu (MountedVolume *volume)
{
	return (volume->getInstallerStatus () == kStatusOK);
}

void 
XPFInstallCDPopup::setMountedVolume (MountedVolume *volume)
{
	fPrefs->setInstallCD (volume);
}

void
XPFInstallCDPopup::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	
	switch (theChange) {
		
		case cSetInstallCD:
			ArrayIndex_AC index = fVolumeList.GetIdentityItemNo (volume);
			if (index) {
				if (fUseNoneItem) index += 2;
				SetCurrentItem (index, true);			
			}
			break;
		
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

// ==============
// XPFHelperPopup
// ==============

#undef Inherited
#define Inherited XPFVolumePopup

MA_DEFINE_CLASS (XPFHelperPopup);

void 
XPFHelperPopup::DoPostCreate (TDocument* itsDocument)
{
	fUseNoneItem = true;
	fTarget = NULL;
	
	Inherited::DoPostCreate (itsDocument);	

	if (fPrefs) {
		DoUpdate (cSetTargetDisk, fPrefs, fPrefs->getTargetDisk (), NULL);
	} else {
		DoUpdate (cSetVolumeInspectorVolume, GetWindow (), ((XPFVolumeInspectorWindow *) GetWindow ())->getVolume (), NULL);
		GetWindow ()->AddDependent (this);
	}
}

bool 
XPFHelperPopup::useVolumeInMenu (MountedVolume *volume)
{
	return (volume->getHelperStatus () == kStatusOK);
}

void 
XPFHelperPopup::setMountedVolume (MountedVolume *volume)
{
	if (fTarget) fTarget->setHelperDisk (volume);
}

void
XPFHelperPopup::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	ArrayIndex_AC index = 0;
	
	switch (theChange) {
		case cSetHelperDisk:
			if (volume == fTarget) {
				if (volume) index = fVolumeList.GetIdentityItemNo (volume->getHelperDisk ());
				if (index) index += 2;
				SetCurrentItem (index, true);
			}
			break;
		
		case cSetTargetDisk:
		case cSetVolumeInspectorVolume:
			fTarget = volume;
			if (volume) {
				SetActiveState (true, true);
				EnableItem (1, !volume->getRequiresBootHelper ());
				for (short x = 3; x <= GetNumberOfItems (); x++) EnableItem (x, true);
				index = fVolumeList.GetIdentityItemNo (volume);
				if (index) EnableItem (index + 2, false);
			} else {
				SetActiveState (false, true);
			}
			DoUpdate (cSetHelperDisk, changedObject, changeData, NULL);
			break;

		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;			
	}
}

// =======================
// XPFVolumeInspectorPopup
// =======================

#undef Inherited
#define Inherited XPFVolumePopup

MA_DEFINE_CLASS (XPFVolumeInspectorPopup);

void 
XPFVolumeInspectorPopup::DoPostCreate (TDocument* itsDocument)
{
	fUseNoneItem = false;
	GetWindow ()->AddDependent (this);
	Inherited::DoPostCreate (itsDocument);	
}

bool 
XPFVolumeInspectorPopup::useVolumeInMenu (MountedVolume *volume)
{
	#pragma unused (volume)
	return true;
}

void 
XPFVolumeInspectorPopup::setMountedVolume (MountedVolume *volume)
{
	((XPFVolumeInspectorWindow *) GetWindow ())->setVolume (volume);
}

void
XPFVolumeInspectorPopup::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	ArrayIndex_AC index = 0;
	
	switch (theChange) {
		case cSetVolumeInspectorVolume:
			if (volume) index = fVolumeList.GetIdentityItemNo (volume);
			SetCurrentItem (index, true);
			break;
		
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;			
	}
}