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
#include "XPFLog.h"
    
#define kReadNVRAMProcInfo		0x0398
#define kWriteNVRAMProcInfo		0x0788
#define kReadNVRAMSelector		0x022E
#define kWriteNVRAMSelector		0x032F
#define kMagicTrapNumber 		0x02F3

ToolboxNVRAM::ToolboxNVRAM ()
{
	readFromNVRAM ();
}  

UInt8 
ToolboxNVRAM::readByte (unsigned offset)
{
	return CallUniversalProc (GetToolboxTrapAddress (kMagicTrapNumber), kReadNVRAMProcInfo, kReadNVRAMSelector, offset);
}

void
ToolboxNVRAM::writeByte (unsigned offset, UInt8 byte)
{
	CallUniversalProc (GetToolboxTrapAddress (kMagicTrapNumber), kWriteNVRAMProcInfo, kWriteNVRAMSelector, offset, byte);
}

void
ToolboxNVRAM::readFromNVRAM ()
{		
	char *bufferPtr = (char *) &fBuffer;
		
	for (int cnt = 0; cnt < kOFPartitionSize; cnt++) {
		bufferPtr[cnt] = readByte (kOFPartitionOffset + cnt);
	}
		
	NVRAMValue *current;
	char *tempString;
	NVRAMString *nvramString;
	
	for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {

		switch (current->getValueType ()) { 

			case kStringValue:
				nvramString = &fBuffer.stringValues[current->getOffset()];
				if ((nvramString->offset > kOFPartitionOffset) && (nvramString->length > 0) &&
						((nvramString->offset + nvramString->length) <= (kOFPartitionOffset + kOFPartitionSize))) {
					tempString = NewPtr (nvramString->length + 1);
					memcpy (tempString, bufferPtr + nvramString->offset - kOFPartitionOffset, nvramString->length);
					tempString[nvramString->length] = 0;
					current->setStringValue (tempString);
					DisposePtr (tempString);
				} else {
					current->setStringValue ("");
				}
				break;
				
			case kBooleanValue:
				current->setBooleanValue ((fBuffer.flags & (1 << (31 - current->getOffset ()))) != 0);
				break;
				
			case kNumericValue:
				current->setNumericValue (fBuffer.numericValues[current->getOffset ()]);
				break;
		}
	}
	
	fHasChanged = false;
}
				
int
ToolboxNVRAM::writeToNVRAM ()
{
//	if (!fHasChanged) return noErr;
	fHasChanged = false;
	
	char *bufferPtr = (char *) &fBuffer;

	Erase_AC (&fBuffer);
	fBuffer.magic = 0x1275;
	fBuffer.version = 0x05;
	fBuffer.pages = 0x08;
	fBuffer.here = (unsigned) &fBuffer.strings[0] - (unsigned) &fBuffer + kOFPartitionOffset;
	fBuffer.next = 0;
	fBuffer.top = kOFPartitionOffset + kOFPartitionSize;
		
	NVRAMValue *current;
	NVRAMString *nvramString;
	unsigned length;
	
	for (TemplateAutoList_AC <NVRAMValue>::Iterator iter (&fNVRAMValues); (current = iter.Current ()); iter.Next ()) {

		switch (current->getValueType ()) { 

			case kStringValue:
				nvramString = &fBuffer.stringValues[current->getOffset()];
				length = strlen (current->getStringValue ());
				fBuffer.top -= length;
				if (fBuffer.top >= fBuffer.here) {
					nvramString->offset = fBuffer.top;
					nvramString->length = length;
					if (length > 0) BlockMoveData (current->getStringValue (), bufferPtr - kOFPartitionOffset + fBuffer.top , length);
				}
				break;
				
			case kBooleanValue:
				if (current->getBooleanValue ()) fBuffer.flags |= 1 << (31 - current->getOffset ());
				break;
				
			case kNumericValue:
				fBuffer.numericValues[current->getOffset()] = current->getNumericValue ();
				break;
		}
	}
				
	if (fBuffer.top < fBuffer.here) {
		#if qLogging
			gLogFile << "NVRAM size limits exceeded" << endl_AC;
		#endif
		return -1;
	}
	
 	fBuffer.checksum = 0;
	unsigned checksum = 0;
 	UInt16 *tmpBuffer = (UInt16 *) &fBuffer;
 	for (int cnt = 0; cnt < kOFPartitionSize / 2; cnt++) checksum += tmpBuffer[cnt];
	checksum %= 0x0000FFFF;
	fBuffer.checksum = ~checksum;
	
	try {
		for (int cnt = 0; cnt < kOFPartitionSize; cnt++) {
			writeByte (kOFPartitionOffset + cnt, bufferPtr[cnt]);
		}
	}
	
	catch (...) {
		return paramErr;
	}

	return noErr;
}

