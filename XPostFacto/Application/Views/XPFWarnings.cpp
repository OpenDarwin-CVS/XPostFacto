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

#include "XPFWarnings.h"
#include "XPFPrefs.h"
#include "XPostFacto.h"
#include "XPFStrings.h"

#include "XPFLog.h"

//========================================================================================
// CLASS XPFInstallWarnings
//========================================================================================

#undef Inherited
#define Inherited TStaticText

MA_DEFINE_CLASS (XPFInstallWarning);

void 
XPFInstallWarning::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	itsDocument->AddDependent (this);
	
	fIcon = dynamic_cast_AC (TIcon*, FindPeerView ('inwa'));
	
	DoUpdate (cNoCommand, itsDocument, NULL, NULL);
}

void
XPFInstallWarning::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{	
	#pragma unused (theChange, changedObject, changeData, dependencySpace)

	CStr255_AC message;
	bool tooBig = ((XPFPrefs *) GetDocument ())->getTooBigForNVRAM (true);
	if (tooBig) message.CopyFrom (kXPFStringsResource, kTooBigForNVRAMForInstall, 255);
	SetText (message, false);
	if (fIcon) fIcon->Show (tooBig, true);
}

//========================================================================================
// CLASS XPFRestartWarnings
//========================================================================================

#undef Inherited
#define Inherited TStaticText

MA_DEFINE_CLASS (XPFRestartWarning);

void 
XPFRestartWarning::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);
		
	itsDocument->AddDependent (this);
	
	fIcon = dynamic_cast_AC (TIcon*, FindPeerView ('rewa'));
	
	DoUpdate (cNoCommand, itsDocument, NULL, NULL);
}

void
XPFRestartWarning::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{	
	#pragma unused (theChange, changedObject, changeData, dependencySpace)

	CStr255_AC message;
	bool tooBig = ((XPFPrefs *) GetDocument ())->getTooBigForNVRAM (false);
	if (tooBig) message.CopyFrom (kXPFStringsResource, kTooBigForNVRAM, 255);
	SetText (message, false);
	if (fIcon) fIcon->Show (tooBig, true);
}

//========================================================================================
// CLASS XPFWarningIcon
//========================================================================================

#undef Inherited
#define Inherited TIcon

MA_DEFINE_CLASS (XPFWarningIcon);

IconRef XPFWarningIcon::gIconRef = NULL;

XPFWarningIcon::~XPFWarningIcon ()
{
	if (gIconRef) ReleaseIconRef (gIconRef);
}

void 
XPFWarningIcon::DoPostCreate (TDocument* itsDocument)
{
	Inherited::DoPostCreate (itsDocument);

	if (gIconRef) {
		AcquireIconRef (gIconRef);
	} else {
		GetIconRef (kOnSystemDisk, kSystemIconsCreator, kAlertCautionIcon, &gIconRef);
	}

	if (gIconRef) {
		IconFamilyHandle familyHandle;
		IconSuiteRef iconSuite;
		OSErr err = IconRefToIconFamily (gIconRef, kSelectorAllAvailableData, &familyHandle);
		if (err == noErr) err = IconFamilyToIconSuite (familyHandle, kSelectorAllAvailableData, &iconSuite);
		if (err == noErr) SetIconSuite (iconSuite);
	}
}
