/*

Copyright (c) 2001, 2002
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


#include "XPFWindow.h"
#include "XPFApplication.h"
#include "XPFStrings.h"

#include <iostream.h>

#include "MountedVolume.h"
#include "XPFVersion.h"
#include "XPFPrefs.h"
#include "XPostFacto.h"

//========================================================================================
// CLASS XPFWindow
//========================================================================================

MA_DEFINE_CLASS(XPFWindow);

XPFWindow::~XPFWindow()
{
	RemoveAllDependencies();
}

void 
XPFWindow::DoPostCreate(TDocument* itsDocument)
{
	TWindow::DoPostCreate (itsDocument);
	fStartupMenu = (TPopup *) this->FindSubView ('Star');
	fRestartButton = (TButton *) this->FindSubView ('Rest');
	fInstallMenu = (TPopup *) this->FindSubView ('Into');
	fInstallButton = (TButton *) this->FindSubView ('Inst');
	fMessageText = (TStaticText *) this->FindSubView ('Mess');
	fInstallCluster = (TView *) this->FindSubView ('Clus');
	fInstallMessageText = (TStaticText *) this->FindSubView ('Inme');
	
	CStr255_AC title ("XPostFacto ");
	title += kXPFVersion;
	this->SetTitle (title);
	
	fApp = (XPFApplication *) gApplication;
	fPrefs = fApp->getPrefs ();
	
	fApp->AddDependent (this);
	fPrefs->AddDependent (this);

	updateControlStatus ();
	updateBootMessage ();
	updateInstallMessage ();
	constructVolumeList ();
	
	fInstallMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getInstallDisk ()), true); 
	fStartupMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getBootDisk ()), true); 
}

void 
XPFWindow::Close()
{
	TWindow::Close ();
	if (!gApplication->GetDone ()) gApplication->DoMenuCommand (cQuit);
}

void 
XPFWindow::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	TWindow::DoEvent (eventNumber, source, event);
	if ((eventNumber == mPopupHit) && (source == fStartupMenu)) {
		fPrefs->setBootDisk (MountedVolume::GetVolumeList()->At (fStartupMenu->GetCurrentItem()));
	} else if ((eventNumber == mButtonHit) && (source == fRestartButton)) {
		fApp->restart ();
	} else if ((eventNumber == mPopupHit) && (source == fInstallMenu)) {
		fPrefs->setInstallDisk (MountedVolume::GetVolumeList()->At (fInstallMenu->GetCurrentItem()));
	} else if ((eventNumber == mButtonHit) && (source == fInstallButton)) {
		fApp->install ();
	}
}

void
XPFWindow::updateControlStatus ()
{
	unsigned bootStatus = fPrefs->getBootDisk()->getBootStatus ();
	unsigned installerStatus = fPrefs->getBootDisk()->getInstallerStatus ();
	unsigned installTargetStatus = installerStatus;
	if (installTargetStatus == kStatusOK) installTargetStatus = fPrefs->getInstallDisk()->getInstallTargetStatus ();
	if (fApp->getCopyInProgress()) {	
		fStartupMenu->SetActiveState (false, true);
		fRestartButton->SetActiveState (false, true);
		fInstallMenu->SetActiveState (false, true);
		fInstallButton->SetActiveState (false, true);
		fMessageText->SetActiveState (true, true);
		fInstallCluster->SetActiveState (true, true);
		fInstallMessageText->SetActiveState (true, true);
	} else {
		fStartupMenu->SetActiveState (true, false);
		fRestartButton->SetActiveState (bootStatus == kStatusOK, true);
		fInstallMenu->SetActiveState (installerStatus == kStatusOK, true);
		fInstallButton->SetActiveState (installTargetStatus == kStatusOK, true);
		fMessageText->SetActiveState (true, true);
		fInstallCluster->SetActiveState (installerStatus == kStatusOK, true);
		fInstallMessageText->SetActiveState (installerStatus == kStatusOK, true);
	}
}

void
XPFWindow::updateBootMessage ()
{
	CStr255_AC reason ("");	
	CStr255_AC message ("");
		
	unsigned bootStatus = fPrefs->getBootDisk()->getBootStatus ();

	switch (bootStatus) {
		case kNotHFSPlus:
		case kNoMachKernel:
		case kNotSCSI:
		case kNoOFName:
		case kNoBootX:
			if (fPrefs->getBootDisk()->getInstallerStatus() == kStatusOK) break;
			GetIndString (reason, kXPFStringsResource, bootStatus);
			message = "'";
			message += fPrefs->getBootDisk()->getVolumeName ();
			message += "' ";
			message += reason;
			break;
			
		case kStatusOK:
			message = "boot-device: ";
			message += fPrefs->getBootDevice ();
			message += "\nboot-command: ";
			message += fPrefs->getBootCommand ();
			message += "\nauto-boot?: ";
			message += fPrefs->getAutoBoot () ? "true" : "false";
				
			CStr255_AC inputDevice = fPrefs->getInputDevice ();
			CStr255_AC outputDevice = fPrefs->getOutputDevice ();
			if (inputDevice != "") {
				message += "\ninput-device: ";
				message += inputDevice;
			}	
			if (outputDevice != "") {
				message += "\noutput-device: ";
				message += outputDevice;
			}
			break;

		default:
			message = "Error condition ";
			message += bootStatus;
			break;

	}
	
	fMessageText->SetText (message, true);
}

void
XPFWindow::updateInstallMessage ()
{
	CStr255_AC reason ("");	
	CStr255_AC message ("");
		
	unsigned installStatus = fPrefs->getBootDisk()->getInstallerStatus ();

	switch (installStatus) {			
		case kNotHFSPlus:
		case kNoMachKernel:
		case kNotSCSI:
		case kNoOFName:
		case kNoBootX:
		case kNotInstaller:
			GetIndString (reason, kXPFStringsResource, installStatus);
			message = "'";
			message += fPrefs->getBootDisk()->getVolumeName ();
			message += "' ";
			message += reason;
			break;
		
		case kStatusOK:
		
			unsigned targetStatus  = fPrefs->getInstallDisk()->getInstallTargetStatus();
			switch (targetStatus) {
				case kNotHFSPlus:
				case kNotWriteable:
				case kNotSCSI:
				case kNoOFName:
					GetIndString (reason, kXPFStringsResource, targetStatus);
					message = "'";
					message += fPrefs->getInstallDisk()->getVolumeName ();
					message += "' ";
					message += reason;
					break;

				case kStatusOK:
					message = "boot-device: ";
					message += fPrefs->getBootDeviceForInstall ();
					message += "\nboot-file: ";
					message += fPrefs->getBootFileForInstall ();
					message += "\nboot-command: ";
					message += fPrefs->getBootCommandForInstall ();
					break;
					
				default:
					message = "Error condition ";
					message += targetStatus;
					break;
			}
			break;

		default:
			message = "Error condition ";
			message += installStatus;
			break;

	}
	
	fInstallMessageText->SetText (message, true);
}

void
XPFWindow::constructVolumeList ()
{
	fStartupMenu->DeleteAll ();
	fInstallMenu->DeleteAll ();
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current(); iter.Next()) {
		fStartupMenu->AddItem (iter->getVolumeName ());
		fInstallMenu->AddItem (iter->getVolumeName ());
	}
	fInstallMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getInstallDisk ()), true); 
	fStartupMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getBootDisk ()), true); 
}

void
XPFWindow::DoUpdate(ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	#pragma unused (changedObject, changeData, dependencySpace)
	
	// if we are quitting, then we stop updating the UI
	if (fApp->GetDone ()) return;	
	
	switch (theChange) {
		case cSetCopyInProgress:
			updateControlStatus ();
			break;
			
		case cSetCopyMessage:
			CStr255_AC message ("Copying ");
			message += fApp->getCopyingFile ();
			fInstallMessageText->SetText (message, true);
			break;
		
		case cVolumeListChange:
			constructVolumeList ();
			break;

		case cSetInstallDisk:
			fInstallMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getInstallDisk ()), true); 
			updateControlStatus ();
			updateBootMessage ();
			updateInstallMessage ();
			break;
			
		case cSetBootDisk:
			fStartupMenu->SetCurrentItem (MountedVolume::GetVolumeList()->GetIdentityItemNo (fPrefs->getBootDisk ()), true); 
			updateControlStatus ();
			updateBootMessage ();
			updateInstallMessage ();
			break;
			
		case cToggleVerboseMode:
		case cToggleSingleUserMode:
		case cToggleDebugBreakpoint:
		case cToggleDebugPrint:
		case cToggleDebugNMI:
		case cToggleDebugkprintf:
		case cToggleDebugUseDDB:
		case cToggleDebugSystemLog:
		case cToggleDebugARP:
		case cToggleDebugOldGDB:
			updateBootMessage ();
			updateInstallMessage ();
			break;

	}
}


