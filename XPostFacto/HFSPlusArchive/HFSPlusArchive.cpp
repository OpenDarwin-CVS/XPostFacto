/*

    Copyright (c) 2001, 2002
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

#include "HFSPlusArchive.h"

#ifdef __GNUC__
void ThrowIfOSErr_AC(OSStatus inErr) {if (inErr) throw (inErr);}
#else
#include "CStream_AC.h"
#endif

#include "MoreFilesExtras.h"
#include <Threads.h>

#ifdef __MACH__
	#include <sys/types.h>
	#include <unistd.h>
#endif

const UInt32 kHFSPlusArchiveSignature = 'hfsA';
const UInt32 kHFSPlusArchiveVersion = 1;

#define kCopyBufferSize (16 * 1024)

HFSPlusArchive::HFSPlusArchive (FILE_STREAM_TYPE *stream, pascal Boolean 
						(*copyFilter) (void *refCon, const FSRef *src), void *refCon)
	: fStream (stream), fRefCon (refCon)
{
	fCopyFilter = copyFilter;
	fErr = noErr;
	fCopyBuffer = NewPtr (kCopyBufferSize);
	fHeaderWritten = false;
	fSuspendExtraction = false;
}

HFSPlusArchive::~HFSPlusArchive ()
{
	if (fCopyBuffer) DisposePtr (fCopyBuffer);
}

OSErr
HFSPlusArchive::flushWrites ()
{
	try {
		// I'm going to write a last 0 to indicate the end of the file.
		SInt64 size = 0;
		fStream.WriteBytes (&size, sizeof (size));
		fStream.Flush ();
	}
	catch (int err) {
		return err;
	}
	return noErr;
}

void
HFSPlusArchive::archiveCurrentItemAsDirectory ()
{
	FSIterator iterator;
	ThrowIfOSErr_AC (FSOpenIterator (&fCurrentItem, kFSIterateFlat, &iterator));
	try {
		while (true) {
			fErr = FSGetCatalogInfoBulk (iterator, 1, &fActualObjects, NULL, 
							kFSCatInfoNodeFlags | kFSCatInfoSettableInfo, &fCatalogInfo, 
							&fCurrentItem, NULL, &fItemName);
			if (fErr == errFSNoMoreItems) {
				fErr = noErr;
				break;
			} else if (fErr != noErr) {
				throw fErr;
			} else if (fActualObjects == 1) {
				archiveCurrentItem ();
			}
		}
	}
	catch (...) {
		FSCloseIterator (iterator);
		throw;
	}

	FSCloseIterator (iterator);		

	// We've reached the end of the directory. The next thing in regular entry would be
	// the length of the name. So we write a 0 to indicate that we're finished the
	// directory.

	fItemName.length = 0;
	fStream.WriteBytes (&fItemName.length, sizeof (fItemName.length));
}

void
HFSPlusArchive::archiveCurrentItemAsFile ()
{
	// Iterate over the forks. Write a header with fork name and length. Then write data.
	// I'm using local variables here as this function doesn't recurse
	// Of course, this whole "no local variables thing" is probably premature optimization
	CatPositionRec forkIterator;		
	forkIterator.initialize = 0;
	HFSUniStr255 forkName;
	SInt64 forkSize;
	
	while (true) {
		fErr = FSIterateForks (&fCurrentItem, &forkIterator, &forkName, &forkSize, NULL);
		if (fErr == noErr) {
			if (forkSize > 0) {
				fStream.WriteBytes (&forkSize, sizeof (forkSize));
				fStream.WriteBytes (&forkName.length, sizeof (forkName.length));
				fStream.WriteBytes (&forkName.unicode, forkName.length * sizeof (UniChar));
				SInt16 forkRefNum;
				ThrowIfOSErr_AC (FSOpenFork (&fCurrentItem, forkName.length, forkName.unicode, 
					fsRdWrPerm, &forkRefNum));
				try {
					ThrowIfOSErr_AC (FSSetForkPosition (forkRefNum, fsFromStart, 0));
					while (forkSize > 0) {
						ThrowIfOSErr_AC (FSReadFork (forkRefNum, fsAtMark, 0, 
								forkSize < kCopyBufferSize ? forkSize : kCopyBufferSize,
								fCopyBuffer, NULL));
						fStream.WriteBytes (fCopyBuffer, forkSize < kCopyBufferSize ? forkSize : kCopyBufferSize);
						forkSize -= kCopyBufferSize;
					}
				}
				catch (...)
				{
					FSCloseFork (forkRefNum);
					throw;
				}
				FSCloseFork (forkRefNum);
			}
		} else {
			fErr = noErr;	// clear the error
			break;
		}
	}
	// To indicate the end of the forks, we write a 0 fork length
	forkSize = 0;
	fStream.WriteBytes (&forkSize, sizeof (forkSize));
}

void 
HFSPlusArchive::archiveCurrentItem ()
{
	fStream.WriteBytes (&fItemName.length, sizeof (fItemName.length));
	fStream.WriteBytes (&fItemName.unicode, fItemName.length * sizeof (UniChar));
	fStream.WriteBytes (&fCatalogInfo, sizeof (fCatalogInfo));
	
	if (fCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) {
		archiveCurrentItemAsDirectory ();
	} else {
		archiveCurrentItemAsFile ();
	}
}
	
OSErr
HFSPlusArchive::addContentsToArchive (const FSRef *directory)
{
	try {
		ThrowIfOSErr_AC (FSGetCatalogInfo (directory, kFSCatInfoNodeFlags, &fCatalogInfo, NULL, NULL, NULL));
		if (!(fCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask)) return errFSNotAFolder;

		FSIterator iterator;
		ThrowIfOSErr_AC (FSOpenIterator (directory, kFSIterateFlat, &iterator));
		try {
			while (true) {
				fErr = FSGetCatalogInfoBulk (iterator, 1, &fActualObjects, NULL, 
								kFSCatInfoNodeFlags | kFSCatInfoSettableInfo, &fCatalogInfo, 
								&fCurrentItem, NULL, &fItemName);
				if (fErr == errFSNoMoreItems) {
					fErr = noErr;
					break;	
				} else if (fErr != noErr) {
					throw fErr;
				} else if (fActualObjects == 1) {
					if (!fHeaderWritten) {
						fStream.WriteBytes (&kHFSPlusArchiveSignature, sizeof (kHFSPlusArchiveSignature));
						fStream.WriteBytes (&kHFSPlusArchiveVersion, sizeof (kHFSPlusArchiveVersion));
						fHeaderWritten = true;
					}
					archiveCurrentItem ();
				}   
			} 
		}	
		catch (...) {
			FSCloseIterator (iterator);		
			throw;
		}
	
		FSCloseIterator (iterator);		
		return fErr;
	}
	catch (OSErr err) {
		return err;
	}
}	
		
OSErr
HFSPlusArchive::addContentsToArchive (const FSSpec *directory)
{
	FSRef ref;
	OSErr err = FSpMakeFSRef (directory, &ref);
	if (err != noErr) return err;
	return addContentsToArchive (&ref);
}

OSErr 
HFSPlusArchive::addToArchive (const FSSpec *spec)
{
	FSRef ref;
	OSErr err = FSpMakeFSRef (spec, &ref);
	if (err != noErr) return err;
	return addToArchive (&ref);
}

OSErr 
HFSPlusArchive::addToArchive (const FSRef *ref)
{
	if (!fCopyBuffer) return memFullErr;
	BlockMoveData (ref, &fCurrentItem, sizeof (fCurrentItem));
	fErr = FSGetCatalogInfo (&fCurrentItem, kFSCatInfoNodeFlags | kFSCatInfoSettableInfo, &fCatalogInfo,
								&fItemName, NULL, NULL);
	if (fErr == noErr) {
		try {
			if (!fHeaderWritten) {
				fStream.WriteBytes (&kHFSPlusArchiveSignature, sizeof (kHFSPlusArchiveSignature));
				fStream.WriteBytes (&kHFSPlusArchiveVersion, sizeof (kHFSPlusArchiveVersion));
				fHeaderWritten = true;
			}
			archiveCurrentItem ();
		}
		catch (OSErr err) {
			return err;
		}
	}
	return fErr;
}

void
HFSPlusArchive::extractDirectoryTo (const FSRef *ref)
{
//	::YieldToAnyThread ();
	if (!fSuspendExtraction) {
		FSRef newRef;
		fErr = FSCreateDirectoryUnicode (ref, fItemName.length, fItemName.unicode,
					kFSCatInfoNone, NULL, &newRef, NULL, NULL);
		if (fErr == dupFNErr) {
			// The folder already exists. The behaviour that I want here is to 
			// suppress output for this folder, but to continue on after that.
/*			fSuspendExtraction = true;
			fErr = noErr;
			extractItemsTo (&newRef);
			fSuspendExtraction = false;
*/
			// Now the behaviour I want is to overwrite the directory. So we need to
			// delete it, and then try again.
			fErr = noErr;
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (ref, fItemName.length, fItemName.unicode, 
													kTextEncodingUnknown, &newRef));
			FSSpec theSpec;
			ThrowIfOSErr_AC (FSGetCatalogInfo (&newRef, kFSCatInfoNone, NULL, NULL, &theSpec, NULL));

			ThrowIfOSErr_AC (DeleteDirectory (theSpec.vRefNum, theSpec.parID, theSpec.name)); 
			ThrowIfOSErr_AC (FSCreateDirectoryUnicode (ref, fItemName.length, fItemName.unicode,
					kFSCatInfoNone, NULL, &newRef, NULL, NULL));
		}
						
		if (fErr == noErr) {
			// extract to the directory and then set catalog info
			if (fCopyFilter) (*fCopyFilter) (fRefCon, &newRef);
			FSCatalogInfo newCatalogInfo;
			BlockMoveData (&fCatalogInfo, &newCatalogInfo, sizeof (fCatalogInfo));
			extractItemsTo (&newRef);
			ThrowIfOSErr_AC (FSSetCatalogInfo (&newRef, kFSCatInfoSettableInfo, &newCatalogInfo));
			// In Mac OS X, we need to set the user and group, because it doesn't get done by FSSetCatalogInfo
			#ifdef __MACH__
				ThrowIfOSErr_AC (FSRefMakePath (&newRef, (UInt8 *) fItemPath, 255));
				chown (fItemPath, newCatalogInfo.permissions[0], newCatalogInfo.permissions[1]);
			#endif
		} else {
			throw fErr;   // bail out due to error
		}
	} else {
		// extraction suspended. So we just keep fake-extracting for a while.
		extractItemsTo (ref);	
	}
}

void
HFSPlusArchive::extractFileTo (const FSRef *ref)
{
	FSRef newRef;
	SInt16 forkRefNum;
	bool fileExists = false;

	if (!fSuspendExtraction) {
		fErr = FSCreateFileUnicode (ref, fItemName.length, fItemName.unicode, kFSCatInfoNone, NULL,
				&newRef, NULL);
		if (fErr == dupFNErr) {
			fileExists = true;
			// the "fileExists" semantics was to simply skip the file. But now the behaviour I want
			// is to overwrite it. So I just get the FSRef.
			ThrowIfOSErr_AC (FSMakeFSRefUnicode (ref, fItemName.length, fItemName.unicode, 
													kTextEncodingUnknown, &newRef));
			fErr = noErr;
		} else if (fErr != noErr) {
			throw fErr;
		}
	}
	
	if (fCopyFilter) (*fCopyFilter) (fRefCon, &newRef);
	
	while (true) {
//		::YieldToAnyThread ();
		HFSUniStr255 forkName;
		SInt64 forkSize;
		fStream.ReadBytes (&forkSize, sizeof (forkSize));
		if (forkSize == 0) break; // we're done!
		fStream.ReadBytes (&forkName.length, sizeof (forkName.length));
		fStream.ReadBytes (&forkName.unicode, forkName.length * sizeof (UniChar));
		
		forkRefNum = 0;
		try {
			if (!fSuspendExtraction) {
				if (!fileExists) ThrowIfOSErr_AC (FSCreateFork (&newRef, forkName.length, forkName.unicode));	
				ThrowIfOSErr_AC (FSOpenFork (&newRef, forkName.length, forkName.unicode, 
							fsRdWrPerm, &forkRefNum));
				if (fContiguousAlloc) {
					ThrowIfOSErr_AC (FSAllocateFork (forkRefNum, 
							kFSAllocAllOrNothingMask | kFSAllocContiguousMask | kFSAllocNoRoundUpMask,
							fsFromStart, 0, forkSize, NULL));
				}			
				ThrowIfOSErr_AC (FSSetForkSize (forkRefNum, fsFromStart, forkSize));
				ThrowIfOSErr_AC (FSSetForkPosition (forkRefNum, fsFromStart, 0));
			}
			
			while (forkSize > 0) {
//				::YieldToAnyThread ();
				fStream.ReadBytes (fCopyBuffer, forkSize < kCopyBufferSize ? forkSize : kCopyBufferSize);
				if (forkRefNum) ThrowIfOSErr_AC (FSWriteFork (forkRefNum, fsAtMark, 0, 
									forkSize < kCopyBufferSize ? forkSize : kCopyBufferSize,
									fCopyBuffer, NULL));
				forkSize -= kCopyBufferSize;
			}
		}
		catch (...) {
			if (forkRefNum) FSCloseFork (forkRefNum);
			throw;
		}			
		
		if (forkRefNum) ThrowIfOSErr_AC (FSCloseFork (forkRefNum));
	}
	
	if (!fSuspendExtraction) {
		ThrowIfOSErr_AC (FSSetCatalogInfo (&newRef, kFSCatInfoSettableInfo, &fCatalogInfo));
		// In Mac OS X, we need to set the user and group, because it doesn't get done by FSSetCatalogInfo
		#ifdef __MACH__
			ThrowIfOSErr_AC (FSRefMakePath (&newRef, (UInt8 *) fItemPath, 255));
			chown (fItemPath, fCatalogInfo.permissions[0], fCatalogInfo.permissions[1]);
		#endif
	}
}	

void
HFSPlusArchive::extractItemsTo (const FSRef *ref)
{
	while (true) {
//		::YieldToAnyThread ();
		try {
			fStream.ReadBytes (&fItemName.length, sizeof (fItemName.length));
			if (fItemName.length == 0) break; // we're done!
		}
		catch (...) {
			fErr = noErr;
			break;	// we're at the very end of the stream (presumably)
		}
	
		fStream.ReadBytes (&fItemName.unicode, fItemName.length * sizeof (UniChar));
		fStream.ReadBytes (&fCatalogInfo, sizeof (fCatalogInfo));

		if (fCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask) {
			extractDirectoryTo (ref);			
		} else {
			extractFileTo (ref);			
		}
	}	
}

OSErr 
HFSPlusArchive::extractArchiveTo (const FSSpec *spec, bool contiguousAlloc)
{
	FSRef ref;
	OSErr err = FSpMakeFSRef (spec, &ref);
	if (err != noErr) return err;
	return extractArchiveTo (&ref, contiguousAlloc);
}

OSErr 
HFSPlusArchive::extractArchiveTo (const FSRef *ref, bool contiguousAlloc)
{
	fContiguousAlloc = contiguousAlloc;
	if (!fCopyBuffer) return memFullErr;

	fErr = FSGetCatalogInfo (ref, kFSCatInfoNodeFlags, &fCatalogInfo, NULL, NULL, NULL);
	if (fErr != noErr) return fErr;
	if (!(fCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask)) return errFSNotAFolder;

	UInt32 signature;
	fStream.ReadBytes (&signature, sizeof (signature));
	if (signature != kHFSPlusArchiveSignature) return errWrongSignature;
	
	UInt32 version;
	fStream.ReadBytes (&version, sizeof (version));
	if (version > kHFSPlusArchiveVersion) return errWrongVersion;

	// Now, start the ball rolling
	try {
		extractItemsTo (ref);
	}
	catch (OSErr err) {
		fErr = err;
	}
	catch (int error) {
		fErr = error;
	}
	catch (...) {
		fErr = -27;
	}
		
	return fErr;
}