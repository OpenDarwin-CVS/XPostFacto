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

#include "XPFSettingsText.h"
#include "XPFPrefs.h"
#include "MountedVolume.h"
#include "XPostFacto.h"
#include "XPFStrings.h"

//========================================================================================
// CLASS XPFSettingsText
//========================================================================================

#undef Inherited
#define Inherited TStaticText

MA_DEFINE_CLASS (XPFSettingsText);

void 
XPFSettingsText::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	itsDocument->AddDependent (this);
	
	DoUpdate (cSetTargetDisk, itsDocument, ((XPFPrefs *) itsDocument)->getTargetDisk (), NULL);
}

void
XPFSettingsText::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	// Basically, any change broadcast by fPrefs interests us, so we don't need to check
	#pragma unused (theChange, changedObject, changeData, dependencySpace)
	
	CStr255_AC message;
	XPFPrefs *prefs = (XPFPrefs *) GetDocument ();
	
	MountedVolume *target = prefs->getRebootInMacOS9 () ? prefs->getMacOS9Disk () : prefs->getTargetDisk ();
	MountedVolume *installCD = prefs->getInstallCD ();
	
	if (target) {
		bool forInstall = !prefs->getRebootInMacOS9 () && (target->getInstallTargetStatus () == kStatusOK) && installCD;

		message += "boot-device: ";
		message += prefs->getBootDevice (forInstall);
		message += "\nboot-file: ";
		message += prefs->getBootFile (forInstall);
		if (prefs->getRebootInMacOS9 ()) {
			message += "\nboot-command: ";
			message += prefs->getBootCommand ();
		} else {
			message += "\nboot-args: ";
			message += prefs->getBootArgs (forInstall);		
		}
		message += "\nauto-boot?: ";
		message += prefs->getAutoBoot () ? "true" : "false";
		message += "\ninput-device: ";
		message += prefs->getInputDevice (forInstall);
		message += "\noutput-device: ";
		message += prefs->getOutputDevice (forInstall);
	}
	
	SetText (message, true);
}
