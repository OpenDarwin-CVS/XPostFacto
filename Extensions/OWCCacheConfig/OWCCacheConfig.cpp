/*

 Copyright (c) 2002, 2003
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

#include "OWCCacheConfig.h"
#include "OWCCacheInit.h"
#include <IOKit/IOUserClient.h>
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <pexpert/pexpert.h>

#undef super
#define super IOService

#define OWCCACHESLEEP 250

OSDefineMetaClassAndStructors(OWCCacheConfig, IOService);

bool
OWCCacheConfig::start (IOService *provider)
{
	if (!super::start (provider)) return false;
	
	// If we started up in single-user mode, then we refuse to start. The logic is that one of the
	// reasons we might be in single-user mode is because we're causing trouble, and the user wants
	// to delete us. We also refuse to start if "-c" is specified, which allows the user to turn us
	// off without booting in single-user mode.
	
	char buffer[16];
	if (PE_parse_boot_arg ("-c", buffer)) return false;
	if (PE_parse_boot_arg ("-s", buffer)) return false;
	
	initializeCacheSettings (provider);
	
	registerService ();
	
	return true;
}

IOReturn 
OWCCacheConfig::setProperties (OSObject *properties)
{
    IOReturn err;
    OSDictionary *dict;
	unsigned retVal;
	
    err = IOUserClient::clientHasPrivilege (current_task(), kIOClientPrivilegeAdministrator);
    if (err == kIOReturnSuccess) {

		dict = OSDynamicCast (OSDictionary, properties);
		if (!dict) return (kIOReturnBadArgument);
		
		OSData *data = OSDynamicCast (OSData, dict->getObject("L2CR"));
		if (data) {
			unsigned *newValue = (unsigned *) (data->getBytesNoCopy ());
			IOLog ("Setting L2CR to 0x%x\n", *newValue);
			OWCL2CacheInit (*newValue);
		}
		
		data = OSDynamicCast (OSData, dict->getObject("L3CR"));
		if (data) {
			unsigned *newValue = (unsigned *) (data->getBytesNoCopy ());
			IOLog ("Setting L3CR to 0x%x\n", *newValue);
			retVal = OWCL3CacheInit (*newValue);	
		}
	}
	
	unsigned value;
	mfspr (value, l2cr);
	setProperty ("L2CR", &value, 4);
	
	OSObject *l3Property = getProperty ("L3CR");
	if (l3Property) {
		mfspr (value, 1018);
		setProperty ("L3CR", &value, 4);
	}

    return (err);
}

void
OWCCacheConfig::initializeCacheSettings (IOService *provider)
{	
	OSData *data; 
	unsigned pvr = mfpvr ();
	
	pvr &= 0xFFFF0000;
	pvr >>= 16;
	
	data = OSDynamicCast (OSData, provider->getProperty ("clock-frequency"));
	if (!data) return;
	unsigned clock = * ((unsigned *) data->getBytesNoCopy ());
	
	switch (pvr) {
		case PROCESSOR_VERSION_750:
			initializeL2 (provider, clock, false);
			break;
			
		case PROCESSOR_VERSION_750FX:
			initializeL2FX (provider);
			break;
			
		case PROCESSOR_VERSION_7400:
		case PROCESSOR_VERSION_7410:
			initializeL2 (provider, clock, true);
			break;
			
		case PROCESSOR_VERSION_7450:
		case PROCESSOR_VERSION_7455:
			initializeL3 (provider);
			break;
	
		default:
			return;
	}
}

void
OWCCacheConfig::initializeL2FX (IOService *provider)
{
	unsigned result;
	mfspr (result, l2cr);
	
	if (!(result & l2em)) OWCL2FXCacheInit ();
	
	unsigned cacheSize = 512 * 1024;
	
	IORegistryEntry *l2cache = provider->childFromPath ("l2-cache", gIODTPlane);
	if (!l2cache) {
		l2cache = new IOService;
		l2cache->init ();
		l2cache->attachToParent (provider, gIODTPlane);
		l2cache->release ();
		l2cache->setName ("l2-cache");
		l2cache->setProperty ("name", (void *) "l2-cache", sizeof ("l2-cache"));
	}
	l2cache->setProperty ("cache-unified", (void *) "", 0);
	l2cache->setProperty ("d-cache-size", &cacheSize, 4);
	l2cache->setProperty ("i-cache-size", &cacheSize, 4);
	l2cache->setProperty ("device_type", (void *) "cache", sizeof ("cache"));

	mfspr (result, l2cr);
	setProperty ("L2CR", &result, 4);
}

void
OWCCacheConfig::initializeL2 (IOService *provider, unsigned clock, bool isG4)
{	
	unsigned clockSettings [] = {L2CLK_1, L2CLK_15, L2CLK_2, L2CLK_25, L2CLK_3, L2CLK_35, L2CLK_4};
	unsigned ramSettings [] = {L2RAM_PB, L2RAM_FT, L2RAM_LW};
	unsigned result;
	
	unsigned originalCacheSetting;
	mfspr (originalCacheSetting, l2cr);
	
	IOLog ("Setting cache to 0\n");
	IOSleep (OWCCACHESLEEP);
	result = OWCL2CacheInit (0);			// turn off the cache
	IOLog ("OWCCacheConfig: result 0x%X\n", result);
		
// 	For some reason, the first test always fails. So we run a test out here.
//	IOSleep (OWCCACHESLEEP);
//	XPFL2CacheInit (l2em | l2tsm | L2CLK_2 | L2RAM_PB | L2SIZ_1024);
	
	int ramOffset = 0;
	int clockOffset = 0;
	unsigned ramTestedGood;
	int clockTestedGood;

	for (ramOffset = 0; ramOffset < 3; ramOffset++) {
		ramTestedGood = 0;
		clockTestedGood = -1;
		for (clockOffset = 4; clockOffset >= 0; clockOffset--) {
			unsigned testValue = l2em | l2tsm | clockSettings[clockOffset] | ramSettings[ramOffset];
			testValue |= isG4 ? L2SIZ_2048 : L2SIZ_1024;
			if (ramSettings[ramOffset] == L2RAM_LW) testValue |= l2dfm | (isG4 ? L2OH_2 : L2OH_1);
			if (clock / (clockOffset + 2) * 2  <= 100000) testValue |= l2slm;
			
			// It looks like we need the IOSleep call if we are going to run multiple tests
			// in quick succession (otherwise, we freeze). My theory is we spend too much
			// time with interrupts off if we don't sleep.
			
			IOLog ("OWCCacheConfig: testing 0x%X\n", testValue);
			IOSleep (OWCCACHESLEEP);	
			result = OWCL2CacheInit (testValue);
			IOLog ("OWCCacheConfig: result 0x%X\n", result);
			
			switch (result) {
				case 256 * 1024:
				case 512 * 1024:
				case 1024 * 1024:
				case 2048 * 1024:
					ramTestedGood = result;
					clockTestedGood = clockOffset;
					break;
			
				default:
					clockOffset = 0;	// skip to the end
					break;
			}
			
			if ((ramSettings[ramOffset] == L2RAM_FT) && (clockOffset == 2)) break;
		}

		// Now, see if we tested good.
		if (ramTestedGood) break;
	}	
	
	unsigned baseValue, setValue;
	
	if (ramTestedGood) {
		baseValue = l2em | clockSettings[clockTestedGood] | ramSettings[ramOffset];

		switch (ramTestedGood) {
			case 256 * 1024:
				baseValue |= L2SIZ_256;
				break;
				
			case 512 * 1024:
				baseValue |= L2SIZ_512;
				break;
				
			case 1024 * 1024:
				baseValue |= L2SIZ_1024;
				break;
				
			case 2048 * 1024:
				baseValue |= L2SIZ_2048;
				break;
		}

		if (ramSettings[ramOffset] == L2RAM_LW) baseValue |= l2dfm | (isG4 ? L2OH_2 : L2OH_1);
		
		// Now, move up one clock setting (for safety)
		setValue = baseValue & ~l2clkm;
		setValue |= clockSettings[clockTestedGood + 1];
		if ((clock / (clockTestedGood + 3) * 2) <= 100000) setValue |= l2slm;
		
		IOSleep (OWCCACHESLEEP);
		if (originalCacheSetting & l2em) {
			setProperty ("Original L2CR", (void *) &originalCacheSetting, sizeof (originalCacheSetting));
			IOLog ("Setting L2CR to 0x%x\n", originalCacheSetting);
			OWCL2CacheInit (originalCacheSetting);
		} else {
			IOLog ("Setting L2CR to 0x%x\n", setValue);
			OWCL2CacheInit (setValue);
		}
		
		IORegistryEntry *l2cache = provider->childFromPath ("l2-cache", gIODTPlane);
		if (!l2cache) {
			l2cache = new IOService;
			l2cache->init ();
			l2cache->attachToParent (provider, gIODTPlane);
			l2cache->release ();
			l2cache->setName ("l2-cache");
			l2cache->setProperty ("name", (void *) "l2-cache", sizeof ("l2-cache"));
		}
		l2cache->setProperty ("cache-unified", (void *) "", 0);
		l2cache->setProperty ("d-cache-size", &ramTestedGood, 4);
		l2cache->setProperty ("i-cache-size", &ramTestedGood, 4);
		l2cache->setProperty ("device_type", (void *) "cache", sizeof ("cache"));
		
	} else {
		baseValue = 0;
		IOLog ("OWCCacheConfig: Unable to determine appropriate L2CR settings\n");
	}
	
	setProperty ("L2CRBase", &baseValue, 4);
	
	mfspr (setValue, l2cr);
	setProperty ("L2CR", &setValue, 4);
}

void
OWCCacheConfig::initializeL3 (IOService *provider)
{
	unsigned clockSettings [] = {L3CLK_2, L3CLK_25, L3CLK_3, L3CLK_35, L3CLK_4, L3CLK_5, L3CLK_6};
	unsigned ramSettings [] = {L3RT_PB, L3RT_DDR, L3RT_LW};
	unsigned ckspSettings [] = {0x0, 0x0, L3CKSP_2, L3CKSP_3, L3CKSP_4, L3CKSP_5};
	unsigned pspSettings [] = {L3PSP_0, L3PSP_1, L3PSP_2, L3PSP_3, L3PSP_4, L3PSP_5};
	int maxPSP [] = {1, 1, 2, 2, 3, 4, 5};
	
	unsigned originalCacheSetting;
	mfspr (originalCacheSetting, 1018);
	
	IOSleep (OWCCACHESLEEP);
	OWCL3CacheInit (0);			// turn off the cache
		
	int ramOffset, clockOffset;
	unsigned testValue, result, testcksp, testpsp;
	int cksp, psp, goodL3Value;
	bool try2MB = true;
	OSArray *l3crBase = OSArray::withCapacity (7);
	OSData *data = 0;
	
	for (ramOffset = 0; ramOffset < 3; ramOffset++) {
		// First, we see what settings, if any, will work for the slowest clock speed
		clockOffset = 6;
		cksp = 0;
		psp = 0;
		goodL3Value = 0;
		
		for (int cycle = 35; cycle >= 12; cycle--) {
			testcksp = cycle / clockOffset;
			testpsp = cycle % clockOffset;
			
			testValue = l3clkenm | l3dxm | l3dmemm | clockSettings[clockOffset] | ramSettings[ramOffset] | ckspSettings [testcksp] | pspSettings [testpsp];
			if (try2MB) testValue |= l3dmsizm;
			
			IOLog ("Testing clock %d, cycle %d, cksp %d, psp %d, value 0x%X\n", clockOffset, cycle, testcksp, testpsp, testValue);
			IOSleep (OWCCACHESLEEP);
			result = OWCL3CacheInit (testValue);
			IOLog ("Result 0x%X\n", result);
			
			bool testedGood = false;
			
			if (result == 2048 * 1024UL) {
				testedGood = true;
			} else if (result == 1024 * 1024UL) {
				if (!try2MB) testedGood = true;
			} else if (result == 512 * 1024UL) {
				if (try2MB) {
					try2MB = false;
					cycle++;
					continue;
				}
			}
						
			if (testedGood) {
				cksp = testcksp;
				psp = testpsp;
				goodL3Value = testValue;
			} else {
				break;
			}
		}
		
		if (goodL3Value != 0) {
			// record that the testValue was good
			goodL3Value &= ~(l3dmemm | l3dmsizm);
			goodL3Value |= l3em;
			if (try2MB) goodL3Value |= l3sizm;
			data = OSData::withBytes (&goodL3Value, 4);
			l3crBase->setObject (data);
			data->release ();

			// see what other clockspeeds will work
			for (clockOffset = 5; clockOffset >= 0; clockOffset--) {
				testValue = l3clkenm | l3dxm | l3dmemm | clockSettings[clockOffset] | ramSettings[ramOffset];
				if (try2MB) testValue |= l3dmsizm;
				
				// adjust cksp and psp for the clock speed
				if (psp > maxPSP[clockOffset]) {
					psp = 0;
					cksp++;
				}

				if (cksp > 5) break;
				
				testValue |= ckspSettings[cksp] | pspSettings [psp];
			
				IOLog ("Testing clock %d, cksp %d, psp %d, value 0x%X\n", clockOffset, cksp, psp, testValue);
				IOSleep (OWCCACHESLEEP);
				result = OWCL3CacheInit (testValue);
				IOLog ("Result 0x%X\n", result);
			
				if (result == (try2MB ? (2048 * 1024UL) : (1024 * 1024UL))) {
					testValue &= ~(l3dmemm | l3dmsizm);
					testValue |= l3em;
					if (try2MB) testValue |= l3sizm;
					data = OSData::withBytes (&testValue, 4);
					l3crBase->setObject (data);
					data->release ();
				} else {
					break;
				}
			}
			break; // don't need to test any more RAM types
		}
	}
	
	unsigned setValue;

	if (l3crBase->getCount () > 0) {
		setProperty ("L3CR Base", l3crBase);

		unsigned size = 256 * 1024;
		IORegistryEntry *l2cache = provider->childFromPath ("l2-cache", gIODTPlane);
		if (!l2cache) {
			l2cache = new IOService;
			l2cache->init ();
			l2cache->attachToParent (provider, gIODTPlane);
			l2cache->release ();
			l2cache->setName ("l2-cache");
			l2cache->setProperty ("name", (void *) "l2-cache", sizeof ("l2-cache"));
		}
		l2cache->setProperty ("cache-unified", (void *) "", 0);
		l2cache->setProperty ("d-cache-size", &size, 4);
		l2cache->setProperty ("i-cache-size", &size, 4);
		l2cache->setProperty ("device_type", (void *) "cache", sizeof ("cache"));
		
		size = (setValue & l3sizm) ? 2048 * 1024 : 1024 * 1024;
		
		IORegistryEntry *l3cache = l2cache->childFromPath ("l2-cache", gIODTPlane);
		if (!l3cache) {
			l3cache = new IOService;
			l3cache->init ();
			l3cache->attachToParent (l2cache, gIODTPlane);
			l3cache->release ();
			l3cache->setName ("l2-cache");
			l3cache->setProperty ("name", (void *) "l2-cache", sizeof ("l2-cache"));
		}
		l3cache->setProperty ("cache-unified", (void *) "", 0);
		l3cache->setProperty ("d-cache-size", &size, 4);
		l3cache->setProperty ("i-cache-size", &size, 4);
		l3cache->setProperty ("device_type", (void *) "cache", sizeof ("cache"));
		
		// Now, figure out which value to actually use
		
		if (l3crBase->getCount () > 1) {
			data = (OSData *) l3crBase->getObject (l3crBase->getCount () - 2);
		} else {
			data = (OSData *) l3crBase->getObject (0); 
		}
		
		setValue = * (unsigned *) data->getBytesNoCopy ();
		
		psp = (setValue & l3pspm) >> 13;
		cksp = ((setValue & l3ckspm) >> 16) + 2;
		clockOffset = (setValue & l3clkm) >> 23;
		if (clockOffset == 0) {
			clockOffset = 6;
		} else {
			clockOffset -= 2;
		}
		psp++;  // increment for safety
		if (psp > maxPSP[clockOffset]) {
			psp = 0;
			cksp++;
		}
		
		setValue &= ~(l3ckspm | l3pspm);
		setValue |= ckspSettings[cksp] | pspSettings[psp];
		IOSleep (OWCCACHESLEEP);
		
		if (originalCacheSetting & l3em) {
			setProperty ("Original L3CR", (void *) &originalCacheSetting, sizeof (originalCacheSetting));
			OWCL3CacheInit (originalCacheSetting & l3dxm);
		} else {
			IOLog ("Setting clock %d, cksp %d, psp %d, value 0x%X\n", clockOffset, cksp, psp, setValue);
			result = OWCL3CacheInit (setValue);
			IOLog ("Result 0x%X\n", result);
		}

	} else {
		IOLog ("OWCCacheConfig: Unable to determine appropriate L3CR settings\n");
	}
	
	l3crBase->release ();
			
	mfspr (setValue, 1018);
	setProperty ("L3CR", &setValue, 4);
}
