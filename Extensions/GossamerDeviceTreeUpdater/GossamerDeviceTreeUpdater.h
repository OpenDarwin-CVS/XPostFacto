/*

 Copyright (c) 2003
 Other World Computing
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer as
 the first lines of this file unmodified.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 This software is provided by Other World Computing ``as is'' and any express or
 implied warranties, including, but not limited to, the implied warranties
 of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall Other World Computing or Ryan Rempel be liable for any direct, indirect,
 incidental, special, exemplary, or consequential damages (including, but
 not limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use of
 this software, even if advised of the possibility of such damage.
 
*/

#ifndef __GOSSAMERDEVICETREEUPDATER_H__
#define __GOSSAMERDEVICETREEUPDATER_H__

#include <IOKit/IOService.h>

class OpenPMUNVRAMController;

class GossamerDeviceTreeUpdater : public IOService
{
	OSDeclareDefaultStructors (GossamerDeviceTreeUpdater);
  
  	public:
		
		virtual bool start (IOService *provider);
		virtual void free ();
			
		static void updateDeviceTree (void *self);
		static bool updateApplePMU (void *self, void *refCon, IOService *applePMU);
		
	private:
	
		void adjustProperty (const char *key, const char *nameMatched);
		
	private:
	
		IONotifier *fNotifier;
		OpenPMUNVRAMController *fOpenPMUNVRAMController;
				
};

#endif