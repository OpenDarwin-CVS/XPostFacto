/*

    Copyright (c) 2001
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

	FSRefCopying.h
	==============
		
	The idea is to copy files using FSRefs, to preserve long file names.

	This code is descended from Apple's MoreFiles code, but with
	extensive changes.
			
	One known problem is that the semantics of the error-handling callbacks
	are not entirely consistent. I haven't really paid attention to them,
	as I'm not using them at the moment (I suppose I should).
	
*/

#ifndef __FSREFCOPYING__
#define __FSREFCOPYING__

#include <MacTypes.h>
#include <Files.h>

#include "DirectoryCopy.h"

#ifdef __cplusplus
extern "C" {
#endif

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
);

OSErr
FSRefDTCopyComment (const FSRef *srcRef, const FSRef *dstRef);


OSErr	FSCopyFork(		 short srcRefNum,
						 short dstRefNum,
						 void *copyBufferPtr,
						 long copyBufferSize);
						 
/*
    The FSCopyFork function copies all data from the source fork to the
    destination fork of open file forks and makes sure the destination EOF
    is equal to the source EOF.
    
    srcRefNum       input:  The source file reference number.
    dstRefNum       input:  The destination file reference number.
    copyBufferPtr   input:  Pointer to buffer to use during copy. The
                            buffer should be at least 512-bytes minimum.
                            The larger the buffer, the faster the copy.
    copyBufferSize  input:  The size of the copy buffer.
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Disk full   
        ioErr               –36     Data does not match in read-verify mode
        fnOpnErr            -38     File not open
        wPrErr              -44     Hardware volume lock    
        fLckdErr            -45     File is locked  
        vLckdErr            -46     Software volume lock    
        rfNumErr            -51     Bad reference number
        wrPermErr           -61     Read/write permission doesn’t
                                    allow writing   
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file
*/
						 




OSErr	
FSRefCopyFileMgrAttributes (const FSRef *srcRef,
							const FSRef *dstRef,
							Boolean copyLockBit);
							
/*
    The FSRefCopyFileMgrAttributes function copies all File Manager attributes
    from the source file or directory to the destination file or directory.
    If copyLockBit is true, then set the locked state of the destination
    to match the source.

    srcRef	    input:  Pointer to source FSRef.
	dstRef		input:	Pointer to destintation FSRef.
    copyLockBit input:  If true, set the locked state of the destination
                        to match the source.
    
    Result Codes
        noErr               0       No error
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        fnfErr              -43     File not found
        fLckdErr            -45     File is locked
        vLckdErr            -46     Volume is locked or read-only
        paramErr            -50     No default volume
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname
    
*/

/***************/

OSErr
FSRefFileCopyFSSpec (
  const FSSpec *     srcSpec,
  const FSSpec *     dstDirSpec,
  ConstHFSUniStr255Param	copyName,
  void *            copyBufferPtr,
  long              copyBufferSize,
  Boolean           preflight);




/*****************************************************************************/


OSErr
FSRefFileCopy (
  const FSRef *     srcRef,
  const FSRef *     dstDirRef,
  ConstHFSUniStr255Param	copyName,
  void *            copyBufferPtr,
  long              copyBufferSize,
  Boolean           preflight);


/*
    The FSRefFileCopy function duplicates a file and optionally renames it.
	The srcRef is used to
    determine the location of the file to copy.  The dstRef is
    used to determine the location of the
    destination directory.  If copyName <> NIL, then it points
    to the name of the new file.  If copyBufferPtr <> NIL, it
    points to a buffer of copyBufferSize that is used to copy
    the file's data.  The larger the supplied buffer, the
    faster the copy.  If copyBufferPtr = NIL, then this routine
    allocates a buffer in the application heap. If you pass a
    copy buffer to this routine, make its size a multiple of 512
    ($200) bytes for optimum performance.
    
    srcRef       	input:  An FSRef record specifying the source file.
    dstRef          input:  An FSRef record specifying the destination
                            directory.
    copyName        input:  Points to the new file name if the file is
                            to be renamed or nil if the file isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want FileCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, FSpFileCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold both the data and resource forks
                            before starting the copy.
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   FSRefDirectoryCopy
*/


/*****************************************************************************/

typedef CALLBACK_API( Boolean , FSCopyErrProcPtr )(OSErr error, short failedOperation, const FSRef * srcRef, const FSRef * dstRef);
/*
    This is the prototype for the CopyErrProc function DirectoryCopy
    calls if an error condition is detected sometime during the copy.  If
    CopyErrProc returns false, then DirectoryCopy attempts to continue with
    the directory copy operation.  If CopyErrProc returns true, then
    DirectoryCopy stops the directory copy operation.

    error           input:  The error result code that caused CopyErrProc to
                            be called.
    failedOperation input:  The operation that returned an error to
                            DirectoryCopy.
    srcVRefNum      input:  Source volume specification.
    srcDirID        input:  Source directory ID.
    srcName         input:  Source file or directory name, or nil if
                            srcDirID specifies the directory.
    dstVRefNum      input:  Destination volume specification.
    dstDirID        input:  Destination directory ID.
    dstName         input:  Destination file or directory name, or nil if
                            dstDirID specifies the directory.

    __________
    
    Also see:   FSRefFilteredDirectoryCopy, FSRefDirectoryCopy
*/
#define CallFSCopyErrProc(userRoutine, error, failedOperation, srcRefFS, dstFSRef) \
    (*(userRoutine))((error), (failedOperation), (srcRefFS), (dstFSRef))

/*****************************************************************************/

typedef CALLBACK_API( Boolean , FSCopyFilterProcPtr )(const FSRef * src);
/*
    This is the prototype for the CopyFilterProc function called by
    FilteredDirectoryCopy and GetLevelSize. If true is returned,
    the file/folder is included in the copy, otherwise it is excluded.
    
    pb  input:  Points to the CInfoPBRec for the item under consideration.

    __________
    
    Also see:   FSRefFilteredDirectoryCopy
*/
#define CallFSCopyFilterProc(userRoutine, src) \
    (*(userRoutine))((src))

/*****************************************************************************/

OSErr
FSRefFilteredDirectoryCopy(
  const FSRef *       srcRef,
  const FSRef *       dstDirRef,
  ConstHFSUniStr255Param 	  copyName,
  void *              copyBufferPtr,
  long                copyBufferSize,
  Boolean             preflight,
  FSCopyErrProcPtr      copyErrHandler,
  FSCopyFilterProcPtr   copyFilterProc);


/*
    The FSRefFilteredDirectoryCopy function makes a copy of a directory
    structure in a new location. If copyBufferPtr <> NIL, it points to
    a buffer of copyBufferSize that is used to copy files data. The
    larger the supplied buffer, the faster the copy. If
    copyBufferPtr = NIL, then this routine allocates a buffer in the
    application heap. If you pass a copy buffer to this routine, make
    its size a multiple of 512 ($200) bytes for optimum performance.
    
    The optional copyFilterProc parameter lets a routine you define
    decide what files or directories are copied to the destination.
    
    srcRef          input:  An FSRef record specifying the directory to copy.
    dstRef          input:  An FSRef record specifying destination directory
                            of the copy.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, FSpDirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    copyFilterProc  input:  A pointer to the filter routine you want called
                            for each item in the source directory, or NULL
                            if you don't want to filter.
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, CopyFilterProcPtr, FSRefDirectoryCopy, FSRefFileCopy
*/

/*****************************************************************************/

OSErr
FSRefDirectoryCopy(
  const FSRef *      srcRef,
  const FSRef *      dstDirRef,
  ConstHFSUniStr255Param copyName,
  void *             copyBufferPtr,
  long               copyBufferSize,
  Boolean            preflight,
  FSCopyErrProcPtr   copyErrHandler);


/*
    The FSRefDirectoryCopy function makes a copy of a directory structure in a
    new location. If copyBufferPtr <> NIL, it points to a buffer of
    copyBufferSize that is used to copy files data.  The larger the
    supplied buffer, the faster the copy.  If copyBufferPtr = NIL, then this
    routine allocates a buffer in the application heap. If you pass a
    copy buffer to this routine, make its size a multiple of 512
    ($200) bytes for optimum performance.
    
    srcRef          input:  An FSRef record specifying the directory to copy.
    dstRef          input:  An FSRef record specifying destination directory
                            of the copy.
    copyName        input:  Points to the new directory name if the directory
                            is to be renamed or nil if the directory isn't to
                            be renamed.
    copyBufferPtr   input:  Points to a buffer of copyBufferSize that
                            is used the i/o buffer for the copy or
                            nil if you want DirectoryCopy to allocate its
                            own buffer in the application heap.
    copyBufferSize  input:  The size of the buffer pointed to
                            by copyBufferPtr.
    preflight       input:  If true, FSpDirectoryCopy makes sure there are
                            enough allocation blocks on the destination
                            volume to hold the directory's files before
                            starting the copy.
    copyErrHandler  input:  A pointer to the routine you want called if an
                            error condition is detected during the copy, or
                            nil if you don't want to handle error conditions.
                            If you don't handle error conditions, the first
                            error will cause the copy to quit and
                            DirectoryCopy will return the error.
                            Error handling is recommended...
    
    Result Codes
        noErr               0       No error
        readErr             –19     Driver does not respond to read requests
        writErr             –20     Driver does not respond to write requests
        badUnitErr          –21     Driver reference number does not
                                    match unit table
        unitEmptyErr        –22     Driver reference number specifies a
                                    nil handle in unit table
        abortErr            –27     Request aborted by KillIO
        notOpenErr          –28     Driver not open
        dskFulErr           -34     Destination volume is full
        nsvErr              -35     No such volume
        ioErr               -36     I/O error
        bdNamErr            -37     Bad filename
        tmfoErr             -42     Too many files open
        fnfErr              -43     Source file not found, or destination
                                    directory does not exist
        wPrErr              -44     Volume locked by hardware
        fLckdErr            -45     File is locked
        vLckdErr            -46     Destination volume is read-only
        fBsyErr             -47     The source or destination file could
                                    not be opened with the correct access
                                    modes
        dupFNErr            -48     Destination file already exists
        opWrErr             -49     File already open for writing
        paramErr            -50     No default volume or function not
                                    supported by volume
        permErr             -54     File is already open and cannot be opened using specified deny modes
        memFullErr          -108    Copy buffer could not be allocated
        dirNFErr            -120    Directory not found or incomplete pathname
        wrgVolTypErr        -123    Function not supported by volume
        afpAccessDenied     -5000   User does not have the correct access
        afpDenyConflict     -5006   The source or destination file could
                                    not be opened with the correct access
                                    modes
        afpObjectTypeErr    -5025   Source is a directory, directory not found
                                    or incomplete pathname
    
    __________
    
    Also see:   CopyErrProcPtr, DirectoryCopy, FilteredDirectoryCopy,
                FSpFilteredDirectoryCopy, FileCopy, FSpFileCopy
*/

/*****************************************************************************/

OSErr FSRefDeleteDirectory (FSRef *directory);
OSErr FSRefDeleteDirectoryContents (FSRef *directory);


#ifdef __cplusplus
}
#endif

#endif /* __FSREFCOPYING__ */

