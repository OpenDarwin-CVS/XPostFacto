/*

Copyright (c) 2003, 2005
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


#include "XPFVolumeList.h"
#include "XPFApplication.h"
#include "XPFStrings.h"

#include "MountedVolume.h"
#include "XPFVersion.h"
#include "XPFPrefs.h"
#include "XPostFacto.h"
#include "XPFLog.h"
#include "XPFIODevice.h"
#include "XPFVolumeDisplay.h"
#include "XPFVolumeInspectorWindow.h"

//========================================================================================
// CLASS XPFVolumeList
//========================================================================================

#define Inherited TScroller

void 
XPFVolumeList::DoPostCreate(TDocument* itsDocument)
{
	fTrackSelection = NULL;
	fPrefs = (XPFPrefs *) GetDocument ();
	
	fLastUpTime = 0;

	TScroller::DoPostCreate (itsDocument);
	
	fApp = (XPFApplication *) gApplication;
	
	// Create the initial views. We'll catch changes through the "update" thingy.
	
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		DoUpdate (cNewMountedVolume, fPrefs, iter.Current (), NULL);
	}
	
	fApp->AddDependent (this);
	fPrefs->AddDependent (this);
		
	DoUpdate (cSetTargetDisk, fPrefs, fPrefs->getTargetDisk (), NULL);
	DoUpdate (cSetInstallCD, fPrefs, fPrefs->getInstallCD (), NULL);
	DoUpdate (cSetMacOS9Disk, fPrefs, fPrefs->getMacOS9Disk (), NULL);
	DoUpdate (cSetRebootInMacOS9, fPrefs, NULL, NULL);
}

void 
XPFVolumeList::DoMouseCommand(CViewPoint&		theMouse,
						   TToolboxEvent*	event,
						   CPoint_AC		hysteresis)
{
	#pragma unused (event, hysteresis)

	fTrackSelection = NULL;
	for (CSubViewIterator iter (this); iter; ++iter) {
		if (iter->AdornerWithID (kSelectionAdorner)) {
			if (iter->IsActive ()) fTrackSelection = iter.Current ();
			break;
		}
	}

	TTracker *tracker = TH_new TTracker (cTrackingControl, this, kCantUndo, kDoesNotCauseChange, 
				NULL, this, this, theMouse);
	tracker->fTrackNonMovement = true;
	tracker->fViewConstrain = false;
	PostCommand (tracker);
}

void 
XPFVolumeList::DoMouseUp (CViewPoint &theMouse, TToolboxEvent* event, CPoint_AC hysteresis)
{
	if (
		((event->GetWhen() - fLastUpTime) < GetDblTime ()) &&
		(abs (fLastMousePoint.h - event->GetWhere().h) <= GetStdHysteresis().h) && 
		(abs (fLastMousePoint.v - event->GetWhere().v) <= GetStdHysteresis().v)
	) {
		TView *selection = NULL;
		for (CSubViewIterator iter (this); iter; ++iter) {
			if (iter->GetFrame ().Contains (theMouse)) {
				if (iter->IsActive ()) selection = iter.Current ();
				break;
			}
		}
		if (selection) XPFVolumeInspectorWindow::ShowInspectorForVolume (((XPFVolumeDisplay *) selection)->getVolume ());
	}
	
	fLastMousePoint = event->GetWhere ();
	fLastUpTime = event->GetWhen ();	

	Inherited::DoMouseUp (theMouse, event, hysteresis);						   
}						   

void 
XPFVolumeList::TrackFeedback(TrackPhase			aTrackPhase,
							 const CViewPoint&	anchorPoint,
							 const CViewPoint&	previousPoint,
							 const CViewPoint&	nextPoint,
							 bool				mouseDidMove,
							 bool				turnItOn)
{
	#pragma unused (aTrackPhase, anchorPoint, previousPoint, nextPoint, mouseDidMove, turnItOn)
	
	// This is here so that the superclass doesn't track feedback.
}

void 
XPFVolumeList::TrackMouse(TrackPhase	aTrackPhase,
						  CViewPoint&	anchorPoint,
						  CViewPoint&	previousPoint,
						  CViewPoint&	nextPoint,
						  bool			mouseDidMove)
{
	#pragma unused (anchorPoint, previousPoint, mouseDidMove)

	switch (aTrackPhase) {
	
		case trackContinue:
			if (!mouseDidMove) return;
			// break intentionally omitted
			
		case trackBegin:
			for (CSubViewIterator iter (this); iter; ++iter) {
				if (iter->GetFrame ().Contains (nextPoint)) {
					if (iter->IsActive () && (iter.Current () != fTrackSelection)) {
						if (fTrackSelection) fTrackSelection->DoHighlightSelection (hlOn, hlOff);
						iter->DoHighlightSelection (hlOff, hlOn);
						fTrackSelection = iter.Current ();
					}
					break;
				}
			}
			break;
			
		case trackEnd:
			for (CSubViewIterator iter (this); iter; ++iter) {
				if (iter->GetFrame ().Contains (nextPoint)) {
					if (iter->IsActive ()) fTrackSelection = iter.Current ();
					break;
				}
			}
			if (fTrackSelection) handleUserSelectedVolume (((XPFVolumeDisplay *) fTrackSelection)->getVolume ());
			fTrackSelection = NULL;
			break;
	}
}

void
XPFVolumeList::DoUpdate (ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace)
{
	MountedVolume *volume = (MountedVolume *) changeData;
	
	if (isSetSelectionCommandNumber (theChange)) {
		for (CSubViewIterator iter (this); iter; ++iter) {
			iter->DoUpdate (cSetSelectedVolume, changedObject, changeData, dependencySpace);		
		}
	}
	
	switch (theChange) {
				
		case cNewMountedVolume:
			if (useVolumeInList (volume)) {
				CViewPoint offset (0, fScrollLimit.v);
				XPFVolumeDisplay *display = (XPFVolumeDisplay *) TViewServer::fgViewServer->DoCreateViews (GetDocument (), this, kVolumeDisplay, offset);
				display->setVolume (volume);
				this->SetScrollParameters (display->GetSize (), false, true);
				display->ScrollSelectionIntoView (false);
				// Make sure that we highlight the disk if it is the default
				if (volume == fPrefs->getTargetDisk ()) DoUpdate (cSetTargetDisk, changedObject, changeData, dependencySpace);
				if (volume == fPrefs->getMacOS9Disk ()) DoUpdate (cSetMacOS9Disk, changedObject, changeData, dependencySpace);
				if (volume == fPrefs->getInstallCD ()) DoUpdate (cSetInstallCD, changedObject, changeData, dependencySpace);
				this->ForceRedraw ();
			}
			break;
			
		case cSetRebootInMacOS9:
			if (fPrefs->getRebootInMacOS9 ()) {
				DoUpdate (cSetMacOS9Disk, fPrefs, fPrefs->getMacOS9Disk (), NULL);
			} else {
				DoUpdate (cSetTargetDisk, fPrefs, fPrefs->getTargetDisk (), NULL);		
			}
			break;
		
		default:
			Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
			break;
	}
}

void 
XPFVolumeList::ScrollBy (const CViewPoint& delta, bool redraw)
{
	Inherited::ScrollBy (delta, redraw);
	if (redraw && fTrackSelection) fTrackSelection->DoHighlightSelection (hlOff, hlOn);
}

// -------------------
// XPFTargetVolumeList
// -------------------

MA_DEFINE_CLASS(XPFTargetVolumeList);

#undef Inherited
#define Inherited XPFVolumeList

bool
XPFTargetVolumeList::useVolumeInList (MountedVolume *volume)
{
	return !volume->getHasInstaller ();
}

void 
XPFTargetVolumeList::handleUserSelectedVolume (MountedVolume *vol)
{
	if (fPrefs->getRebootInMacOS9 ()) {
		fPrefs->setMacOS9Disk (vol);
	} else {
		fPrefs->setTargetDisk (vol);
	}
}

bool
XPFTargetVolumeList::isSetSelectionCommandNumber (CommandNumber command)
{
	return command == fPrefs->getRebootInMacOS9 () ? cSetMacOS9Disk : cSetTargetDisk;
}

// -------------------
// XPFInstallCDList
// -------------------

MA_DEFINE_CLASS(XPFInstallCDList);

#undef Inherited
#define Inherited XPFVolumeList

bool
XPFInstallCDList::useVolumeInList (MountedVolume *volume)
{
	return volume->getHasInstaller ();
}

void 
XPFInstallCDList::handleUserSelectedVolume (MountedVolume *vol)
{
	fPrefs->setInstallCD (vol);
}

void
XPFInstallCDList::hideMiddleView ()
{
	if (fMiddleView->GetShown ()) {
		fMiddleView->Show (false, false);
	
		TWindow *myWindow = GetWindow ();
		CViewPoint middleSize = fMiddleView->GetSize ();
		
		CViewPoint bottomLocation = fBottomView->GetLocation ();
		bottomLocation.v -= middleSize.v;
		fBottomView->Locate (bottomLocation, true);

		CViewPoint windowSize = myWindow->GetSize ();
		windowSize.v -= middleSize.v;
		myWindow->Resize (windowSize, true);		
	}
}

void
XPFInstallCDList::showMiddleView ()
{
	if (!fMiddleView->GetShown ()) {
		TWindow *myWindow = GetWindow ();
		CViewPoint middleSize = fMiddleView->GetSize ();
		
		CViewPoint windowSize = myWindow->GetSize ();
		windowSize.v += middleSize.v;
		myWindow->Resize (windowSize, true);
		
		CViewPoint bottomLocation = fBottomView->GetLocation ();
		bottomLocation.v += middleSize.v;
		fBottomView->Locate (bottomLocation, true);
		
		fMiddleView->Show (true, true);
	}
}

void 
XPFInstallCDList::DoPostCreate (TDocument* itsDocument)
{
	fMiddleView = GetWindow ()->FindSubView ('invw');
	fBottomView = GetWindow ()->FindSubView ('bovw');
	
	Inherited::DoPostCreate (itsDocument);

	if (!HasSubViews ()) hideMiddleView ();
}

void 
XPFInstallCDList::AddedASubView (TView* theSubView)
{
	Inherited::AddedASubView (theSubView);
	showMiddleView ();
}

void 
XPFInstallCDList::RemovedASubView (TView* theSubView)
{
	Inherited::RemovedASubView (theSubView);
	if (!HasSubViews ()) hideMiddleView ();
}

void 
XPFInstallCDList::DoUpdate (ChangeID_AC theChange, MDependable_AC* changedObject, void* changeData, CDependencySpace_AC* dependencySpace)
{
	if (theChange == cSetRebootInMacOS9) {
		if (fPrefs->getRebootInMacOS9 ()) {
			hideMiddleView ();
		} else {
			if (HasSubViews ()) showMiddleView ();
		}	
	}
	
	Inherited::DoUpdate (theChange, changedObject, changeData, dependencySpace);
}
