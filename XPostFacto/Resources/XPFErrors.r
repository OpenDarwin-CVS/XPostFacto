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

#include "XPFErrors.h"

#include "MacAppTypes.r"

#define kOperationResourceID 1130
#define kReasonResourceID 1128
#define kRecoveryResourceID 1129

// operation table & strings

resource 'errs' (kOperationResourceID, purgeable) {
	{	
		whichList, 0, kOperationResourceID;
	}
};

resource 'STR#' (kOperationResourceID, purgeable) {
	{
		"";	
	}
};

// reason table & strings

resource 'errs' (kReasonResourceID, purgeable) {
	{
		whichList, 0, kReasonResourceID;
		kErrorWritingNVRAM, kErrorWritingNVRAM, 1;
		kVolumeNotWriteable, kVolumeNotWriteable, 2;
		kErrorReadingPartitionMap, kErrorReadingPartitionMap, 3;
		kInvalidPartitionMap, kInvalidPartitionMap, 4;
		kErrorReadingCapacity, kErrorReadingCapacity, 5;
		kWrite512ByteBlocksOnly, kWrite512ByteBlocksOnly, 6;
		kErrorWritingBlocks, kErrorWritingBlocks, 7;
		kErrorReadingBlocks, kErrorReadingBlocks, 8;
		kErrorFindingVolumeHeader, kErrorFindingVolumeHeader, 9;
		kErrorFindingBootXInCatalog, kErrorFindingBootXInCatalog, 10;
		kRecursionErrorInstallingBootX, kRecursionErrorInstallingBootX, 11;
		kErrorExtractingBootX, kErrorExtractingBootX, 12;
		kCatalogFileTooFragmented, kCatalogFileTooFragmented, 13;
		kCatalogFileInconsistent, kCatalogFileInconsistent, 14;
		kUnalignedAllocationBlockRead, kUnalignedAllocationBlockRead, 15;
		kReadCrossesExtents, kReadCrossesExtents, 16;
		kUserCancelledInstall, kUserCancelledInstall, 17;
		kExtentsFileTooFragmented, kExtentsFileTooFragmented, 18;
		kExtentsFileInconsistent, kExtentsFileInconsistent, 19;
		kCouldNotObtainAuthorization, kCouldNotObtainAuthorization, 20;
		kWrongNVRAMType, kWrongNVRAMType, 21;
		kNoSuchNVRAMKey, kNoSuchNVRAMKey, 22;
		kNVRAMPatchParseError, kNVRAMPatchParseError, 23;
		kWritePartitionOSX, kWritePartitionOSX, 24;
	}
};


resource 'STR#' (kReasonResourceID, purgeable) {
	{
		/* 1 */		"there was a problem writing to NVRAM";
					"the volume was not writeable";
					"there was an error reading a partition map";
					"a partition map was invalid";
		/* 5 */		"an error occurred while reading capacity data";
					"I only know how to write to 512 byte blocks";
					"there was an error while writing blocks";
					"there was an error while reading blocks";
					"there was an error finding the volume header";
		/* 10 */	"there was an error finding BootX in the catalog file";
					"there was a recursion error while installing BootX";
					"there was an error extracting BootX. This may be because the volume is highly fragmented";
					"the catalog file was too fragmented";
					"the catalog file was inconsistent";
		/* 15 */	"reads must be aligned on allocation blocks (for now)";
					"reads cannot cross extent boundaries (yet)";
					"installation was cancelled by the user";
					"the extents file was too fragmented";
					"the extents file was inconsistent";
		/* 20 */	"could not obtain authorization";
					"wrong NVRAM type (internal error)";
					"no such NVRAM key (internal error)";
					"error parsing NVRAM patch (internal error)";
					
					"the Mac OS X version of XPostFacto is unable to install BootX (yet). Try rebooting in Mac OS 9."
	}
};

// recovery table & strings

resource 'errs' (kRecoveryResourceID, purgeable) {
	{
		whichList, 0, kRecoveryResourceID;
	}
};

resource 'STR#' (kRecoveryResourceID, purgeable) {
	{
		"";	
	}
};


//  Recipe
//
// To use application message tables:
//
// 1) Define constants for the application's message and error:
//  CONST
//  msgCantAcceptChar	= messageLookup + -26002;
//  {Note: Applications should use errors between -26000 and -29999.}
//  errCantAddChar= -26002;
//  Adding the predefined constant messageLookup to the message signals ErrorAlert to use the message
//  tables to find text to substitute into the generic alert.
//
//  2) Define the message tables in the application's resource file:
//  First, the operation table and its STR# list:
//  resource 'errs' (1130, purgeable) { // application's operation table 
//  {	whichList, 0, 2103, // indicates that STR# 2103 will be used by this table 
//  -26002, -26002, 1
//  }
//  };
//  resource 'STR#' (2103, purgeable) { // application's operation strings 
//  {
//  "add another character"
//  }
//  };
//
// Next, the errortable and its STR# list:
//  resource 'errs' (1128, purgeable) { // application's reason table 
//  {	whichList, 0, 2101;
//  -26002, -26002, 1
//  }
//  };
//  resource 'STR#' (2101, purgeable) { // application's error reason strings 
//  {
//  "only ^3 characters are allowed"
//  }
//  };
//
// Note that ^3 can be used in any message CPascalStr_AC. ErrorAlert always subsitutes the text
// in the global variable gErrorParm3 for ^3, if present, in the message strings. Thus,
// gErrorParm3 can be used by the application to further customize the alert according to
// the context of the program at the time the error was encountered.
//
//  Finally, the recovery table and its STR# list:
//  resource 'errs' (1129, purgeable) { // application's recovery table 
//  {	whichList, 0, 2102;
//  -26002, -26002, 1
//  }
//  };
//  resource 'STR#' (2102, purgeable) { // application's error recovery strings 
//  {
//  "Delete some characters before adding new ones."
//  }
//  };
//
//  3) If, in the course of executing the program, an error is encountered, call Failure:
//  PROCEDURE TMyEditText.AddCharacter(ch: CHAR);
//  VAR
//  fi	: FailInfo;
//  PROCEDURE Handle::Failure(error: INTEGER; message: LONGINT);
//  BEGIN
//  IF error = errCantAddChar THEN {if the error is recognized, fill in the param}
//  NumToString(fMaxCharsAllowed, gErrorParm3);
//  {Note: the error will be propagated automatically to exception handlers already
//  posted on the exception handler stack.	Eventually, ErrorAlert will be called to
//  display the message with the text located in the application's message tables.}
//  END;
//  BEGIN
//  CatchFailures(fi, HandleFailure);{Setup an exception handler}
//  IF NumOfChars = fMaxCharsAllowed THEN	{Assume fMaxCharsAllowed = 15, for
//  example}
//  ::Failure(errCantAddChar, msgCantAcceptChar){Can't add a char, so signal failure.}
//  ELSE
//  INHERITED AddCharacter(ch);{Can accept char}
//  Success(fi);{Done with exception handler.}
//  END;
//
// 4) If the character could not be added, then the generic alert will appear on the
// screen with the text:
// Could not add another character, because only 15 are allowed. Delete some characters
// before adding new ones.
//----------------------------------------------------------------------------------------
