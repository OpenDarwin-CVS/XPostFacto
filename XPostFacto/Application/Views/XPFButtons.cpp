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

#include "XPFButtons.h"
#include "XPFPrefs.h"
#include "MountedVolume.h"
#include "XPFInstallCommand.h"
#include "XPFRestartCommand.h"
#include "XPostFacto.h"
#include "XPFStrings.h"

//========================================================================================
// CLASS XPFInstallButton
//========================================================================================

#undef Inherited
#define Inherited TButton

MA_DEFINE_CLASS (XPFInstallButton);

void 
XPFInstallButton::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	itsDocument->AddDependent (this);
	
	DoUpdate (cSetTargetDisk, itsDocument, ((XPFPrefs *) itsDocument)->getTargetDisk (), NULL);
	DoUpdate (cSetRebootInMacOS9, itsDocument, NULL, NULL);
}

void 
XPFInstallButton::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	if (eventNumber == mButtonHit) {
		XPFPrefs *prefs = (XPFPrefs *) GetDocument ();
		short retVal = prefs->PoseConfirmDialog (true, false);
		if (retVal == kStdOkItemIndex) {
			PostCommand (prefs->MakeSaveCommand ());		
			PostCommand (TH_new XPFInstallCommand (prefs));
		}
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

bool
XPFInstallButton::determineActiveState ()
{
	XPFPrefs *prefs = (XPFPrefs *) GetDocument ();
	if (prefs->getRebootInMacOS9 ()) return false;
	MountedVolume *targetDisk = prefs->getTargetDisk ();
	MountedVolume *installCD = prefs->getInstallCD ();
	if (!targetDisk) return false;
	if (!installCD) return false;
	if (targetDisk->getInstallTargetStatus () != kStatusOK) return false;
	if (installCD->getInstallerStatus () != kStatusOK) return false;
	if (prefs->getTooBigForNVRAM (true)) return false;
	return true;
}

void
XPFInstallButton::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	#pragma unused (theChange, changedObject, changeData, dependencySpace)

	// basically, any update from fPrefs might change our state
	SetActiveState (determineActiveState (), true);
}

//========================================================================================
// CLASS XPFRestartButton
//========================================================================================

#undef Inherited
#define Inherited TButton

MA_DEFINE_CLASS (XPFRestartButton);

void 
XPFRestartButton::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	itsDocument->AddDependent (this);
	
	DoUpdate (cSetTargetDisk, itsDocument, ((XPFPrefs *) itsDocument)->getTargetDisk (), NULL);
	DoUpdate (cSetRebootInMacOS9, itsDocument, NULL, NULL);
}

void 
XPFRestartButton::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	if (eventNumber == mButtonHit) {
		XPFPrefs *prefs = (XPFPrefs *) GetDocument ();
		short retVal = prefs->PoseConfirmDialog (false, false);
		if (retVal == kStdOkItemIndex) {
			PostCommand (prefs->MakeSaveCommand ());		
			PostCommand (TH_new XPFRestartCommand (prefs, true));
		}
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

bool
XPFRestartButton::determineActiveState ()
{
	XPFPrefs *prefs = (XPFPrefs *) GetDocument ();
	if (prefs->getRebootInMacOS9 ()) return true;
	MountedVolume *targetDisk = prefs->getTargetDisk ();
	if (!targetDisk) return false;
	if (targetDisk->getBootStatus () != kStatusOK) return false;
	if (prefs->getTooBigForNVRAM (false)) return false;
	return true;
}

void
XPFRestartButton::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	#pragma unused (theChange, changedObject, changeData, dependencySpace)

	// Basically, any update from fPrefs could change our active state
	SetActiveState (determineActiveState (), true);
}

//========================================================================================
// CLASS XPFMacOS9Button
//========================================================================================

#undef Inherited
#define Inherited TRadio

MA_DEFINE_CLASS (XPFMacOS9Button);

void 
XPFMacOS9Button::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);		
	itsDocument->AddDependent (this);
	DoUpdate (cSetRebootInMacOS9, itsDocument, NULL, NULL);
}

void 
XPFMacOS9Button::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	if (eventNumber == mRadioHit) {
		((XPFPrefs *) GetDocument ())->setRebootInMacOS9 (true);
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

void
XPFMacOS9Button::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	switch (theChange) {
	
		case cSetRebootInMacOS9:
			DoEvent (((XPFPrefs *) GetDocument ())->getRebootInMacOS9 () ? mTurnOn : mTurnOff, this, NULL);
			break;
			
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

//========================================================================================
// CLASS XPFMacOSXButton
//========================================================================================

#undef Inherited
#define Inherited TRadio

MA_DEFINE_CLASS (XPFMacOSXButton);

void 
XPFMacOSXButton::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);		
	itsDocument->AddDependent (this);
	DoUpdate (cSetRebootInMacOS9, itsDocument, NULL, NULL);
}

void 
XPFMacOSXButton::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	if (eventNumber == mRadioHit) {
		((XPFPrefs *) GetDocument ())->setRebootInMacOS9 (false);
	} else {
		Inherited::DoEvent (eventNumber, source, event);
	}
}

void
XPFMacOSXButton::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	switch (theChange) {
	
		case cSetRebootInMacOS9:
			DoEvent (((XPFPrefs *) GetDocument ())->getRebootInMacOS9 () ? mTurnOff : mTurnOn, this, NULL);
			break;
			
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}
