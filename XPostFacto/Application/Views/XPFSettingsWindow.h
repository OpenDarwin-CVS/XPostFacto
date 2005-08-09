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


#ifndef __XPFSETTINGSWINDOW_H__
#define __XPFSETTINGSWINDOW_H__

#include "UWindow.h"
#include "USlider.h"
#include "XPFIODevice.h"

class XPFApplication;
class XPFPrefs;


class XPFSettingsWindow : public TWindow
{
	MA_DECLARE_CLONE;

public:

	virtual void DoPostCreate(TDocument* itsDocument);
	virtual void DoEvent(EventNumber eventNumber,
						TEventHandler* source,
						TEvent* event);
	void DoUpdate(ChangeID_AC theChange, 
								MDependable_AC* changedObject,
								void* changeData,
								CDependencySpace_AC* dependencySpace);
	
private:

	void constructIODeviceMenu (TPopup *menu, const XPFIODeviceList *list);
	
	XPFApplication *fApp;
	XPFPrefs *fPrefs;

	TStaticText *fThrottleText;

	TPopup *fInputDeviceMenu;
	TPopup *fOutputDeviceMenu;
	
	TCheckBox *fToggleDebugBreakpoint;
	TCheckBox *fToggleDebugPrint;
	TCheckBox *fToggleDebugNMI;
	TCheckBox *fToggleDebugkprintf;
	TCheckBox *fToggleDebugUseDDB;
	TCheckBox *fToggleDebugSystemLog;
	TCheckBox *fToggleDebugARP;
	TCheckBox *fToggleDebugOldGDB;
	TCheckBox *fToggleDebugPanicText;
	
	TCheckBox *fSingleUserMode;
	TCheckBox *fVerboseMode;
	
	TCheckBox *fUseROMNDRV;
	
	TCheckBox *fAutoBoot;
	TCheckBox *fEnableCacheEarly;
	TCheckBox *fUsePatchedRagePro;

	TSlider *fThrottle;
};

#endif
