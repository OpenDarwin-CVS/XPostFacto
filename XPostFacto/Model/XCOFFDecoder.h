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

#ifndef XCOFFDECODER_H
#define XCOFFDECODER_H

#include "Files.h"

struct XFileHeader {
	/* File header */
	UInt16 magic;
#define kFileMagic			0x1DF
	UInt16 nSections;
	UInt32 timeAndDate;
	UInt32 symPtr;
	UInt32 nSyms;
	UInt16 optHeaderSize;
	UInt16 flags;
};

struct XOptHeader {
	/* Optional header */
	UInt16 magic;
#define kOptHeaderMagic		0x10B
	UInt16 version;
	UInt32 textSize;
	UInt32 dataSize;
	UInt32 BSSSize;
	UInt32 entryPoint;
	UInt32 textStart;
	UInt32 dataStart;
	UInt32 toc;
	UInt16 snEntry;
	UInt16 snText;
	UInt16 snData;
	UInt16 snTOC;
	UInt16 snLoader;
	UInt16 snBSS;
	UInt8 filler[28];
};

struct XSection {
	char name[8];
	UInt32 pAddr;
	UInt32 vAddr;
	UInt32 size;
	UInt32 sectionFileOffset;
	UInt32 relocationsFileOffset;
	UInt32 lineNumbersFileOffset;
	UInt16 nRelocations;
	UInt16 nLineNumbers;
	UInt32 flags;
};

struct XCOFFheader {
	XFileHeader file;
	XOptHeader opt;
	XSection text;
	XSection data;
	XSection BSS;
};

class XCOFFDecoder {

	public:

		XCOFFDecoder (CRandomAccessStream_AC *stream);
		unsigned int getEntryPoint () {return fEntryPoint;}
		unsigned int getSize () {return fSize;}
		unsigned int getLoadAddress () {return fHead.text.pAddr;}
		void unpackToFork (SInt16 forkRefNum);
		
	private:

		CRandomAccessStream_AC *fStream;
		XCOFFheader fHead;
		unsigned fSize;
		unsigned fEntryPoint;

};

#endif