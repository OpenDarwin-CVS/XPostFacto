/*

 Copyright (c) 2001, 2002
 Other World Computing
 All rights reserved

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

#ifndef __HFSPLUSARCHIVE_H__
#define __HFSPLUSARCHIVE_H__

#include "ZStream.h"

#include <Files.h>

class HFSPlusArchive {

	enum {
		errWrongVersion = 1,
		errWrongSignature
	};

public:

	HFSPlusArchive (FILE_STREAM_TYPE *stream, pascal Boolean
				 (*copyFilter) (const FSRef *src) = NULL);

	~HFSPlusArchive ();

	OSErr addToArchive (const FSRef *ref);
	OSErr addToArchive (const FSSpec *spec);
	OSErr addContentsToArchive (const FSRef *directory);
	OSErr addContentsToArchive (const FSSpec *directory);

	OSErr flushWrites ();

	OSErr extractArchiveTo (const FSRef *ref, bool contiguousAlloc = false);
	OSErr extractArchiveTo (const FSSpec *spec, bool contiguousAlloc = false);

private:
		
	void extractItemsTo (const FSRef *ref);
	void extractFileTo (const FSRef *ref);
	void extractDirectoryTo (const FSRef *ref);

	void archiveCurrentItem ();
	void archiveCurrentItemAsFile ();
	void archiveCurrentItemAsDirectory ();

	ZSTREAM_TYPE fStream;	// the stream we read from / write to
	Ptr fCopyBuffer;
	bool fHeaderWritten;
	bool fSuspendExtraction;
	pascal Boolean (*fCopyFilter) (const FSRef *src);

// The rest are temporary variables to minimize stack space in recursion.
// This is probably premature optimization, but oh well.

	FSRef fCurrentItem;
	FSCatalogInfo fCatalogInfo;
	HFSUniStr255 fItemName;
	OSErr fErr;
	ItemCount fActualObjects;
	bool fContiguousAlloc;
};

#endif