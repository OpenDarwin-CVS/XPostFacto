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


#include "XPFSettingsWindow.h"
#include "XPFApplication.h"
#include "XPFStrings.h"

#include <iostream.h>

#include "MountedVolume.h"
#include "XPFVersion.h"
#include "XPFPrefs.h"
#include "XPostFacto.h"
#include "XPFLog.h"
#include "XPFIODevice.h"

#include "StdStringUtils_AC.h"

//========================================================================================
// CLASS XPFSettingsWindow
//========================================================================================

MA_DEFINE_CLASS(XPFSettingsWindow);

XPFSettingsWindow::~XPFSettingsWindow()
{
	RemoveAllDependencies();
}

void
XPFSettingsWindow::constructIODeviceMenu (TPopup *menu, const XPFIODeviceList *list)
{
	menu->CreateMenu ();
	menu->AddItem ("None");
	menu->AddSeparator ();

	for (XPFIODeviceIterator iter (list); iter.Current(); iter.Next()) {
		menu->AddItem (iter->getLabel ());
	}
}

void 
XPFSettingsWindow::DoPostCreate(TDocument* itsDocument)
{
	TWindow::DoPostCreate (itsDocument);
	
	fInputDeviceMenu 	= dynamic_cast_or_throw_AC (TPopup*, this->FindSubView ('inpd'));
	fOutputDeviceMenu 	= dynamic_cast_or_throw_AC (TPopup*, this->FindSubView ('outd'));	
	
	constructIODeviceMenu (fInputDeviceMenu, XPFIODevice::GetInputDeviceList ());
	constructIODeviceMenu (fOutputDeviceMenu, XPFIODevice::GetOutputDeviceList ());

	fHelpText = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('hlpt'));
	fThrottleText = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('thrt'));
	
	fToggleDebugBreakpoint = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('debr'));
	fToggleDebugPrint = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('depr'));
	fToggleDebugNMI = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('denm'));
	fToggleDebugkprintf = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('dekp'));
	fToggleDebugUseDDB = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('dudd'));
	fToggleDebugSystemLog = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('dedi'));
	fToggleDebugARP = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('deda'));
	fToggleDebugOldGDB = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('deso'));
	fToggleDebugPanicText = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('desp'));
	
	fSingleUserMode = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('sinm'));
	fVerboseMode = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('verm'));
	
	fAutoBoot = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('auto'));
	fEnableCacheEarly = dynamic_cast_or_throw_AC (TCheckBox*, this->FindSubView ('cach'));
	
	fThrottle = dynamic_cast_or_throw_AC (TSlider*, this->FindSubView ('thro'));

	fApp = (XPFApplication *) gApplication;
	fPrefs = (XPFPrefs *) GetDocument ();
	
	fApp->AddDependent (this);
	fPrefs->AddDependent (this);
	
	DoUpdate (cSetInputDevice, fPrefs, NULL, NULL);
	DoUpdate (cSetOutputDevice, fPrefs, NULL, NULL);
	DoUpdate (cSetDebugPanicText, fPrefs, NULL, NULL);
	DoUpdate (cSetThrottle, fPrefs, NULL, NULL);
}

void 
XPFSettingsWindow::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	TWindow::DoEvent (eventNumber, source, event);
	
	bool newValue;
		
	switch (eventNumber) {
		
		case mControlHit:
			if (source == fThrottle) {
				fPrefs->setThrottle (fThrottle->GetValue ());
			}
			break;

		case mPopupHit:
			if (source == fInputDeviceMenu) {
				fPrefs->setInputDevice ((CChar255_AC) fInputDeviceMenu->GetItemText (fInputDeviceMenu->GetCurrentItem ()));
			} else if (source == fOutputDeviceMenu) {
				fPrefs->setOutputDevice ((CChar255_AC) fOutputDeviceMenu->GetItemText (fOutputDeviceMenu->GetCurrentItem ()));
			}
			break;
	
		case mCheckBoxHit:
			newValue = ((TCheckBox *) source)->IsOn ();
			
			if (source == fToggleDebugBreakpoint) {
				fPrefs->setDebugBreakpoint (newValue);
			} else if (source == fToggleDebugPrint) {
				fPrefs->setDebugPrintf (newValue);
			} else if (source == fToggleDebugNMI) {
				fPrefs->setDebugNMI (newValue);
			} else if (source == fToggleDebugkprintf) {
				fPrefs->setDebugKprintf (newValue);
			} else if (source == fToggleDebugUseDDB) {
				fPrefs->setDebugDDB (newValue);
			} else if (source == fToggleDebugSystemLog) {
				fPrefs->setDebugSyslog (newValue);
			} else if (source == fToggleDebugARP) {
				fPrefs->setDebugARP (newValue);
			} else if (source == fToggleDebugOldGDB) {
				fPrefs->setDebugOldGDB (newValue);
			} else if (source == fToggleDebugPanicText) {
				fPrefs->setDebugPanicText (newValue);
			} else if (source == fVerboseMode) {
				fPrefs->setBootInVerboseMode (newValue);
			} else if (source == fSingleUserMode) {
				fPrefs->setBootInSingleUserMode (newValue);
			} else if (source == fEnableCacheEarly) {
				fPrefs->setEnableCacheEarly (newValue);
			} else if (source == fAutoBoot) {
				fPrefs->setAutoBoot (newValue);
			}
			break;
	}
}

void
XPFSettingsWindow::DoUpdate(ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	#pragma unused (changedObject, changeData, dependencySpace)
	
	// if we are quitting, then we stop updating the UI
	if (fApp->GetDone ()) return;	

	CStr255_AC message;
	unsigned index;
	
	CString_AC aString;
	
	switch (theChange) {
		case cSetInputDevice:
			index = fPrefs->getInputDeviceIndex ();
			if (index == 0) {
				fInputDeviceMenu->SetCurrentItem (1, true);
			} else {
				fInputDeviceMenu->SetCurrentItem (index + 2, true);
			}
			break;
			
		case cSetOutputDevice:
			index = fPrefs->getOutputDeviceIndex ();
			if (index == 0) {
				fOutputDeviceMenu->SetCurrentItem (1, true);
			} else {
				fOutputDeviceMenu->SetCurrentItem (index + 2, true);
			}
			break;
			
		case cSetThrottle:
			fThrottle->SetValue (fPrefs->getThrottle (), true);
			stringprintf (aString, "%d", fPrefs->getThrottle ());
			fThrottleText->SetText (aString, true);
			break;
				
		case cSetVerboseMode:
		case cSetSingleUserMode:
		case cSetEnableCacheEarly:
		case cSetAutoBoot:
		case cSetDebugBreakpoint:
		case cSetDebugPrintf:
		case cSetDebugNMI:
		case cSetDebugKprintf:
		case cSetDebugDDB:
		case cSetDebugSyslog:
		case cSetDebugARP:
		case cSetDebugOldGDB:
		case cSetDebugPanicText:
			fToggleDebugBreakpoint->SetValue (fPrefs->getDebugBreakpoint (), true);
			fToggleDebugPrint->SetValue (fPrefs->getDebugPrintf (), true);
			fToggleDebugNMI->SetValue (fPrefs->getDebugNMI (), true);
			fToggleDebugkprintf->SetValue (fPrefs->getDebugKprintf (), true);
			fToggleDebugUseDDB->SetValue (fPrefs->getDebugDDB (), true);
			fToggleDebugSystemLog->SetValue (fPrefs->getDebugSyslog (), true);
			fToggleDebugARP->SetValue (fPrefs->getDebugARP (), true);
			fToggleDebugOldGDB->SetValue (fPrefs->getDebugOldGDB (), true);
			fToggleDebugPanicText->SetValue (fPrefs->getDebugPanicText (), true);
			fVerboseMode->SetValue (fPrefs->getBootInVerboseMode (), true);
			fSingleUserMode->SetValue (fPrefs->getBootInSingleUserMode (), true);
			fEnableCacheEarly->SetValue (fPrefs->getEnableCacheEarly (), true);
			fAutoBoot->SetValue (fPrefs->getAutoBoot (), true);
			break;

	}
}
