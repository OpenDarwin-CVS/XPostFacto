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

//========================================================================================
// CLASS XPFVolumeList
//========================================================================================

MA_DEFINE_CLASS(XPFVolumeList);

#define Inherited TScroller

XPFVolumeList::~XPFVolumeList()
{
	RemoveAllDependencies();
}

void 
XPFVolumeList::DoPostCreate(TDocument* itsDocument)
{
	fTrackSelection = NULL;
	fPrefs = (XPFPrefs *) GetDocument ();

	TScroller::DoPostCreate (itsDocument);
	
	fApp = (XPFApplication *) gApplication;
	
	// Create the initial views. We'll catch changes through the "update" thingy.
	
	for (MountedVolumeIterator iter (MountedVolume::GetVolumeList ()); iter.Current (); iter.Next ()) {
		DoUpdate (cNewMountedVolume, fPrefs, iter.Current (), NULL);
	}
	
	fApp->AddDependent (this);
	
	for (CSubViewIterator iter (this); iter; ++iter) {
		iter->DoUpdate (cSetTargetDisk, fPrefs, fPrefs->getTargetDisk (), NULL);
	}
}

void 
XPFVolumeList::DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event)
{
	TScroller::DoEvent (eventNumber, source, event);	
}

void 
XPFVolumeList::DoMouseCommand(CViewPoint&		theMouse,
						   TToolboxEvent*	event,
						   CPoint_AC		hysteresis)
{
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
XPFVolumeList::TrackFeedback(TrackPhase			aTrackPhase,
							 const CViewPoint&	anchorPoint,
							 const CViewPoint&	previousPoint,
							 const CViewPoint&	nextPoint,
							 bool				mouseDidMove,
							 bool				turnItOn)
{
	#pragma unused (aTrackPhase, anchorPoint, previousPoint, nextPoint, mouseDidMove, turnItOn)
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
			if (fTrackSelection) fTrackSelection->HandleEvent (mListItemHit, this, NULL);
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
	ArrayIndex_AC index;
	XPFVolumeDisplay *subview, *subviewToRemove;
	
	switch (theChange) {
				
		case cNewMountedVolume:
			CViewPoint offset (0, fScrollLimit.v);
			XPFVolumeDisplay *display = (XPFVolumeDisplay *) TViewServer::fgViewServer->DoCreateViews (GetDocument (), this, 1200, offset);
			display->setVolume (volume);
			this->SetScrollParameters (display->GetSize (), false, true);
			display->ScrollSelectionIntoView (false);
			this->ForceRedraw ();
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

