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

#include "XCOFFDecoder.h"
#include <iostream.h>
#include <Threads.h>

XCOFFDecoder::XCOFFDecoder (CRandomAccessStream_AC *stream)
{
	ThrowIfNULL_AC (stream);
	fStream = stream;
	fStream->SetPosition (0);
	fStream->ReadBytes (&fHead, sizeof (fHead));
	
	fSize = fHead.text.size + fHead.data.size + fHead.BSS.size;

	unsigned entryPointOffset = fHead.opt.entryPoint - fHead.text.pAddr;
	if (entryPointOffset > fHead.text.size) {
		entryPointOffset -= fHead.text.size;
		if (entryPointOffset > fHead.data.size) {
			fEntryPoint = 0;
		} else {
			fStream->SetPosition (fHead.data.sectionFileOffset + entryPointOffset);
			fStream->ReadBytes (&fEntryPoint, sizeof (fEntryPoint));			
		}
	} else {
		fStream->SetPosition (fHead.text.sectionFileOffset + entryPointOffset);
		fStream->ReadBytes (&fEntryPoint, sizeof (fEntryPoint));
	}
}

void
XCOFFDecoder::unpackToFork (SInt16 forkRefNum)
{
	const unsigned bufferSize = 4096;
	char *buffer = (char *) malloc (bufferSize);
	ThrowIfNULL_AC (buffer);
	try {
		unsigned bytesLeft;

		fStream->SetPosition (fHead.text.sectionFileOffset);
		bytesLeft = fHead.text.size;
		while (bytesLeft > 0) {
			unsigned bytesToRead = bytesLeft < bufferSize ? bytesLeft : bufferSize;
			fStream->ReadBytes (buffer,  bytesToRead);
			ThrowIfOSErr_AC (FSWriteFork (forkRefNum, fsAtMark, 0, bytesToRead, buffer, NULL));
			bytesLeft -= bytesToRead;
		}
		
		fStream->SetPosition (fHead.data.sectionFileOffset);
		bytesLeft = fHead.data.size;
		while (bytesLeft > 0) {
			unsigned bytesToRead = bytesLeft < bufferSize ? bytesLeft : bufferSize;
			fStream->ReadBytes (buffer,  bytesToRead);
			ThrowIfOSErr_AC (FSWriteFork (forkRefNum, fsAtMark, 0, bytesToRead, buffer, NULL));
			bytesLeft -= bytesToRead;
		}
				
		BlockSet_AC (buffer, bufferSize, 0);
		bytesLeft = fHead.BSS.size;
		while (bytesLeft > 0) {
			unsigned bytesToWrite = bytesLeft < bufferSize ? bytesLeft : bufferSize;
			ThrowIfOSErr_AC (FSWriteFork (forkRefNum, fsAtMark, 0, bytesToWrite, buffer, NULL));
			bytesLeft -= bytesToWrite;
		}
	}
	catch (...) {
		free (buffer);
		throw;
	}
	free (buffer);
}