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
    In no event shall Ryan Rempel be liable for any direct, indirect,
    incidental, special, exemplary, or consequential damages (including, but
    not limited to, procurement of substitute goods or services; loss of use,
    data, or profits; or business interruption) however caused and on any
    theory of liability, whether in contract, strict liability, or tort
    (including negligence or otherwise) arising in any way out of the use of
    this software, even if advised of the possibility of such damage.
    
*/  

/*

	FSRefCopying.c
	==============
		
	The idea is to copy files & directories, a la MoreFiles, but using FSRefs
	to preserve long filenames.
	
	This code is descended from Apple's MoreFiles, but with extensive changes.
	
*/

#include <MacTypes.h>
#include <MacErrors.h>
#include <MacMemory.h>
#include <Files.h>
#include <Script.h>
#include <Math64.h>

#include "FSRefCopying.h"
#include "MoreFiles.h"
#include "MoreFilesExtras.h"
#include "MoreDesktopMgr.h"
#include "FSRefCopying.h"

#include <Threads.h>

/*****************************************************************************/

/* local constants */

/*	The deny-mode privileges to use when opening the source and destination files. */

enum
{
	srcCopyMode = dmRdDenyWr,
	dstCopyMode = dmWrDenyRdWr
};

/*	The largest (16K) and smallest (.5K) copy buffer to use if the caller doesn't supply 
**	their own copy buffer. */

enum
{
	bigCopyBuffSize  = 0x00004000,
	minCopyBuffSize  = 0x00000200
};

/*****************************************************************************/

/* static prototypes */

static	OSErr	GetDestinationDirInfo(const FSRef *dstRef,
									  Boolean *isDirectory,
									  Boolean *isDropBox);
/*	GetDestinationDirInfo tells us if the destination is a directory, it's
	directory ID, and if it's an AppleShare drop box (write privileges only --
	no read or search privileges).
	vRefNum		input:	Volume specification.
	dirID		input:	Directory ID.
	name		input:	Pointer to object name, or nil when dirID
						specifies a directory that's the object.
	theDirID	output:	If the object is a file, then its parent directory
						ID. If the object is a directory, then its ID.
	isDirectory	output:	True if object is a directory; false if
						object is a file.
	isDropBox	output:	True if directory is an AppleShare drop box.
*/

static	OSErr	CheckForForks(const FSRef *ref,
							  Boolean *hasDataFork,
							  Boolean *hasResourceFork);
/*	CheckForForks tells us if there is a data or resource fork to copy.
	vRefNum		input:	Volume specification of the file's current
							location.
	dirID		input:	Directory ID of the file's current location.
	name		input:	The name of the file.
*/

static	OSErr	PreflightFileCopySpace(const FSRef *srcRef,
									   const FSRef *dstDirRef,
									   Boolean *spaceOK);
/*	PreflightFileCopySpace determines if there's enough space on a
	volume to copy the specified file to that volume.
	Note: The results of this routine are not perfect. For example if the
	volume's catalog or extents overflow file grows when the new file is
	created, more allocation blocks may be needed beyond those needed for
	the file's data and resource forks.

	srcVRefNum		input:	Volume specification of the file's current
							location.
	srcDirID		input:	Directory ID of the file's current location.
	srcName			input:	The name of the file.
	dstVolName		input:	A pointer to the name of the volume where
							the file will be copied or NULL.
	dstVRefNum		input:	Volume specification indicating the volume
							where the file will be copied.
	spaceOK			output:	true if there's enough space on the volume for
							the file's data and resource forks.
*/

/*****************************************************************************/

static	OSErr	GetDestinationDirInfo(const FSRef *ref,
									  Boolean *isDirectory,
									  Boolean *isDropBox)
{
	OSErr error;
	FSCatalogInfo info;
	error = FSGetCatalogInfo (ref, kFSCatInfoNodeFlags | kFSCatInfoUserPrivs, &info, NULL, NULL, NULL);

	*isDirectory = info.nodeFlags & kFSNodeIsDirectoryMask;
	*isDropBox = userHasDropBoxAccess (info.userPrivileges);
	
	return ( error );
}

/*****************************************************************************/

static	OSErr	CheckForForks(const FSRef *ref,
							  Boolean *hasDataFork,
							  Boolean *hasResourceFork)
{
	OSErr error;
	FSCatalogInfo info;

	error = FSGetCatalogInfo (ref, kFSCatInfoDataSizes | kFSCatInfoRsrcSizes, &info, NULL, NULL, NULL);
	*hasDataFork = (info.dataLogicalSize != 0);
	*hasResourceFork = (info.rsrcLogicalSize != 0);
		
	return ( error );
}

/*****************************************************************************/

static	OSErr	PreflightFileCopySpace(const FSRef *srcRef,
									   const FSRef *dstRef,
									   Boolean *spaceOK)
{
	OSErr error;
	unsigned long dstFreeBlocks;
	unsigned long dstBlksPerAllocBlk;
	unsigned long srcDataBlks;
	unsigned long srcResourceBlks;
	FSCatalogInfo catInfo;
	FSVolumeInfo  volInfo;
	
	error = FSGetCatalogInfo (dstRef, kFSCatInfoVolume, &catInfo, NULL, NULL, NULL);
	if (error != noErr) return error;
	
	error = FSGetVolumeInfo (catInfo.volume, 0, NULL, kFSVolInfoSizes | kFSVolInfoBlocks, 
    			&volInfo, NULL, NULL);
	
	if ( error == noErr )
	{
		/* get allocation block size (always multiple of 512) and divide by 512
		  to get number of 512-byte blocks per allocation block */
		dstBlksPerAllocBlk = ((unsigned long) volInfo.blockSize >> 9);
		
		/* Convert freeBytes to free disk blocks (512-byte blocks) */
		dstFreeBlocks = U32SetU(U64ShiftRight(volInfo.freeBytes, 9));
		
		/* Now, get the size of the file's data resource forks */
		error = FSGetCatalogInfo (srcRef, kFSCatInfoDataSizes | kFSCatInfoRsrcSizes, 
						&catInfo, NULL, NULL, NULL);
		
		if ( error == noErr )
		{
			/* Since space on Mac OS disks is always allocated in allocation blocks, */
			/* this code takes into account rounding up to the end of an allocation block. */

			/* get number of 512-byte blocks needed for data fork */
			if ( ((unsigned long) catInfo.dataLogicalSize & 0x000001ff) != 0 )
			{
				srcDataBlks = ((unsigned long) catInfo.dataLogicalSize >> 9) + 1;
			}
			else
			{
				srcDataBlks = (unsigned long) catInfo.dataLogicalSize >> 9;
			}
			
			/* now, calculate number of new allocation blocks needed */
			if ( srcDataBlks % dstBlksPerAllocBlk )
			{
				srcDataBlks = (srcDataBlks / dstBlksPerAllocBlk) + 1;
			}
			else
			{
				srcDataBlks /= dstBlksPerAllocBlk;
			}
		
			/* get number of 512-byte blocks needed for resource fork */
			if ( ((unsigned long) catInfo.rsrcLogicalSize & 0x000001ff) != 0 )
			{
				srcResourceBlks = ((unsigned long) catInfo.rsrcLogicalSize >> 9) + 1;
			}
			else
			{
				srcResourceBlks = (unsigned long) catInfo.rsrcLogicalSize >> 9;
			}

			/* now, calculate number of new allocation blocks needed */
			if ( srcResourceBlks % dstBlksPerAllocBlk )
			{
				srcResourceBlks = (srcResourceBlks / dstBlksPerAllocBlk) + 1;
			}
			else
			{
				srcResourceBlks /= dstBlksPerAllocBlk;
			}
			
			/* Is there enough room on the destination volume for the source file? */
			*spaceOK = ( ((srcDataBlks + srcResourceBlks) * dstBlksPerAllocBlk) <= dstFreeBlocks );
		}
	}
	
	return ( error );
}

/**************/

OSErr
FSGetOrCreateDirectoryUnicode (
    const FSRef *parentRef, 
    UniCharCount nameLength, 
    const UniChar *name, 
    FSCatalogInfoBitmap whichInfo, 
    const FSCatalogInfo *catalogInfo, 
    FSRef *newRef, 
    FSSpec *newSpec, 
    UInt32 *newDirID
)
{
	OSErr error;
	
	// First we try to get the directory (if it exists)
	error = FSMakeFSRefUnicode (parentRef, nameLength, name, NULL, newRef);
	if (error != noErr) {
		// It doesn't exist. So we create it!
		return FSCreateDirectoryUnicode (parentRef, nameLength, name, whichInfo, catalogInfo, newRef, newSpec, newDirID);
	} else {
		// It exists. Now, make sure it is a directory . Also, get the FSSpec & dirID
		FSCatalogInfo catInfo;
		error = FSGetCatalogInfo (newRef, kFSCatInfoNodeFlags | kFSCatInfoNodeID, &catInfo, NULL, newSpec, NULL);
		if (error != noErr) return error;
		if (catInfo.nodeFlags & kFSNodeIsDirectoryMask) {
			// it exists and it is a directory. We set the catalog info for consistency sake.
			if (newDirID) *newDirID = catInfo.nodeID;
			if (catalogInfo) {
				error = FSSetCatalogInfo (newRef, whichInfo, catalogInfo);
				if (error != noErr) return error;
			}
		} else {
			return errFSNotAFolder;
		}
	}
	return noErr;
}

/**************/

OSErr
FSRefDTCopyComment (const FSRef *srcRef, const FSRef *dstRef)
{
	OSErr err;
	FSSpec srcSpec, dstSpec;
	err = FSGetCatalogInfo (srcRef, kFSCatInfoNone, NULL, NULL, &srcSpec, NULL);
	if (err == noErr) {
		FSGetCatalogInfo (dstRef, kFSCatInfoNone, NULL, NULL, &dstSpec, NULL); 
 		if (err == noErr) {
 			err = FSpDTCopyComment(&srcSpec, &dstSpec);
 		}
 	}
	return err;
}

/*************/

OSErr	FSCopyFork(short srcRefNum,
						 short dstRefNum,
						 void *copyBufferPtr,
						 long copyBufferSize)
{
	OSErr srcError;
	OSErr dstError;
	SInt64 srcForkSize;

	if ( (copyBufferPtr == NULL) || (copyBufferSize == 0) ) return ( paramErr );
	
	/* preallocate the destination fork and */
	/* ensure the destination fork's EOF is correct after the copy */
	srcError = FSGetForkSize (srcRefNum, &srcForkSize);
	if ( srcError != noErr ) return ( srcError );
	dstError = FSSetForkSize (dstRefNum, fsFromStart, srcForkSize);
	if ( dstError != noErr ) return ( dstError );

	/* reset source fork's mark */
	srcError = FSSetForkPosition (srcRefNum, fsFromStart, 0);
	if ( srcError != noErr ) return ( srcError );

	/* reset destination fork's mark */
	dstError = FSSetForkPosition (dstRefNum, fsFromStart, 0);
	if ( dstError != noErr ) return ( dstError );

	/* If copyBufferSize is greater than 512 bytes, make it a multiple of 512 bytes */
	/* This will make writes on local volumes faster */
	if ( (copyBufferSize >= 512) && ((copyBufferSize & 0x1ff) != 0) )
	{
		copyBufferSize &= 0xfffffe00;
	}

	while ( (srcError == noErr) && (dstError == noErr) )
	{
		::YieldToAnyThread ();
		ByteCount actualCount;
		srcError = FSReadFork (srcRefNum, fsAtMark + noCacheMask, 0, copyBufferSize, copyBufferPtr, &actualCount);
		dstError = FSWriteFork (dstRefNum, fsAtMark + noCacheMask, 0, actualCount, copyBufferPtr, NULL);
	}

	/* make sure there were no errors at the destination */
	if ( dstError != noErr ) return ( dstError );

	/* make sure the only error at the source was eofErr */
	if ( srcError != eofErr ) return ( srcError );

	return ( noErr );
}


/****************************/

OSErr	
FSRefCopyFileMgrAttributes(	const FSRef *srcRef,
							const FSRef *dstRef,
							Boolean copyLockBit)
{
	OSErr error;
	FSCatalogInfo info;
	Boolean objectIsDirectory;
	Boolean setLockBit;
	
	error = FSGetCatalogInfo (srcRef, kFSCatInfoSettableInfo | kFSCatInfoNodeFlags, &info, 
								NULL, NULL, NULL);
	if ( error == noErr )
	{
		objectIsDirectory = ( info.nodeFlags & kFSNodeIsDirectoryMask );

		/* don't copy the hasBeenInited bit */
		(*((FileInfo *) (info.finderInfo))).finderFlags &= ~kHasBeenInited;
		
		// see if we should set the lock bit
		setLockBit = copyLockBit && (info.nodeFlags & kFSNodeLockedMask);
		
		// then we always clear the locked bit, so we can do it in two steps
		info.nodeFlags &= ~kFSNodeLockedMask;
		
		error = FSSetCatalogInfo (dstRef, kFSCatInfoSettableInfo, &info);

		if ( (error == noErr) && setLockBit )
		{
			info.nodeFlags |= kFSNodeLockedMask;
			error = FSSetCatalogInfo (dstRef, kFSCatInfoNodeFlags, &info);
			if ( (error != noErr) && (objectIsDirectory) )
			{
				error = noErr; /* ignore lock errors if destination is directory */
			}
		}
	}
	
	return ( error );
}

/**************/

OSErr
FSRefFileCopyFSSpec (
  const FSSpec *     srcSpec,
  const FSSpec *     dstDirSpec,
  ConstHFSUniStr255Param	copyName,
  void *            copyBufferPtr,
  long              copyBufferSize,
  Boolean           preflight)
{
	FSRef srcRef, dstDirRef;
	OSErr err;
	err = FSpMakeFSRef (srcSpec, &srcRef);
	if (err != noErr) return err;
	err = FSpMakeFSRef (dstDirSpec, &dstDirRef);
	if (err != noErr) return err;
	return FSRefFileCopy (&srcRef, &dstDirRef, copyName, copyBufferPtr, copyBufferSize, preflight);
}

/***********************************************************************/

OSErr
FSRefFileCopy (
  const FSRef *     srcRef,
  const FSRef *     dstDirRef,
  ConstHFSUniStr255Param	copyName,
  void *            copyBufferPtr,
  long              copyBufferSize,
  Boolean           preflight)
{
	OSErr	err;

	short	srcRefNum = 0,			/* 0 when source data and resource fork are closed  */
			dstDataRefNum = 0,		/* 0 when destination data fork is closed */
			dstRsrcRefNum = 0;		/* 0 when destination resource fork is closed */
	
	HFSUniStr255 dstName;	/* The filename of the destination. It might be the
									** source filename, it might be a new name... */

	HFSUniStr255 dataForkName;
	HFSUniStr255 resourceForkName;	
	
	FSRef dstFileRef;
		
	Boolean	dstCreated = false,		/* true when destination file has been created */
			ourCopyBuffer = false,	/* true if we had to allocate the copy buffer */
			isDirectory,			/* true if destination is really a directory */
			isDropBox;				/* true if destination is an AppleShare drop box */
		
	Boolean	spaceOK;				/* true if there's enough room to copy the file to the destination volume */

	Boolean	hasDataFork;
	Boolean	hasResourceFork;
	
	// Get names
	
	FSGetDataForkName (&dataForkName);
	FSGetResourceForkName (&resourceForkName);

	/* Preflight for size */
	if ( preflight ) {
		err = PreflightFileCopySpace (srcRef, dstDirRef, &spaceOK);
		if ( err != noErr ) return ( err );
		if ( !spaceOK )	return ( dskFulErr );
	}

	/* get the destination's real dirID and make sure it really is a directory */
	err = GetDestinationDirInfo (dstDirRef, &isDirectory, &isDropBox);
	if ( err != noErr ) goto ErrorExit;
	if ( !isDirectory )	return ( dirNFErr );

	/* Make sure a copy buffer is allocated. */
	if ( copyBufferPtr == NULL )
	{
		/* The caller didn't supply a copy buffer so grab one from the application heap.
		** Try to get a big copy buffer, if we can't, try for a 512-byte buffer.
		** If 512 bytes aren't available, we're in trouble. */
		copyBufferSize = bigCopyBuffSize;
		copyBufferPtr = NewPtr(copyBufferSize);
		if ( copyBufferPtr == NULL )
		{
			copyBufferSize = minCopyBuffSize;
			copyBufferPtr = NewPtr(copyBufferSize);
			if ( copyBufferPtr == NULL ) return ( memFullErr );
		}
		ourCopyBuffer = true;
	}
	
	/* Open the source data fork. */
	err = FSOpenFork (srcRef, dataForkName.length, dataForkName.unicode, fsRdPerm /* srcCopyMode */, &srcRefNum);
	if ( err != noErr ) return ( err );

	/* Once a file is opened, we have to exit via ErrorExit to make sure things are cleaned up */
	
	/* See if the copy will be renamed. */
	if ( copyName != NULL )	{			/* Did caller supply copy file name? */
		BlockMoveData (copyName, &dstName, sizeof (HFSUniStr255));	/* Yes, use the caller supplied copy file name. */
	} else {
		/* They didn't, so get the source file name and use it. */
		err = FSGetCatalogInfo (srcRef, kFSCatInfoNone, NULL, &dstName, NULL, NULL);
		if ( err != noErr ) goto ErrorExit;
	}

	/* Create the destination file. */
	
	err = FSCreateFileUnicode (dstDirRef, dstName.length, dstName.unicode, 
								kFSCatInfoNone, NULL, &dstFileRef, NULL);	
	if ( err != noErr ) goto ErrorExit;
	dstCreated = true;	/* After creating the destination file, any
						** error conditions should delete the destination file */


	/* An AppleShare dropbox folder is a folder for which the user has the Make Changes
	** privilege (write access), but not See Files (read access) and See Folders (search access).
	** Copying a file into an AppleShare dropbox presents some special problems. Here are the
	** rules we have to follow to copy a file into a dropbox:
	** ¥ File attributes can be changed only when both forks of a file are empty.
	** ¥ DeskTop Manager comments can be added to a file only when both forks of a file 
	**   are empty.
	** ¥ A fork can be opened for write access only when both forks of a file are empty.
	** So, with those rules to live with, we'll do those operations now while both forks
	** are empty. */

	if ( isDropBox )
	{
		/* We only set the file attributes now if the file is being copied into a
		** drop box. In all other cases, it is better to set the attributes last
		** so that if FileCopy is modified to give up time to other processes
		** periodicly, the Finder won't try to read any bundle information (because
		** the bundle-bit will still be clear) from a partially copied file. If the
		** copy is into a drop box, we have to set the attributes now, but since the
		** destination forks are opened with write/deny-read/deny-write permissions,
		** any Finder that might see the file in the drop box won't be able to open
		** its resource fork until the resource fork is closed.
		**
		** Note: if you do modify FileCopy to give up time to other processes, don't
		** give up time between the time the destination file is created (above) and
		** the time both forks are opened (below). That way, you stand the best chance
		** of making sure the Finder doesn't read a partially copied resource fork.
		*/
		/* Copy attributes but don't lock the destination. */
		err = FSRefCopyFileMgrAttributes(srcRef, &dstFileRef, false);
		if ( err != noErr ) goto ErrorExit;
	}

	/* Attempt to copy the comments while both forks are empty.
	** Ignore the result because we really don't care if it worked or not. */
	FSRefDTCopyComment (srcRef, &dstFileRef);

	/* See which forks we need to copy. By doing this, we won't create a data or resource fork
	** for the destination unless it's really needed (some foreign file systems such as
	** the ProDOS File System and Macintosh PC Exchange have to create additional disk
	** structures to support resource forks). */
	err = CheckForForks(srcRef, &hasDataFork, &hasResourceFork);
	if ( err != noErr ) goto ErrorExit;
		
	if ( hasDataFork )
	{
		/* Open the destination data fork. */
		err = FSOpenFork (&dstFileRef, dataForkName.length, dataForkName.unicode,
							/* dstCopyMode */ fsRdWrPerm, &dstDataRefNum);
		if ( err != noErr ) goto ErrorExit;
	}

	if ( hasResourceFork )
	{
		/* Open the destination resource fork. */
		err = FSOpenFork (&dstFileRef, resourceForkName.length, resourceForkName.unicode,
							/* dstCopyMode */ fsRdWrPerm, &dstRsrcRefNum);
		if ( err != noErr ) goto ErrorExit;
	}

	if ( hasDataFork )
	{

		/* Copy the data fork. */
		err = FSCopyFork (srcRefNum, dstDataRefNum, copyBufferPtr, copyBufferSize);
		if ( err != noErr ) goto ErrorExit;
	
		/* Close both data forks and clear reference numbers. */
		(void) FSCloseFork (srcRefNum);
		(void) FSCloseFork (dstDataRefNum);
		srcRefNum = dstDataRefNum = 0;
	}
	else
	{
		/* Close the source data fork since it was opened earlier */
		(void) FSClose(srcRefNum);
		srcRefNum = 0;
	}

	if ( hasResourceFork )
	{
		/* Open the source resource fork. */
		err = FSOpenFork (srcRef, resourceForkName.length, resourceForkName.unicode,
							/* srcCopyMode */ fsRdPerm, &srcRefNum);
		if ( err != noErr )
		{
			goto ErrorExit;
		}
	
		/* Copy the resource fork. */
		err = FSCopyFork(srcRefNum, dstRsrcRefNum, copyBufferPtr, copyBufferSize);
		if ( err != noErr )
		{
			goto ErrorExit;
		}
	
		/* Close both resource forks and clear reference numbers. */
		(void) FSCloseFork (srcRefNum);
		(void) FSCloseFork (dstRsrcRefNum);
		srcRefNum = dstRsrcRefNum = 0;
	}
	
	/* Get rid of the copy buffer if we allocated it. */
	if ( ourCopyBuffer )
	{
		DisposePtr((Ptr)copyBufferPtr);
	}

	/* Attempt to copy attributes again to set mod date.  Copy lock condition this time
	** since we're done with the copy operation.  This operation will fail if we're copying
	** into an AppleShare dropbox, so we don't check for error conditions. */
	FSRefCopyFileMgrAttributes(srcRef, &dstFileRef, true);

	/* Hey, we did it! */
	return ( noErr );
	
ErrorExit:
	if ( srcRefNum != 0 )		(void) FSCloseFork (srcRefNum);		/* Close the source file */
	if ( dstDataRefNum != 0 ) 	(void) FSCloseFork (dstDataRefNum);	/* Close the destination file data fork */
	if ( dstRsrcRefNum != 0 )	(void) FSCloseFork (dstRsrcRefNum);	/* Close the destination file resource fork */
	if ( dstCreated )			(void) FSDeleteObject (&dstFileRef);	/* Delete dest file.  This may fail if the file 
												   is in a "drop folder" */
	if ( ourCopyBuffer )		DisposePtr((Ptr)copyBufferPtr);   /* dispose of any memory we allocated */
	return ( err );
}

/*****************************************************************************/

/* local constants */

enum
{
	dirCopyBigCopyBuffSize  = 0x00004000,
	dirCopyMinCopyBuffSize  = 0x00000200
};


/*****************************************************************************/

/* local data structures */

/* The EnumerateGlobals structure is used to minimize the amount of
** stack space used when recursively calling CopyLevel and to hold
** global information that might be needed at any time. */

struct EnumerateGlobals
{
	Ptr			copyBuffer;			/* pointer to buffer used for file copy operations */
	long		bufferSize;			/* the size of the copy buffer */
	FSCopyErrProcPtr errorHandler;	/* pointer to error handling function */
	FSCopyFilterProcPtr copyFilterProc; /* pointer to filter function */
	OSErr		error;				/* temporary holder of results - saves 2 bytes of stack each level */
	Boolean		bailout;			/* set to true to by error handling function if fatal error */
	HFSUniStr255	itemName;			/* the name of the current item */
	FSRef   	item;	
	FSCatalogInfo catInfo;			
};

typedef struct EnumerateGlobals EnumerateGlobals;
typedef EnumerateGlobals *EnumerateGlobalsPtr;


/* The PreflightGlobals structure is used to minimize the amount of
** stack space used when recursively calling GetLevelSize and to hold
** global information that might be needed at any time. */

struct PreflightGlobals
{
	OSErr			result;				/* temporary holder of results - saves 2 bytes of stack each level */
	FSRef			item;				/* the parameter block used for PBGetCatInfo calls */
	FSCatalogInfo	catInfo;
	unsigned long	dstBlksPerAllocBlk;	/* the number of 512 byte blocks per allocation block on destination */
										
	unsigned long	allocBlksNeeded;	/* the total number of allocation blocks needed  */

	unsigned long	tempBlocks;			/* temporary storage for calculations (save some stack space)  */
	FSCopyFilterProcPtr copyFilterProc;	/* pointer to filter function */
};

typedef struct PreflightGlobals PreflightGlobals;
typedef PreflightGlobals *PreflightGlobalsPtr;

/*****************************************************************************/

/* static prototypes */

static	void	GetLevelSize(long currentDirID,
							 PreflightGlobals *theGlobals);

static	OSErr	PreflightDirectoryCopySpace(const FSRef *srcRef,
											const FSRef *dstRef,
											CopyFilterProcPtr copyFilterProc,
											Boolean *spaceOK);

static	void	CopyLevel(long sourceDirID,
						  long dstDirID,
						  EnumerateGlobals *theGlobals);
						  
/*****************************************************************************/

static void	
GetLevelSize (const FSRef *currentDirRef, PreflightGlobals *theGlobals)
{
	FSIterator iterator;
	FSOpenIterator (currentDirRef, kFSIterateFlat, &iterator);
	
	do
	{
		ItemCount actualObjects;
		theGlobals->result = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, kFSCatInfoNodeFlags, 
						&theGlobals->catInfo, &theGlobals->item, NULL, NULL);

		if ( (theGlobals->result == noErr) && (actualObjects == 1) )
		{
			::YieldToAnyThread ();
			if ( (theGlobals->copyFilterProc == NULL) ||
				 CallFSCopyFilterProc(theGlobals->copyFilterProc, &theGlobals->item) ) /* filter if filter proc was supplied */
			{
				/* Either there's no filter proc OR the filter proc says to use this item */
				if ( theGlobals->catInfo.nodeFlags & kFSNodeIsDirectoryMask )
				{
					/* we have a directory */
					
					GetLevelSize(&theGlobals->item, theGlobals); /* recurse */
					theGlobals->result = noErr; /* clear error return on way back */
				}
				else
				{
					/* We have a file - add its allocation blocks to allocBlksNeeded. */
					/* Since space on Mac OS disks is always allocated in allocation blocks, */
					/* this takes into account rounding up to the end of an allocation block. */
					
					/* get number of 512-byte blocks needed for data fork */
					if ( ((unsigned long) theGlobals->catInfo.dataLogicalSize & 0x000001ff) != 0 )
					{
						theGlobals->tempBlocks = ((unsigned long)theGlobals->catInfo.dataLogicalSize >> 9) + 1;
					}
					else
					{
						theGlobals->tempBlocks = (unsigned long)theGlobals->catInfo.dataLogicalSize >> 9;
					}
					/* now, calculate number of new allocation blocks needed for the data fork and add it to the total */
					if ( theGlobals->tempBlocks % theGlobals->dstBlksPerAllocBlk )
					{
						theGlobals->allocBlksNeeded += (theGlobals->tempBlocks / theGlobals->dstBlksPerAllocBlk) + 1;
					}
					else
					{
						theGlobals->allocBlksNeeded += theGlobals->tempBlocks / theGlobals->dstBlksPerAllocBlk;
					}
					
					/* get number of 512-byte blocks needed for resource fork */
					if ( ((unsigned long)theGlobals->catInfo.rsrcLogicalSize & 0x000001ff) != 0 )
					{
						theGlobals->tempBlocks = ((unsigned long)theGlobals->catInfo.rsrcLogicalSize >> 9) + 1;
					}
					else
					{
						theGlobals->tempBlocks = (unsigned long)theGlobals->catInfo.rsrcLogicalSize >> 9;
					}
					/* now, calculate number of new allocation blocks needed for the resource  fork and add it to the total */
					if ( theGlobals->tempBlocks % theGlobals->dstBlksPerAllocBlk )
					{
						theGlobals->allocBlksNeeded += (theGlobals->tempBlocks / theGlobals->dstBlksPerAllocBlk) + 1;
					}
					else
					{
						theGlobals->allocBlksNeeded += theGlobals->tempBlocks / theGlobals->dstBlksPerAllocBlk;
					}
				}
			}
		}
	} while ( theGlobals->result == noErr );
	FSCloseIterator (iterator);
}

/*****************************************************************************/

static	OSErr	PreflightDirectoryCopySpace(const FSRef *srcRef,
											const FSRef *dstRef,
											FSCopyFilterProcPtr copyFilterProc,
											Boolean *spaceOK)
{
	OSErr error;
	unsigned long dstFreeBlocks;
	PreflightGlobals theGlobals;
	
	FSCatalogInfo catInfo;
	FSVolumeInfo  volInfo;
	
	error = FSGetCatalogInfo (dstRef, kFSCatInfoVolume, &catInfo, NULL, NULL, NULL);
	if (error != noErr) return error;
	
	error = FSGetVolumeInfo (catInfo.volume, 0, NULL, kFSVolInfoSizes | kFSVolInfoBlocks, 
    			&volInfo, NULL, NULL);

	if ( error == noErr )
	{
		/* Convert freeBytes to free disk blocks (512-byte blocks) */
		dstFreeBlocks = U32SetU(U64ShiftRight(volInfo.freeBytes, 9));
		
		/* get allocation block size (always multiple of 512) and divide by 512
		  to get number of 512-byte blocks per allocation block */
		theGlobals.dstBlksPerAllocBlk = ((unsigned long) volInfo.blockSize >> 9);
		
		theGlobals.allocBlksNeeded = 0;
				
		theGlobals.copyFilterProc = copyFilterProc;
		
		GetLevelSize(srcRef, &theGlobals);
		
		/* Is there enough room on the destination volume for the source file?					*/
		/* Note:	This will work because the largest number of disk blocks supported			*/
		/*			on a 2TB volume is 0xffffffff and (allocBlksNeeded * dstBlksPerAllocBlk)	*/
		/*			will always be less than 0xffffffff.										*/
		*spaceOK = ((theGlobals.allocBlksNeeded * theGlobals.dstBlksPerAllocBlk) <= dstFreeBlocks);
	}

	return ( error );
}

/*****************************************************************************/

static	void	CopyLevel(const FSRef *src,
						  const FSRef *dst,
						  EnumerateGlobals *theGlobals)
{
	FSIterator iterator;
	FSOpenIterator (src, kFSIterateFlat, &iterator);

	do
	{	
		/* Get next source item at the current directory level */
		ItemCount actualObjects;
		theGlobals->error = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, kFSCatInfoNodeFlags, 
						&theGlobals->catInfo, &theGlobals->item, NULL, &theGlobals->itemName);
								
		if ( (theGlobals->error == noErr) && (actualObjects == 1) )
		{
			::YieldToAnyThread ();
			if ( (theGlobals->copyFilterProc == NULL) ||
				 CallFSCopyFilterProc(theGlobals->copyFilterProc, &theGlobals->item) ) /* filter if filter proc was supplied */
			{
				/* Either there's no filter proc OR the filter proc says to use this item */

				/* We have an item.  Is it a file or directory? */
				if ( theGlobals->catInfo.nodeFlags & kFSNodeIsDirectoryMask )
				{
					/* We have a directory */
					
					/* Create a new directory at the destination. No errors allowed! */
							
					FSRef newDestDir;						
					theGlobals->error = FSCreateDirectoryUnicode (dst, theGlobals->itemName.length, theGlobals->itemName.unicode,
						kFSCatInfoNone, NULL, &newDestDir, NULL, NULL);

					if ( theGlobals->error == noErr )
					{
						/* Save the current item -- since it will change on recursion. */
						FSRef currentSrcDir;
						BlockMoveData (&theGlobals->item, &currentSrcDir, sizeof (currentSrcDir));
						
						/* Dive again (copy the directory level we just found below this one) */
						CopyLevel (&theGlobals->item, &newDestDir, theGlobals);
						
						if ( !theGlobals->bailout )
						{
							/* Copy comment from old to new directory. */
							/* Ignore the result because we really don't care if it worked or not. */
							(void) FSRefDTCopyComment (&currentSrcDir, &newDestDir);
							
							/* Copy directory attributes (dates, etc.) to newDirID. */
							/* No errors allowed */
							theGlobals->error = FSRefCopyFileMgrAttributes(&currentSrcDir, &newDestDir, true);
							
							/* handle any errors from CopyFileMgrAttributes */
							if ( theGlobals->error != noErr )
							{
								if ( theGlobals->errorHandler != NULL )
								{
									theGlobals->bailout =  CallFSCopyErrProc(theGlobals->errorHandler, theGlobals->error, copyDirFMAttributesOp,
															&currentSrcDir, &newDestDir);
								}
								else
								{
									/* If you don't handle the errors with an error handler, */
									/* then the copy stops here. */
									theGlobals->bailout = true;
								}
							}
						}
					}
					else	/* error handling for DirCreate */
					{
						if ( theGlobals->errorHandler != NULL )
						{
							theGlobals->bailout = CallFSCopyErrProc( theGlobals->errorHandler, 
													theGlobals->error, dirCreateOp,
													&theGlobals->item, &newDestDir );
						}
						else
						{
							/* If you don't handle the errors with an error handler, */
							/* then the copy stops here. */
							theGlobals->bailout = true;
						}
					}
					
					if ( !theGlobals->bailout )
					{
						/* clear error return on way back if we aren't bailing out */
						theGlobals->error = noErr;
					}
				}
				else
				{
					/* We have a file, so copy it */
										
					theGlobals->error = FSRefFileCopy (&theGlobals->item, dst, NULL,
												theGlobals->copyBuffer, theGlobals->bufferSize, false);
							
					/* handle any errors from FileCopy */
					if ( theGlobals->error != noErr )
					{
						if ( theGlobals->errorHandler != NULL )
						{
							theGlobals->bailout = CallFSCopyErrProc(theGlobals->errorHandler, theGlobals->error, fileCopyOp,
													&theGlobals->item, dst);
							if ( !theGlobals->bailout )
							{
								/* If the CopyErrProc handled the problem, clear the error here */
								theGlobals->error = noErr;
							}
						}
						else
						{
							/* If you don't handle the errors with an error handler, */
							/* then the copy stops here. */
							theGlobals->bailout = true;
						}
					}
				}
			}
		}
		else
		{	/* error handling for PBGetCatInfo */
			/* it's normal to get a fnfErr when indexing; that only means you've hit the end of the directory */
			if ( (theGlobals->error != errFSNoMoreItems) && (theGlobals->error != noErr) )
			{
				if ( theGlobals->errorHandler != NULL )
				{ 
					theGlobals->bailout = CallFSCopyErrProc(theGlobals->errorHandler, theGlobals->error, getNextItemOp,
											src, NULL);
					if ( !theGlobals->bailout )
					{
						/* If the CopyErrProc handled the problem, clear the error here */
						theGlobals->error = noErr;
					}
				}
				else
				{
					/* If you don't handle the errors with an error handler, */
					/* then the copy stops here. */
					theGlobals->bailout = true;
				}
			}
		}
	} while ( (theGlobals->error == noErr) && (!theGlobals->bailout) ); /* time to fall back a level? */
	
	FSCloseIterator (iterator);
}

/*****************************************************************************/
OSErr
FSRefFilteredDirectoryCopy(
  const FSRef *       srcRef,
  const FSRef *       dstRef,
  ConstHFSUniStr255Param 	  copyName,
  void *              copyBufferPtr,
  long                copyBufferSize,
  Boolean             preflight,
  FSCopyErrProcPtr      copyErrHandler,
  FSCopyFilterProcPtr   copyFilterProc)
{
	EnumerateGlobals theGlobals;
	Boolean	isDirectory;
	OSErr	error;
	Boolean ourCopyBuffer = false;
	HFSUniStr255 srcDirName;
	Boolean spaceOK;	
	FSCatalogInfo catInfo;		
	
	/* Make sure a copy buffer is allocated. */
	if ( copyBufferPtr == NULL )
	{
		/* The caller didn't supply a copy buffer so grab one from the application heap.
		** Try to get a big copy buffer, if we can't, try for a 512-byte buffer.
		** If 512 bytes aren't available, we're in trouble. */
		copyBufferSize = dirCopyBigCopyBuffSize;
		copyBufferPtr = NewPtr(copyBufferSize);
		if ( copyBufferPtr == NULL )
		{
			copyBufferSize = dirCopyMinCopyBuffSize;
			copyBufferPtr = NewPtr(copyBufferSize);
			if ( copyBufferPtr == NULL )
			{
				return ( memFullErr );
			}
		}
		ourCopyBuffer = true;
	}
	
	/* Get the real dirID where we're copying from and make sure it is a directory. */
	error = FSGetCatalogInfo (srcRef, kFSCatInfoNodeFlags, &catInfo, NULL, NULL, NULL);
	isDirectory = catInfo.nodeFlags & kFSNodeIsDirectoryMask;

	if ( error != noErr ) goto ErrorExit;
	if ( !isDirectory ) {
		error = dirNFErr;
		goto ErrorExit;
	}
	
	/*  Get the real dirID where we're going to put the copy and make sure it is a directory. */
	error = FSGetCatalogInfo (dstRef, kFSCatInfoNodeFlags, &catInfo, NULL, NULL, NULL);
	isDirectory = catInfo.nodeFlags & kFSNodeIsDirectoryMask;
	if ( error != noErr ) goto ErrorExit;
	if ( !isDirectory ) {
		error =  dirNFErr;
		goto ErrorExit;
	}
		
	if ( preflight )
	{
		error = PreflightDirectoryCopySpace(srcRef, dstRef, copyFilterProc, &spaceOK);
		if ( error != noErr ) goto ErrorExit;
		if ( !spaceOK ) {
			error = dskFulErr; /* not enough room on destination */
			goto ErrorExit;
		}
	}

	/* Create the new directory in the destination directory with the */
	/* same name as the source directory. */
	error = FSGetCatalogInfo (srcRef, kFSCatInfoNone, NULL, &srcDirName, NULL, NULL);
	if ( error != noErr ) goto ErrorExit;
	
	/* use the copyName as srcDirName if supplied */
	if (copyName == NULL) copyName = &srcDirName;
	FSRef newDstRef;
	if (error == noErr) {
		error = FSCreateDirectoryUnicode (dstRef, copyName->length, copyName->unicode,
					kFSCatInfoNone, NULL, &newDstRef, NULL, NULL);
	}

	if ( error != noErr )
	{
		/* handle any errors from DirCreate */
		if ( copyErrHandler != NULL )
		{
			if ( CallFSCopyErrProc(copyErrHandler, error, dirCreateOp, srcRef, dstRef) )
				// This isn't quite right. It doesn't tell the error procedure exactly
				// what we were trying to create. May need to fix this.
			{
				goto ErrorExit;
			}
			else
			{
				/* If the CopyErrProc handled the problem, clear the error here */
				/* and continue */
				error = noErr;
			}
		}
		else
		{
			/* If you don't handle the errors with an error handler, */
			/* then the copy stops here. */
			goto ErrorExit;
		}
	}
	
	/* newDstRef is now the newly created directory! */
		
	/* Set up the globals we need to access from the recursive routine. */
	theGlobals.copyBuffer = (Ptr)copyBufferPtr;
	theGlobals.bufferSize = copyBufferSize;
	theGlobals.errorHandler = copyErrHandler;
	theGlobals.bailout = false;
	theGlobals.copyFilterProc =  copyFilterProc;
		
	/* Here we go into recursion land... */
	CopyLevel(srcRef, &newDstRef, &theGlobals);
	error = theGlobals.error;	/* get the result */
	
	if ( !theGlobals.bailout )
	{
		/* Copy comment from source to destination directory. */
		/* Ignore the result because we really don't care if it worked or not. */
		(void) FSRefDTCopyComment(srcRef, &newDstRef);
		
		/* Copy the File Manager attributes */
		error = FSRefCopyFileMgrAttributes(srcRef, &newDstRef, true);
		
		/* handle any errors from CopyFileMgrAttributes */
		if ( (error != noErr) && (copyErrHandler != NULL) )
		{
			theGlobals.bailout = CallFSCopyErrProc(copyErrHandler, error, copyDirFMAttributesOp,
												srcRef, &newDstRef);
		}
	}

ErrorExit:
	/* Get rid of the copy buffer if we allocated it. */
	if ( ourCopyBuffer )
	{
		DisposePtr((Ptr)copyBufferPtr);
	}

	return ( error );
}

/**************/

OSErr
FSRefDirectoryCopy(
  const FSRef *      srcRef,
  const FSRef *      dstDirRef,
  ConstHFSUniStr255Param copyName,
  void *             copyBufferPtr,
  long               copyBufferSize,
  Boolean            preflight,
  FSCopyErrProcPtr   copyErrHandler)
{
	return FSRefFilteredDirectoryCopy (	srcRef, dstDirRef, copyName, 
										copyBufferPtr, copyBufferSize, preflight, 
										copyErrHandler, NULL);
}



OSErr	
FSRefDeleteDirectoryContents (FSRef *directory)
{
	OSErr error;
	FSCatalogInfo catInfo;
	error = FSGetCatalogInfo (directory, kFSCatInfoNodeFlags, &catInfo, NULL, NULL, NULL);
	if (error != noErr) return error;
	if (catInfo.nodeFlags & kFSNodeIsDirectoryMask) {
		// iterate through the directory, deleting as we go. We can't iterate in the usual way,
		// so we keep creating & releasing the iterator.
		FSIterator iterator;
		ThrowIfOSErr_AC (FSOpenIterator (directory, kFSIterateFlat, &iterator));
		while (true) {
			::YieldToAnyThread ();
			ItemCount actualObjects;
			FSRef item;
			error = FSGetCatalogInfoBulk (iterator, 1, &actualObjects, NULL, 
												kFSCatInfoNodeFlags, &catInfo, &item, NULL, NULL);
			if (error == errFSNoMoreItems) {
				error = noErr;
				FSCloseIterator (iterator);
				break;
			} else if (error != noErr) {
				FSCloseIterator (iterator);
				return error;
			} else if (actualObjects == 1) {
				FSCloseIterator (iterator);
				if (catInfo.nodeFlags & kFSNodeIsDirectoryMask) {
					error = FSRefDeleteDirectory (&item);
				} else {
					error = FSDeleteObject (&item);
				}
				if (error != noErr) return error;
				ThrowIfOSErr_AC (FSOpenIterator (directory, kFSIterateFlat, &iterator));
			} 
		}
	} else {
		return errFSNotAFolder;
	}
	
	return error;
}

/*****************************************************************************/

OSErr	
FSRefDeleteDirectory (FSRef *directory)
{
	OSErr error;
	
	error = FSRefDeleteDirectoryContents (directory);
	if (error == noErr) {
		error = FSDeleteObject (directory);
	}
	
	return error;
}


