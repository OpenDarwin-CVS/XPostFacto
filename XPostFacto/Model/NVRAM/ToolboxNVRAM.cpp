/*

Copyright (c) 2002
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

// ===================
// Toolbox NVRAM
// ===================

#include "ToolboxNVRAM.h"
#include <Debugging.h>

/*

EXTERN_API_C( long )
CallUniversalProc(
  UniversalProcPtr   theProcPtr,
  ProcInfoType       procInfo,
  ...);

*/
    
ToolboxNVRAM::ToolboxNVRAM ()
{
	OSErr err;
	CFragConnectionID connID = kInvalidID;

	err = GetSharedLibrary( "\pInterfaceLib", kCompiledCFragArch, kReferenceCFrag, &connID, NULL, NULL );

	if ( err == noErr ) err = FindSymbol (connID, "\pCallUniversalProc", (Ptr *) &callUniversalProcPtr, NULL);
	if ( err == noErr ) err = FindSymbol (connID, "\pGetToolboxTrapAddress", (Ptr *) &callGetToolboxTrapAddressPtr, NULL);

	if ( err != noErr ) {
		callUniversalProcPtr = NULL;
		callGetToolboxTrapAddressPtr = NULL;
		throw err;
	}
}  

#define kReadNVRAMProcInfo		0x0398
#define kWriteNVRAMProcInfo		0x0788
#define kReadNVRAMSelector		0x022E
#define kWriteNVRAMSelector		0x032F
#define kMagicTrapNumber 		0x02F3

UniversalProcPtr
ToolboxNVRAM::getProcPtr ()
{
	UniversalProcPtr ptr = (*callGetToolboxTrapAddressPtr) (kMagicTrapNumber);
	return ptr;
}

UInt8 
ToolboxNVRAM::readByte (unsigned offset)
{
	if (callUniversalProcPtr == NULL) throw paramErr;
	return (*callUniversalProcPtr) (getProcPtr (), kReadNVRAMProcInfo, kReadNVRAMSelector, offset);
}

void
ToolboxNVRAM::writeByte (unsigned offset, UInt8 byte)
{
	if (callUniversalProcPtr == NULL) throw paramErr;
	(*callUniversalProcPtr) (getProcPtr (), kWriteNVRAMProcInfo, kWriteNVRAMSelector, offset, byte);
}


