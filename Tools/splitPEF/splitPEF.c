/*

    Copyright (c) 2003
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

/*

Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved. 

This file contains Original Code and/or Modifications of Original Code as defined 
in and that are subject to the Apple Public Source License Version 2.0 (the 'License').  
You may not use this file except in compliance with the License.  Please obtain a copy 
of the License at http://www.opensource.apple.com/apsl/ and read it before using this file. 

The Original Code and all software distributed under the License are distributed on an 
'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND APPLE HEREBY 
DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see the License 
for the specific language governing rights and limitations under the License.

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "vers_rsrc.h"
#include "GetSymbolFromPEF.h"

// The Driver Description
enum {
  kInitialDriverDescriptor	= 0,
  kVersionOneDriverDescriptor	= 1,
  kTheDescriptionSignature	= 'mtej',
  kDriverDescriptionSignature	= 'pdes'
};

struct DriverType {
  unsigned char nameInfoStr[32]; // Driver Name/Info String
  unsigned long	version;         // Driver Version Number - really NumVersion
};
typedef struct DriverType DriverType;

struct DriverDescription {
  unsigned long driverDescSignature; // Signature field of this structure
  unsigned long driverDescVersion;   // Version of this data structure
  DriverType    driverType;          // Type of Driver
  char          otherStuff[512];
};
typedef struct DriverDescription DriverDescription;

#define	kMagic1		'Joy!'
#define	kMagic2		'peff'
#define kMagic3		'pwpc'

// The code in splitPEF will look a little strange, because I'm copying from the code in BootX
// with as few changes as possible -- the idea is to help test the logic, which is harder to
// do in BootX itself

char *gLoadAddr = NULL;
char gOutputDirectory[1024];
int gFile = 0;

long
LoadFile (const char *file)
{
	long len;
	struct stat sb;
	int err;
	
	err = stat (file, &sb);
	if (err) return 0;
	if ((sb.st_mode & S_IFMT) != S_IFREG) return 0;
	len = sb.st_size;	
	
	sprintf (gOutputDirectory, "%s.splitPEF", file);
	err = stat (gOutputDirectory, &sb);
	if (err) {
		err = mkdir (gOutputDirectory, 0775);
		if (err) return 0;
	} else {
		if ((sb.st_mode & S_IFMT) != S_IFDIR) return 0;
	}
	
	gFile = open (file, O_RDONLY, NULL);
	if (gFile <= 0) return 0;
	
	gLoadAddr = mmap (NULL, len, PROT_READ, NULL, gFile, NULL);
	if (!gLoadAddr) {
		close (gFile);
		gFile = 0;
		return 0;
	}
	
	return len;
}

void
LoadDisplayDriver (char *buffer, long len)
{
	char              descripName[] = " TheDriverDescription";
	long              err;
	DriverDescription descrip;
	char              fileName[128];
  
	descripName[0] = strlen (descripName + 1);
	err = GetSymbolFromPEF (descripName, (LogicalAddress) buffer, (LogicalAddress) &descrip, sizeof(descrip));
	if (err != 0) {
		printf ("\nGetSymbolFromPEF returns %ld\n", err);
		return;
	}
	
	if ((descrip.driverDescSignature != kTheDescriptionSignature) ||
		(descrip.driverDescVersion != kInitialDriverDescriptor)
	) return;
  
	strcpy (fileName, gOutputDirectory);
	strcat (fileName, "/");
	strncat (fileName, descrip.driverType.nameInfoStr + 1, descrip.driverType.nameInfoStr[0]);
	strcat (fileName, " ");
	VERS_string (fileName + strlen (fileName), 128 - strlen (fileName), descrip.driverType.version);

	int outFile = open (fileName, O_WRONLY | O_CREAT | O_TRUNC, 0775);
	if (outFile <= 0) return;
	
	write (outFile, buffer, len);
	
	close (outFile);
}

void 
splitPEF (const char *file)
{
	char *currentContainer, *fileEnd;
	long fileLen;
	
	fileLen = LoadFile (file);
	if (fileLen == -1) return;
	if (fileLen == 0) return;
			
	currentContainer = (char *) gLoadAddr;
	fileEnd = currentContainer + fileLen;
		
	while (currentContainer < fileEnd) {
		unsigned *nextContainer = (unsigned *) currentContainer;
		while (++nextContainer < (unsigned *) fileEnd) if ((nextContainer[0] == kMagic1) && (nextContainer[1] == kMagic2) && (nextContainer[2] == kMagic3)) 
				break;

		LoadDisplayDriver (currentContainer, ((unsigned) nextContainer) - ((unsigned) currentContainer));
			
		currentContainer = (char *) nextContainer;
	}
	
	munmap (gLoadAddr, fileLen);
	gLoadAddr = NULL;

	close (gFile);	
	gFile = 0;
}

int 
main (int argc, const char * argv[]) 
{
	int x;
	for (x = 1; x < argc; x++) splitPEF (argv[x]);
	return 0;
}