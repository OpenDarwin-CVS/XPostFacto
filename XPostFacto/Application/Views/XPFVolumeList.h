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


#ifndef __XPFVOLUMELIST_H__
#define __XPFVOLUMELIST_H__

#include "UScroller.h"

class XPFApplication;
class XPFPrefs;

class XPFVolumeList : public TScroller
{
	MA_DECLARE_CLONE;

public:

	virtual	~XPFVolumeList();		
	virtual void DoPostCreate(TDocument* itsDocument);
	
	virtual void DoEvent	(EventNumber eventNumber,
							TEventHandler* source,
							TEvent* event);
						
	void DoUpdate	(ChangeID_AC theChange, 
					MDependable_AC* changedObject,
					void* changeData,
					CDependencySpace_AC* dependencySpace);
					
	void DoMouseCommand (CViewPoint&		theMouse,
						TToolboxEvent*	event,
						CPoint_AC		hysteresis);	
						
	void TrackFeedback 	(TrackPhase			aTrackPhase,
					const CViewPoint&	anchorPoint,
					const CViewPoint&	previousPoint,
					const CViewPoint&	nextPoint,
					bool				mouseDidMove,
					bool				turnItOn);

	void TrackMouse		(TrackPhase	aTrackPhase,
					CViewPoint&	anchorPoint,
					CViewPoint&	previousPoint,
					CViewPoint&	nextPoint,
					bool		mouseDidMove);
					
	void ScrollBy (const CViewPoint& delta, bool redraw);

					
private:
	
	XPFApplication *fApp;
	XPFPrefs *fPrefs;
	TView *fTrackSelection;

};

#endif