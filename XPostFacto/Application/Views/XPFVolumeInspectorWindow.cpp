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


#include "XPFVolumeInspectorWindow.h"
#include "XPFStrings.h"

#include "MountedVolume.h"
#include "XPostFacto.h"
#include "XPFLog.h"
#include "vers_rsrc.h"
#include "XPFUpdate.h"
#include "XPFUpdateCommands.h"
#include "XPFApplication.h"
#include "XPFPrefs.h"

#include <stdio.h>

//========================================================================================
// CLASS XPFVolumeInspectorWindow
//========================================================================================

MA_DEFINE_CLASS(XPFVolumeInspectorWindow);

XPFVolumeInspectorWindow *XPFVolumeInspectorWindow::gInstance = NULL;

#define Inherited TWindow

void
XPFVolumeInspectorWindow::ShowInspectorForVolume (MountedVolume *volume)
{
	if (gInstance) {
		gInstance->Show (true, false);
		gInstance->Select ();
	} else {
		gInstance = (XPFVolumeInspectorWindow *) TViewServer::fgViewServer->NewTemplateWindow (kVolumeInspectorWindow, NULL);
		gInstance->Open ();
	}

	if (volume) {
		gInstance->setVolume (volume);
	} else {
		if (!gInstance->fVolume) gInstance->setVolume (MountedVolume::GetVolumeList ()->First ());
	}
}

XPFVolumeInspectorWindow::XPFVolumeInspectorWindow (WindowRef itsWMgrWindow, bool canResize, bool canClose, bool inDispose)
	: fVolume (NULL), TWindow (itsWMgrWindow, canResize, canClose, inDispose)	
{
}

XPFVolumeInspectorWindow::~XPFVolumeInspectorWindow()
{
	gInstance = NULL;
}

void 
XPFVolumeInspectorWindow::DoPostCreate(TDocument* itsDocument)
{
	fVolumeName = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('voln'));
	fOpenFirmwareName = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('ofnm'));
	fMacOSXVersion = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('mosv'));
	fBootXVersion = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('boov'));
	fMacOS9FolderID = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('mos9'));
	fBlessedFolderID = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('blsf'));

	fWarningIcon = dynamic_cast_or_throw_AC (TIcon*, this->FindSubView ('warn'));
	fWarningText = dynamic_cast_or_throw_AC (TStaticText*, this->FindSubView ('wart'));
	
	fPrefs = ((XPFApplication *) gApplication)->getPrefs ();
	fPrefs->AddDependent (this);
	
	TWindow::DoPostCreate (itsDocument);
}

void
XPFVolumeInspectorWindow::setVolume (MountedVolume *volume)
{
	if (volume && volume != fVolume) {
		fVolume = volume;
		fVolume->AddDependent (this);
		Changed (cSetVolumeInspectorVolume, volume);
		updateFields ();
	}
}

void
XPFVolumeInspectorWindow::updateFields ()
{
	fVolumeName->SetText (fVolume->getVolumeName (), true);

	if (fVolume->getValidOpenFirmwareName ()) {
		fOpenFirmwareName->SetText (fVolume->getOpenFirmwareName (true), true);
	} else {
		fOpenFirmwareName->SetTextWithStrListID (kXPFStringsResource, kNoOFName, true);
	}

	if (fVolume->getMacOSXMajorVersion ()) {
		fMacOSXVersion->SetText (fVolume->getMacOSXVersion (), true);
	} else {
		fMacOSXVersion->SetTextWithStrListID (kXPFStringsResource, kNotInstalled, true);
	}
	
	UInt32 bootXVersion = fVolume->getActiveBootXVersion ();
	if (bootXVersion == kBootXNotInstalled) {
		fBootXVersion->SetTextWithStrListID (kXPFStringsResource, kNotInstalled, true);	
	} else if (bootXVersion == kBootXImproperlyInstalled) {
		fBootXVersion->SetTextWithStrListID (kXPFStringsResource, kRequiresInstallation, true);
	} else {
		char vers [32];
		VERS_string (vers, 31, bootXVersion);
		fBootXVersion->SetText (vers, true);	
	}
	
	char buffer[32];
	UInt32 macos9folderID = fVolume->getMacOS9SystemFolderNodeID ();
	if (macos9folderID) {
		sprintf (buffer, "%lu", macos9folderID);
		fMacOS9FolderID->SetText (buffer, true);
	} else {
		fMacOS9FolderID->SetTextWithStrListID (kXPFStringsResource, kNotInstalled, true);		
	}
	
	sprintf (buffer, "%lu", fVolume->getBlessedFolderID ());
	fBlessedFolderID->SetText (buffer, true);
	
	UInt32 warning = fVolume->getBootWarning (fPrefs->getInstallCD ());
	if (warning) {
		fWarningIcon->Show (true, true);
		fWarningText->SetTextWithStrListID (kXPFStringsResource, warning, true);
	} else {
		fWarningIcon->Show (false, true);
		fWarningText->SetText ("", true);
	}
}

void 
XPFVolumeInspectorWindow::DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace)
{
	if ((changedObject == fVolume) || (changedObject == fPrefs)) updateFields ();
	Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
}

void 
XPFVolumeInspectorWindow::DoSetupMenus ()
{
	Inherited::DoSetupMenus ();
	
	bool enable = fVolume && fVolume->getIsWriteable ();
	
	Enable (cInstallBootX, enable);
	Enable (cInstallExtensions, enable);
	Enable (cInstallStartupItem, enable);
	Enable (cInstallEverything, enable);
	Enable (cRecopyHelperFiles, enable && fVolume->getHelperDisk () != NULL);
	Enable (cUninstall, enable);
	Enable (cBlessMacOS9SystemFolder, enable && fVolume->getMacOS9SystemFolderNodeID ());
	Enable (cEmptyCache, enable);
	Enable (cCheckPermissions, enable);
	Enable (cFixSymlinks, enable && fVolume->getSymlinkStatus () != kSymlinkStatusCannotFix);
}

void 
XPFVolumeInspectorWindow::DoMenuCommand (CommandNumber aCommandNumber)
{
	XPFUpdate *update = NULL;

	switch (aCommandNumber) 
	{			
		case cInstallBootX:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFInstallBootXCommand (update));
			break;
			
		case cInstallExtensions:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFInstallExtensionsCommand (update));
			break;
		
		case cInstallStartupItem:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFInstallStartupCommand (update));
			break;
			
		case cInstallEverything:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFInstallBootXCommand (update));
			PerformCommand (TH_new XPFInstallExtensionsCommand (update));
			PerformCommand (TH_new XPFInstallStartupCommand (update));
			break;
						
		case cRecopyHelperFiles:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFRecopyHelperFilesCommand (update));
			break;
			
		case cBlessMacOS9SystemFolder:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFBlessMacOS9SystemFolderCommand (update));
			break;
			
		case cUninstall:
			MAParamText ("$VOLUME$", fVolume->getVolumeName ());
				
			TWindow *dialog = TViewServer::fgViewServer->NewTemplateWindow (kUninstallDialog, NULL);
			IDType result = dialog->PoseModally ();
			dialog->Close ();
				
			if (result == 'chan') {
				update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
				PerformCommand (TH_new XPFUninstallCommand (update));			
			}
			break;
			
		case cEmptyCache:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFEmptyCacheCommand (update));
			break;
			
		case cCheckPermissions:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFCheckPermissionsCommand (update));
			break;
						
		case cFixSymlinks:
			update = new XPFUpdate (fVolume, fVolume->getHelperDisk ());
			PerformCommand (TH_new XPFFixSymlinksCommand (update));
			break;
						
		default:
			Inherited::DoMenuCommand (aCommandNumber);
			break;
	}
	
	if (update) delete update;
}

