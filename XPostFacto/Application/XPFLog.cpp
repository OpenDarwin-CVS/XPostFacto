/*

    Copyright (c) 2001
    Ryan Rempel, ryan.rempel@utoronto.ca 
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

#include "XPFLog.h"

#include "UTEViewStream.h"

#ifdef __MACH__
	#include <syslog.h>
	#include <stdarg.h>
#endif

XPFLog gLogFile ("\p:XPostFacto Log");

#include "CFileStream_AC.cpp"

XPFLog::XPFLog (ConstStr255Param fileName)
:  CCharOutputSink_AC (TH_new CFileStreamRep_AC (&fLogFile)), fLogFile ('TEXT', '????', true)
{
	fViewStream = NULL;
	fActive = false;
	SInt16 logVRefNum;
	SInt32 logDirID;
	
#ifdef __MACH__
	fBuffer[0] = 0;
	openlog ("XPostFacto", 0, LOG_USER);
#else	
	try {
		ThrowIfOSErr_AC (FindFolder (kOnSystemDisk, kPreferencesFolderType, true, &logVRefNum, &logDirID));
		
		ThrowIfOSErr_AC (fLogFile.SpecifyWithTrio (logVRefNum, logDirID, "\p:Unsupported UtilityX Log"));
		if (fLogFile.GetFileSpec().Exists()) fLogFile.RenameFile ("XPostFacto Log");
		
		ThrowIfOSErr_AC (fLogFile.SpecifyWithTrio (logVRefNum, logDirID, fileName));
		
		if (!fLogFile.GetFileSpec().Exists()) ThrowIfOSErr_AC (fLogFile.CreateCFile ());
		fLogFile.SetPermissions (fsRdWrPerm, fsRdWrPerm);
		ThrowIfOSErr_AC (fLogFile.OpenFile ());
		
		long dataLength;
		ThrowIfOSErr_AC (fLogFile.GetDataLength (dataLength));
		if (dataLength > 100 * 1024) {
			ThrowIfOSErr_AC (fLogFile.SetDataLength (0));
			fLogFile.SetPosition (0);
		} else {
			fLogFile.SetPosition (dataLength);
		}
		fActive = true;
	}
	catch (...)
	{
		fActive = false;
	}
#endif
}

XPFLog::~XPFLog ()
{
#ifdef __MACH__
	closelog ();
#endif
}

void 
XPFLog::WriteCharBytes(const char* inStr, long inSize)
{
#ifdef __MACH__
	strncat (fBuffer, inStr, 2047 - strlen (fBuffer));
#endif

	if (fActive) CCharOutputSink_AC::WriteCharBytes (inStr, inSize);
	
	if (fViewStream) {
		fViewStream->WriteCharBytes (inStr, inSize);
		fViewStream->Flush ();
	}
}

#ifdef __MACH__

CStream_AC& 
XPFLog::Flush ()
{
	char *c = fBuffer;
	while (*c) {
		if (*c == 13) *c = 10;
		c++;
	}
	syslog (LOG_INFO, fBuffer);
	fBuffer[0] = 0;
	
	return CCharOutputSink_AC::Flush ();
}

#endif
