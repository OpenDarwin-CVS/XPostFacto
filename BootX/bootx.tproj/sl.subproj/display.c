/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  display.c - Functions to manage and find display.
 *
 *  Copyright (c) 1998-2002 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include <sl.h>

#include "clut.h"
#include "appleboot.h"
#include "failedboot.h"
#include "netboot.h"

struct DisplayInfo {
  CICell screenPH;
  CICell screenIH;
  CICell address;
  CICell width;
  CICell height;
  CICell depth;
  CICell linebytes;
  CICell triedToOpen;
};

typedef struct DisplayInfo DisplayInfo, *DisplayInfoPtr;

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

static long FindDisplays(void);
static long OpenDisplays(void);
static long OpenDisplay(long displayNum);
static long InitDisplay(long displayNum);
static long LookUpCLUTIndex(long index, long depth);
static long LoadDisplayDriver(char *pef, long pefLen);

static long        gNumDisplays;
static long        gMainDisplayNum;
static DisplayInfo gDisplays[16];

static unsigned char *gAppleBoot;
static unsigned char *gNetBoot;
static unsigned char *gFailedBoot;

// Public Functions

long InitDisplays(void)
{
  FindDisplays();
  OpenDisplays();
  
  return 0;
}


long DrawSplashScreen(long stage)
{
  DisplayInfoPtr display;
  short          *appleBoot16, *netBoot16;
  long           *appleBoot32, *netBoot32;
  long           cnt, x, y, pixelSize;
  
  if (gMainDisplayNum == -1) return 0;
  
  display = &gDisplays[gMainDisplayNum];
  
  switch (stage) {
  case 0 :
    // Make sure the boot display is marked.
    SetProp(display->screenPH, "AAPL,boot-display", NULL, 0);
    
    switch (display->depth) {
    case 16 :
      appleBoot16 =
	AllocateBootXMemory(kAppleBootWidth * kAppleBootHeight * 2);
      for (cnt = 0; cnt < (kAppleBootWidth * kAppleBootHeight); cnt++)
	appleBoot16[cnt] = LookUpCLUTIndex(gAppleBootPict[cnt], 16);
      gAppleBoot = (char *)appleBoot16;
      break;
      
    case 32 :
      appleBoot32 =
	AllocateBootXMemory(kAppleBootWidth * kAppleBootHeight * 4);
      for (cnt = 0; cnt < (kAppleBootWidth * kAppleBootHeight); cnt++)
	appleBoot32[cnt] = LookUpCLUTIndex(gAppleBootPict[cnt], 32);
      gAppleBoot = (char *)appleBoot32;
      break;
      
    default :
      gAppleBoot = (unsigned char *)gAppleBootPict;
      break;
    }
    
    x = (display->width - kAppleBootWidth) / 2;
    y = (display->height - kAppleBootHeight) / 2 + kAppleBootOffset;
    
    CallMethod(5, 0, display->screenIH, "draw-rectangle", (long)gAppleBoot,
	       x, y, kAppleBootWidth, kAppleBootHeight);
    
    if (gBootFileType != kNetworkDeviceType) {
      SpinInit(0, 0, NULL, 0, 0, 0, 0, 0, 0, 0);
    } else {
      switch (display->depth) {
      case 16 :
	pixelSize = 2;
	netBoot16 =
	  AllocateBootXMemory(kNetBootWidth * kNetBootHeight * kNetBootFrames * 2);
	for (cnt = 0; cnt < (kNetBootWidth * kNetBootHeight * kNetBootFrames); cnt++)
	  netBoot16[cnt] = LookUpCLUTIndex(gNetBootPict[cnt], 16);
	gNetBoot = (char *)netBoot16;
	break;
	
      case 32 :
	pixelSize = 4;
	netBoot32 =
	  AllocateBootXMemory(kNetBootWidth * kNetBootHeight * kNetBootFrames * 4);
	for (cnt = 0; cnt < (kNetBootWidth * kNetBootHeight * kNetBootFrames); cnt++)
	  netBoot32[cnt] = LookUpCLUTIndex(gNetBootPict[cnt], 32);
	gNetBoot = (char *)netBoot32;
	break;
	
      default :
	pixelSize = 1;
	gNetBoot = (unsigned char *)gNetBootPict;
	break;
      }
      
      x = (display->width - kNetBootWidth) / 2;
      y = (display->height - kNetBootHeight) / 2 + kNetBootOffset;
      
      CallMethod(5, 0, display->screenIH, "draw-rectangle", (long)gNetBoot,
		 x, y, kNetBootWidth, kNetBootHeight);
      
      // Set up the spin cursor.
      SpinInit(display->screenIH, gNetBoot,
	       x, y,
	       kNetBootWidth, kNetBootHeight,
	       kNetBootFrames, kNetBootFPS, pixelSize, 0);
    }
    break;
    
  case 1 :
    x = (display->width - kAppleBootWidth) / 2;
    y = (display->height - kAppleBootHeight) / 2 + kAppleBootOffset;
    
    CallMethod(5, 0, display->screenIH, "draw-rectangle", (long)gAppleBoot,
	       x, y, kAppleBootWidth, kAppleBootHeight);
    
    if (gBootFileType == kNetworkDeviceType) {
      x = (display->width - kNetBootWidth) / 2;
      y = (display->height - kNetBootHeight) / 2 + kNetBootOffset;
      
      // Erase the netboot picture with 75% grey.
      CallMethod(5, 0, display->screenIH, "fill-rectangle",
		 LookUpCLUTIndex(0x01, display->depth),
		 x, y, kNetBootWidth, kNetBootHeight);
    }
    break;
    
  default :
    return -1;
    break;
  }
  
  return 0;
}


long DrawFailedBootPicture(void)
{
  long           cnt;
  short          *failedBoot16;
  long           *failedBoot32, posX, posY;
  DisplayInfoPtr display = &gDisplays[gMainDisplayNum];
  
  switch (display->depth) {
  case 16 :
    failedBoot16 = AllocateBootXMemory(32 * 32 * 2);
    for (cnt = 0; cnt < (32 * 32); cnt++)
      failedBoot16[cnt] = LookUpCLUTIndex(gFailedBootPict[cnt], 16);
    gFailedBoot = (char *)failedBoot16;
    break;
    
  case 32 :
    failedBoot32 = AllocateBootXMemory(32 * 32 * 4);
    for (cnt = 0; cnt < (32 * 32); cnt++)
      failedBoot32[cnt] = LookUpCLUTIndex(gFailedBootPict[cnt], 32);
    gFailedBoot = (char *)failedBoot32;
    break;
    
  default :
    gFailedBoot = (unsigned char *)gFailedBootPict;
    break;
  }
  
  // Erase the newboot picture with 75% grey.
  posX = (display->width - kNetBootWidth) / 2;
  posY = (display->height - kNetBootHeight) / 2 + kNetBootOffset;
  CallMethod(5, 0, display->screenIH, "fill-rectangle",
	     LookUpCLUTIndex(0x01, display->depth),
	     posX, posY, kNetBootWidth, kNetBootHeight);
  
  // Draw the failed boot picture.
  posX = (display->width - kFailedBootWidth) / 2;
  posY = ((display->height - kFailedBootHeight)) / 2 + kFailedBootOffset;
  CallMethod(5, 0, display->screenIH, "draw-rectangle",
	     (long)gFailedBoot, posX, posY,
	     kFailedBootWidth, kFailedBootHeight);
  
  return 0;
}


void GetMainScreenPH(Boot_Video_Ptr video)
{
  DisplayInfoPtr display;
  long           address, size;
  
  if (gMainDisplayNum == -1) {
    // No display, set it to zero.
    video->v_baseAddr = 0;
    video->v_rowBytes = 0;
    video->v_width = 0;
    video->v_height = 0;
    video->v_depth = 0;
  } else {
    display = &gDisplays[gMainDisplayNum];
    
    video->v_baseAddr = display->address;
    video->v_rowBytes = display->linebytes;
    video->v_width = display->width;
    video->v_height = display->height;
    video->v_depth = display->depth;
  }
  
  // Allocate memory and a range for the CLUT.
  size = 256 * 3;
  address = AllocateKernelMemory(size);
  AllocateMemoryRange("BootCLUT", address, size);
  bcopy((char *)gClut, (char *)address, size);
  
  // Allocate memory and a range for the failed boot picture.
  size = 32 + kFailedBootWidth * kFailedBootHeight;
  address = AllocateKernelMemory(size);
  AllocateMemoryRange("Pict-FailedBoot", address, size);
  ((long *)address)[0] = kFailedBootWidth;
  ((long *)address)[1] = kFailedBootHeight;
  ((long *)address)[2] = kFailedBootOffset;
  bcopy((char *)gFailedBootPict, (char *)(address + 32), size - 32);
}


// Private Functions

static long FindDisplays(void)
{
  CICell screenPH, controlPH;
  long   cnt, size;
  char prop[128];
  
  // Find all the screens in the system.
  screenPH = 0;
  while (1) {
    screenPH = SearchForNode(screenPH, 1, "device_type", "display");
    if (screenPH != 0) gDisplays[gNumDisplays++].screenPH = screenPH;
    else break;
  }
  
  // Find /chaos/control, and
  // invalidate gStdOutPH if equal (since new OF was downloaded).
  controlPH = FindDevice("/chaos/control");
  if (gStdOutPH == controlPH) gStdOutPH = 0;
  
  // Find the main screen using the chaos/control, the output-device, or the screen alias.
  // If there is a /chaos/control, it looks like we have to use it. Otherwise, we end up
  // with a kernel panic when the NDRV loads later. I think it is because IONDRVFramebuffer
  // does some special memory mapping for the boot-display. Basically, it seems that if
  // /chaos/control is around, we need to use it as the boot-display. However, it's not so
  // bad, as if there is no monitor attached, the Open will fail below, and we'll actually
  // get another screen anyway.
  // So the changes here are mostly for the sake of the Beige G3, so that something other than
  // the screen alias can be the boot-display.
  
  gMainDisplayNum = -1;
  
  screenPH = controlPH;
  if (screenPH == -1) {
	size = GetProp (gOptionsPH, "output-device", prop, 127);
	if (size == -1) prop[0] = '\0'; else prop[size] = '\0';
	screenPH = FindDevice (prop);
	if (screenPH != -1) {
		size = GetProp (screenPH, "device_type", prop, 127);
		if (size == -1) prop[0] = '\0'; else prop[size] = '\0';
		if (strcmp (prop, "display")) screenPH = -1;
	}
	
	if (screenPH == -1) screenPH = FindDevice("screen");
  }
  
  for (cnt = 0; cnt < gNumDisplays; cnt++)
    if (gDisplays[cnt].screenPH == screenPH) gMainDisplayNum = cnt;
  
  return 0;
}


static long OpenDisplays(void)
{
  long cnt;
  
  // Open the main screen or
  // look for a main screen if we don't have one.
  if ((gMainDisplayNum == -1) || !OpenDisplay(gMainDisplayNum)) {
    gMainDisplayNum = -1;
    for (cnt = 0; cnt < gNumDisplays; cnt++) {
      if (OpenDisplay(cnt)) {
	gMainDisplayNum = cnt;
	break;
      }
    }
  }
  
  // Open the rest of the displays
  if (gOFVersion >= kOFVersion3x) {
    for (cnt = 0; cnt < gNumDisplays; cnt++) {
      OpenDisplay(cnt);
    }
  }  
  return 0;
}


static long OpenDisplay(long displayNum)
{
  char   screenPath[258], displayType[32];
  CICell screenPH, screenIH;
  long   ret, size;
  
  // Only try to open a screen once.
  if (gDisplays[displayNum].triedToOpen) {
    return gDisplays[displayNum].screenIH != 0;
  } else {
    gDisplays[displayNum].triedToOpen = -1;
  }
  
  screenPH = gDisplays[displayNum].screenPH;
  
  // Try to use mac-boot's ihandle.
  Interpret(0, 1, "\" _screen-ihandle\" $find if execute else 0 then",
	    &screenIH);
  if ((screenIH != 0) && (InstanceToPackage(screenIH) != screenPH)) {
    screenIH = 0;
  }
  
  // Try to use stdout as the screen's ihandle
  if ((screenIH == 0) && (gStdOutPH == screenPH)) {
    screenIH = gStdOutIH;
  }
  
  // Try to open the display.
  if (screenIH == 0) {
    screenPath[255] = '\0';
    ret = PackageToPath(screenPH, screenPath, 255);
    if (ret != -1) {
      strcat(screenPath, ":0");
      screenIH = Open(screenPath);
	  
	  // Work around problem with Radeon 7000 on Old World machines
	  // Adapted from code by Joe van Tunen
	  if (screenIH == 0) {
	    if (strcmp (&screenPath[ret - 4], "@0,0:0" ) == 0) {
		  screenPath[ret - 4] = '\0';
		  strcat (screenPath, ":0");
		  screenIH = Open (screenPath);
        }
	  }
    }
  }
  
  // Find out what type of display is attached.
  size = GetProp(screenPH, "display-type", displayType, 31);
  if (size != -1) {
    displayType[size] = '\0';
    // If the display-type is NONE, don't use the display.
    if (!strcmp(displayType, "NONE")) screenIH = 0;
  }
  
  // Save the ihandle for later use.
  gDisplays[displayNum].screenIH = screenIH;
  
  // Initialize the display.
  if (screenIH != 0) InitDisplay(displayNum);
  
  return screenIH != 0;
}


static long InitDisplay(long displayNum)
{
  DisplayInfoPtr display = &gDisplays[displayNum];
  CICell         screenPH = display->screenPH;
  CICell         screenIH = display->screenIH;
  
  // Get the vital info for this screen.
  GetProp(screenPH, "address", (char *)&(display->address), 4);
  GetProp(screenPH, "width", (char *)&(display->width), 4);
  GetProp(screenPH, "height", (char *)&(display->height), 4);
  GetProp(screenPH, "depth", (char *)&(display->depth), 4);
  GetProp(screenPH, "linebytes", (char *)&(display->linebytes), 4);
  
  // Replace some of the drivers words.
  Interpret(3, 1,
   " value depthbytes"
   " value rowbytes"
   " to active-package"
   " frame-buffer-adr value this-frame-buffer-adr"
   
   " : rect-setup"      // ( adr|index x y w h -- w adr|index xy-adr h )
   "   >r >r rowbytes * swap depthbytes * + this-frame-buffer-adr +"
   "   r> depthbytes * -rot r>"
   " ;"
   
   " : DRAW-RECTANGLE"                          // ( adr x y w h -- )
   "   rect-setup"                              // ( w adr xy-adr h )
   "   0 ?do"                                   // ( w adr xy-adr )
   "     2dup 4 pick move"
   "     2 pick rowbytes d+"
   "   loop"
   "   3drop"
   " ;"
   
   " : FILL-RECTANGLE"                           // ( index x y w h -- )
   "   rect-setup rot depthbytes case"
   "     1 of dup 8 << or dup 10 << or endof"
   "     2 of dup 10 << or endof"
   "   endcase -rot 0 ?do"
   "     dup 3 pick 3 pick filll"
   "     rowbytes +"
   "   loop"
   "   3drop"
   " ;"
   
   " : READ-RECTANGLE"                            // ( adr x y w h -- )
   "   rect-setup  >r swap r> 0 ?do"
   "     2dup 4 pick move"
   "     rowbytes 3 pick d+"
   "   loop"
   "   3drop"
   " ;"
   
   " this-frame-buffer-adr"
   " 0 to active-package"
   , display->screenPH, display->linebytes,
   display->depth / 8, &display->address);
  
  // Set the CLUT for 8 bit displays.
  if (display->depth == 8) {
    CallMethod(3, 0, screenIH, "set-colors", (long)gClut, 0, 256);
  }
  
  // Set the screen to 75% grey.
  CallMethod(5, 0, screenIH, "fill-rectangle",
	     LookUpCLUTIndex(0x01, display->depth),
	     0, 0, display->width, display->height);
  
  return 0;
}


static long LookUpCLUTIndex(long index, long depth)
{
  long result, red, green, blue;
  
  red   = gClut[index * 3 + 0];
  green = gClut[index * 3 + 1];
  blue  = gClut[index * 3 + 2];
  
  switch (depth) {
  case 16 :
    result = ((red & 0xF8) << 7)|((green & 0xF8) << 2)|((blue & 0xF8) >> 3);
    break;
    
  case 32 :
    result = (red << 16) | (green << 8) | blue;
    break;
    
  default :
    result = index;
    break;
  }
  
  return result;
}


long LoadDisplayDrivers (char *rootDir)
{  
	char *currentContainer, *fileEnd;
	long ret, flags, index, time;
	long fileLen;
	char dirSpec[512], *name;
	index = 0;
	
	printf ("Loading display drivers from %s\n", rootDir);
	
	while (1) {
		strcpy (dirSpec, rootDir);
		
		ret = GetDirEntry (dirSpec, &index, &name, &flags, &time);
		if (ret == -1) break;
		
		if ((flags & kFileTypeMask) != kFileTypeFlat) continue;
		
		strcat (dirSpec, name);
		
		printf ("Loading driver from %s\n", dirSpec);
		
		fileLen = LoadFile (dirSpec);
		if ((fileLen == 0) || (fileLen == -1)) {
			printf ("Error loading file: %ld\n", fileLen);
			continue;
		}
		
		currentContainer = (char *) kLoadAddr;
		fileEnd = currentContainer + fileLen;
		
		while (currentContainer < fileEnd) {
			unsigned *nextContainer = (unsigned *) currentContainer;
			while (++nextContainer < (unsigned *) fileEnd) if ((nextContainer[0] == kMagic1) && (nextContainer[1] == kMagic2) && (nextContainer[2] == kMagic3)) break;

			ret = LoadDisplayDriver (currentContainer, ((unsigned) nextContainer) - ((unsigned) currentContainer));
			if (ret == -1) printf ("Error loading %s\n", dirSpec);
			
			currentContainer = (char *) nextContainer;
		}
	}
	
	return 0;
}


static long LoadDisplayDriver (char *pef, long pefLen)
{
	char              *currentPef, *buffer;
	long              currentPefLen;
	long              curDisplay;
	char              descripName[] = " TheDriverDescription";
	long              err;
	DriverDescription descrip;
	DriverDescription curDesc;
	char              matchName[40];
	unsigned long     newVersion;
	unsigned long     curVersion;
	CICell            screenPH;
  
	descripName[0] = strlen (descripName + 1);
	err = GetSymbolFromPEF (descripName, pef, &descrip, sizeof(descrip));
	if (err != 0) {
		printf ("\nGetSymbolFromPEF returns %ld\n", err);
		return -1;
	}
	
	if ((descrip.driverDescSignature != kTheDescriptionSignature) ||
			(descrip.driverDescVersion != kInitialDriverDescriptor)) {
		printf ("Incorrect signature or version\n");
		return -1;
	}
  
	strncpy (matchName, descrip.driverType.nameInfoStr + 1, descrip.driverType.nameInfoStr[0]);
	newVersion = descrip.driverType.version;
	if ((newVersion & 0xffff) == 0x8000) newVersion |= 0xff; // final stage, release rev  

	printf ("Finding display for %s\n", matchName);
  
	for (curDisplay = 0; curDisplay < gNumDisplays; curDisplay++) {
		screenPH = gDisplays[curDisplay].screenPH;
		if (MatchThis (screenPH, matchName) != 0) continue;
    
		err = GetPackageProperty (screenPH, "driver,AAPL,MacOS,PowerPC", &currentPef, &currentPefLen);
    
		if (err == 0) {
			err = GetSymbolFromPEF(descripName,currentPef,&curDesc,sizeof(curDesc));
			if (err == 0) {
				if ((curDesc.driverDescSignature == kTheDescriptionSignature) &&
						(curDesc.driverDescVersion == kInitialDriverDescriptor)) {
					curVersion = curDesc.driverType.version;
					if ((curVersion & 0xffff) == 0x8000) curVersion |= 0xff; // final stage, release rev
					if (newVersion <= curVersion) {
						printf ("Newer version already installed\n");
						pefLen = 0;
					}
				}
			}
		}
    
		if (pefLen == 0) continue;
    
		printf ("Installing patch driver\n");

		buffer = (char *) AllocateBootXMemory (pefLen);
		if (buffer == NULL) {
			printf ("No space for the NDRV\n");
			return -1;
		}
		bcopy (pef, buffer, pefLen);
    
		SetProp (screenPH, "driver,AAPL,MacOS,PowerPC", buffer, pefLen);
	}
	
	return 0;
}


#if 0
static void DumpDisplaysInfo(void);

static void DumpDisplaysInfo(void)
{
  long cnt, length;
  char tmpStr[512];
  
  printf("gNumDisplays: %x, gMainDisplayNum: %x\n",
	 gNumDisplays, gMainDisplayNum);
  
  for (cnt = 0; cnt < gNumDisplays; cnt++) {
    printf("Display: %x, screenPH: %x,  screenIH: %x\n",
	   cnt, gDisplays[cnt].screenPH, gDisplays[cnt].screenIH);
    
    if (gDisplays[cnt].screenPH) {
      length = PackageToPath(gDisplays[cnt].screenPH, tmpStr, 511);
      tmpStr[length] = '\0';
      printf("PHandle Path: %s\n", tmpStr);
    }
    
    if (gDisplays[cnt].screenIH) {
      length = InstanceToPath(gDisplays[cnt].screenIH, tmpStr, 511);
      tmpStr[length] = '\0';
      printf("IHandle Path: %s\n", tmpStr);
    }
    
    printf("address = %x\n", gDisplays[cnt].address);
    printf("linebytes = %x\n", gDisplays[cnt].linebytes);
    printf("width = %x\n", gDisplays[cnt].width);
    printf("height = %x\n", gDisplays[cnt].height);
    printf("depth = %x\n", gDisplays[cnt].depth);
    printf("\n");
  }
}
#endif
