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
 *  main.c - Main functions for BootX.
 *
 *  Copyright (c) 1998-2003 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include "XPFCPUSettings.h"

#include <sl.h>

static void Start(void *unused1, void *unused2, ClientInterfacePtr ciPtr);
static void Main(ClientInterfacePtr ciPtr);
static long InitEverything(ClientInterfacePtr ciPtr);
static long DecodeKernel(void);
static long SetUpBootArgs(void);
static long CallKernel(void);
static void FailToBoot(char *reason);
static long InitMemoryMap(void);
static long GetOFVersion(void);
static long TestForKey(long key);
static long GetBootPaths(void);
static void PatchKernelFor60xCPU(void);

const unsigned long StartTVector[2] = {(unsigned long)Start, 0};

char gStackBaseAddr[0x8000];

char *gVectorSaveAddr;
long gImageLastKernelAddr = 0;
long gImageFirstBootXAddr = kLoadAddr;
long gKernelEntryPoint;
long gDeviceTreeAddr;
long gDeviceTreeSize;
long gBootArgsAddr;
long gBootArgsSize;
long gSymbolTableAddr;
long gSymbolTableSize;

long gBootSourceNumber = -1;
long gBootSourceNumberMax;
long gBootMode = kBootModeNormal;
long gBootDeviceType;
long gBootFileType;
char gHaveKernelCache = 0;
char gUseXPFInstallRequired = 0;
char gBootDevice[256] = {0};
char gBootFile[256] = {0};
char gApparentBootFile[256] = {0};
static char gBootKernelCacheFile[512];
static char gExtensionsSpec[4096];
static char gCacheNameAdler[64 + sizeof(gBootFile)];
static char *gPlatformName = gCacheNameAdler;

char gTempStr[4096] = {0};

// make this a global, since used by SetProp
// pointed out by Joe van Tunen
char ofBootArgs[128] = {0};

long *gDeviceTreeMMTmp = 0;

long gOFVersion;

char *gKeyMap;

long gRootAddrCells;
long gRootSizeCells;

CICell gChosenPH;
CICell gOptionsPH;
CICell gScreenPH;
CICell gMemoryMapPH;
CICell gStdOutPH;

CICell gMMUIH;
CICell gMemoryIH;
CICell gStdOutIH;
CICell gKeyboardIH;

static char gOFVectorSave[kVectorSize];
static unsigned long gOFMSRSave;
static unsigned long gOFSPRG0Save;
static unsigned long gOFSPRG1Save;
static unsigned long gOFSPRG2Save;
static unsigned long gOFSPRG3Save;

// Private Functions

static void Start(void *unused1, void *unused2, ClientInterfacePtr ciPtr)
{
  long newSP;
  
  // Move the Stack to a chunk of the BSS
  newSP = (long)gStackBaseAddr + sizeof(gStackBaseAddr) - 0x100;
  __asm__ volatile("mr r1, %0" : : "r" (newSP));
  
  Main(ciPtr);
}


static void Main(ClientInterfacePtr ciPtr)
{
	long ret;
	int  trycache;
	long flags, cachetime, time;

	ret = InitEverything(ciPtr);
	if (ret != 0) FailToBoot ("BootX::InitEverything () failed");

	// Get or infer the boot paths.
	ret = GetBootPaths();
	if (ret != 0) FailToBoot ("BootX::GetBootPaths () failed");

	DrawSplashScreen(0);

	while (ret == 0) {
	trycache = (0 == (gBootMode & kBootModeSafe)) && (gBootKernelCacheFile[0] != 0);

	if (trycache && (gBootFileType == kBlockDeviceType)) do {
	  
		// if we haven't found the kernel yet, don't use the cache
		ret = GetFileInfo(NULL, gBootFile, &flags, &time);
		if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)) {
			trycache = 0;
			break;
		}
		ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
		if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat) || (cachetime < time)) {
			trycache = 0;
			break;
		}
		ret = GetFileInfo(gExtensionsSpec, "Extensions", &flags, &time);
		if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory) && (cachetime < time)) {
			trycache = 0;
			break;
		}
	} while (0);

	if (trycache) {
		ret = LoadFile(gBootKernelCacheFile);
		if (ret != -1) {
			ret = DecodeKernel();
			if (ret != -1) break;
		}
	}
	ret = LoadFile(gBootFile);
	if (ret != -1)
		ret = DecodeKernel();
		if (ret != -1) break;

		ret = GetBootPaths();
		if (ret != 0) FailToBoot ("BootX::GetBootPaths () failed following LoadFile");
	}

	if (ret != 0) FailToBoot("BootX::DecodeKernel () failed");

	if (!gHaveKernelCache) {
		ret = LoadDrivers(gExtensionsSpec);
		if (ret != 0) FailToBoot("BootX::LoadDrivers () failed");
	}

	DrawSplashScreen(1);

	// added by ryan.rempel@utoronto.ca
	XPFfixProcessorSettings ();

	ret = SetUpBootArgs();
	if (ret != 0) FailToBoot ("BootX::SetUpBootArgs () failed");

	ret = CallKernel();

	FailToBoot ("BootX::CallKernel () returned unexpectedly");
}


static long InitEverything(ClientInterfacePtr ciPtr)
{
  long   ret, mem_base, mem_base2, size;
  CICell keyboardPH;
  char   name[32], securityMode[33];
  long length;
  char *compatible;
  
  // Init the OF Client Interface.
  ret = InitCI(ciPtr);
  if (ret != 0) return -1;
  
  // Get the OF Version
  gOFVersion = GetOFVersion();
  if (gOFVersion == 0) return -1;
  
  // Get the address and size cells for the root.
  GetProp(Peer(0), "#address-cells", (char *)&gRootAddrCells, 4);
  GetProp(Peer(0), "#size-cells", (char *)&gRootSizeCells, 4);
  if ((gRootAddrCells > 2) || (gRootAddrCells > 2)) return -1;
  
  // Init the SL Words package.
  ret = InitSLWords();
  if (ret != 0) return -1;
  
  // Get the phandle for /options
  gOptionsPH = FindDevice("/options");
  if (gOptionsPH == -1) return -1;
  
  // Get the phandle for /chosen
  gChosenPH = FindDevice("/chosen");
  if (gChosenPH == -1) return -1;
  
  // Init the Memory Map.
  ret = InitMemoryMap();
  if (ret != 0) return -1;
  
  // Get IHandles for the MMU and Memory
  size = GetProp(gChosenPH, "mmu", (char *)&gMMUIH, 4);
  if (size != 4) {
    printf("Failed to get the IH for the MMU.\n");
    return -1;
  }
  size = GetProp(gChosenPH, "memory", (char *)&gMemoryIH, 4);
  if (size != 4) {
    printf("Failed to get the IH for the Memory.\n");
    return -1;
  }
  
  // Get first element of root's compatible property.
  ret = GetPackageProperty(Peer(0), "compatible", &compatible, &length);
  if (ret != -1)
    strcpy(gPlatformName, compatible);
  
  // Get stdout's IH, so that the boot display can be found.
  ret = GetProp(gChosenPH, "stdout", (char *)&gStdOutIH, 4);
  if (ret == 4) gStdOutPH = InstanceToPackage(gStdOutIH);
  else gStdOutPH = gStdOutIH = 0;
  
  // Try to find the keyboard using chosen
  ret = GetProp(gChosenPH, "stdin", (char *)&gKeyboardIH, 4);
  if (ret != 4) gKeyboardIH = 0;
  else {
    keyboardPH = InstanceToPackage(gKeyboardIH);
    ret = GetProp(keyboardPH, "name", name, 31);
    if (ret != -1) {
      name[ret] = '\0';
      if (strcmp(name, "keyboard") && strcmp(name, "kbd")) gKeyboardIH = 0;
    } else gKeyboardIH = 0;
  }
  
  // Try to the find the keyboard using open if chosen did not work.
  if (gKeyboardIH == 0) gKeyboardIH = Open("keyboard");
  if (gKeyboardIH == 0) gKeyboardIH = Open("kbd");
  
  // Get the key map set up, and make it up to date.
  gKeyMap = InitKeyMap(gKeyboardIH);
  if (gKeyMap == NULL) return -1;
  UpdateKeyMap();
  
  // Test for Secure Boot Mode.
  size = GetProp(gOptionsPH, "security-mode", securityMode, 32);
  if (size != -1) {
    securityMode[size] = '\0';
    if (strcmp(securityMode, "none")) gBootMode |= kBootModeSecure;
  }
  
#if kFailToBoot
  // 'cmd-s' or 'cmd-v' is pressed set outputLevel to kOutputLevelFull
  if (((gBootMode & kBootModeSecure) == 0) && TestForKey(kCommandKey) &&
      (TestForKey('s') || TestForKey('v'))) {
    SetOutputLevel(kOutputLevelFull);
  } else {
    SetOutputLevel(kOutputLevelDisabled);
  }
#else
  SetOutputLevel(kOutputLevelFull);
#endif
  
  // printf now works.
  printf("\n\nMac OS X Loader\n");
  printf("Custom BootX %s, based on BootX %s\n", kBootXVersionXPF, kBootXVersion);

  // Added by ryan.rempel@utoronto.ca
  // On Kanga, we sometimes get MMU errors that are cured by a reset-all
  // So, we'll added the ability to do a reset-all by holding down command-r
  if (TestForKey (kCommandKey) && TestForKey ('r')) {
	  Interpret (0, 0, "reset-all");
  }
  
  // Test for Safe Boot Mode.
  if (((gBootMode & kBootModeSecure) == 0) && TestForKey(kShiftKey)) {
    gBootMode |= kBootModeSafe;
  }
  
  // Claim memory for the FS Cache.
  if (Claim(kFSCacheAddr, kFSCacheSize, 0) == 0) {
    printf("Claim for fs cache failed.\n");
    return -1;
  }
  
  // Claim memory for malloc.
  if (Claim(kMallocAddr, kMallocSize, 0) == 0) {
    printf("Claim for malloc failed.\n");
    return -1;
  }
  malloc_init((char *)kMallocAddr, kMallocSize);
  
  // Claim memory for the Load Addr.
  mem_base = Claim(kLoadAddr, kLoadSize, 0);
  if (mem_base == 0) {
    printf("Claim for Load Area failed.\n");
    return -1;
  }
  
  // Claim the memory for the Image Addr
  if (gOFVersion >= kOFVersion3x) {
    mem_base = Claim(kImageAddr, kImageSize, 0);
    if (mem_base == 0) {
      printf("Claim for Image Area failed.\n");
      return -1;
    }
  } else {
    // Claim the 1:1 mapped chunks first.
    mem_base  = Claim(kImageAddr0, kImageSize0, 0);
    mem_base2 = Claim(kImageAddr2, kImageSize2, 0);
    if ((mem_base == 0) || (mem_base2 == 0)) {
      printf("Claim for Image Area failed.\n");
      return -1;
    }
    
    // Unmap the old xcoff stack.
    CallMethod(2, 0, gMMUIH, "unmap", 0x00380000, 0x00080000);
    
    // Grap the physical memory then the logical.
    CallMethod(3, 1, gMemoryIH, "claim",
	       kImageAddr1Phys, kImageSize1, 0, &mem_base);
    CallMethod(3, 1, gMMUIH, "claim",
	       kImageAddr1, kImageSize1, 0, &mem_base2);
    if ((mem_base == 0) || (mem_base2 == 0)) {
      printf("Claim for Image Area failed.\n");
      return -1;
    }
    
    // Map them together.
    CallMethod(4, 0, gMMUIH, "map",
	       kImageAddr1Phys, kImageAddr1, kImageSize1, 0);
  }
  
  bzero((char *)kImageAddr, kImageSize);
  
  // Allocate some space for the Vector Save area.
  gVectorSaveAddr = AllocateBootXMemory(kVectorSize);
  if (gVectorSaveAddr == 0) {
    printf("Allocation for the Vector Save Area failed.\n");
    return -1;
  }
  
  // Find all the displays and set them up.
  ret = InitDisplays();
  if (ret != 0) {
    printf("InitDisplays failed.\n");
    return -1;
  }
  
  return 0;
}


long ThinFatBinary(void **binary, unsigned long *length)
{
  long ret;
  
  ret = ThinFatBinaryMachO(binary, length);
  if (ret == -1) ret = ThinFatBinaryElf(binary, length);
  
  return ret;
}

static long DecodeKernel(void)
{
  void *binary = (void *)kLoadAddr;
  long ret;
  compressed_kernel_header * kernel_header = (compressed_kernel_header *) kLoadAddr;
  u_int32_t size;
  
  if (kernel_header->signature == 'comp') {
    if (kernel_header->compress_type != 'lzss')
      return -1;
    if (kernel_header->platform_name[0] && strcmp(gPlatformName, kernel_header->platform_name))
      return -1;
    if (kernel_header->root_path[0] && strcmp(gBootFile, kernel_header->root_path))
      return -1;
    
    binary = AllocateBootXMemory(kernel_header->uncompressed_size);
    
    size = decompress_lzss((u_int8_t *) binary, &kernel_header->data[0], kernel_header->compressed_size);
    if (kernel_header->uncompressed_size != size) {
      printf("size mismatch from lzss %x\n", size);
      return -1;
    }
    if (kernel_header->adler32 !=
	Alder32(binary, kernel_header->uncompressed_size)) {
      printf("adler mismatch\n");
      return -1;
    }
  }
  
  ThinFatBinary(&binary, 0);
  
  ret = DecodeMachO(binary);
  if (ret == -1) ret = DecodeElf(binary);
  
  PatchKernelFor60xCPU ();
  
  return ret;
}

// This patches the problem in the 10.2.x kernel that causes trouble for the 603 and 604 CPU
// Peter Caday diagnosed the issue
// See http://www.opendarwin.org/bugzilla/show_bug.cgi?id=854
// ryan.rempel@utoronto.ca

#define PROCESSOR_VERSION_604ev		10
#define PROCESSOR_VERSION_750		8

static void PatchKernelFor60xCPU(void)
{
	unsigned long pvr;
	unsigned long *p = (unsigned long *) gVectorSaveAddr;
	unsigned long count = kVectorSize / sizeof (*p);
	
	__asm__ volatile ("mfpvr %0" : "=r" (pvr));
	pvr >>= 16;
	if ((pvr > PROCESSOR_VERSION_604ev) || (pvr == PROCESSOR_VERSION_750)) return;
	
	while (count--) {
		if (
			(p[0] == 0x7f660120) &&		// mtcrf 0x60,r27 in osfmk/ppc/lowmen_vectors.s
			(p[1] == 0x56a00421) &&		// rlwinm. r0,r21,0,MSR_EE_BIT,MSR_EE_BIT
			(p[2] == 0x4fe74102) &&		// crandc 31,pfThermalb,pfThermIntb
			(p[3] == 0x3abf0080) &&		// la r21,saver0(r31)
			(p[4] == 0x4fe2f902)		// crandc 31,cr0_eq,31
		) {
			p[4] = 0x4fff1102;			// crandc 31,31,cr0_eq
			printf ("==> Patched kernel for 60x CPU\n");
			break;
		}
		p++;
	}
}

static long SetUpBootArgs(void)
{
  boot_args_ptr      args;
  CICell             memoryPH;
  long               graphicsBoot = 1;
  long               ret, cnt, size, dash;
  long               sKey, vKey, keyPos;
  char               *ofArgs, tc, keyStr[8];
  unsigned char      mem_regs[kMaxDRAMBanks*16];
  unsigned long      mem_banks, bank_shift;
  
  // Save file system cache statistics.
  SetProp(gChosenPH, "BootXCacheHits", (char *)&gCacheHits, 4);
  SetProp(gChosenPH, "BootXCacheMisses", (char *)&gCacheMisses, 4);
  SetProp(gChosenPH, "BootXCacheEvicts", (char *)&gCacheEvicts, 4);
  
  // Allocate some memory for the BootArgs.
  gBootArgsSize = sizeof(boot_args);
  gBootArgsAddr = AllocateKernelMemory(gBootArgsSize);
  
  // Add the BootArgs to the memory-map.
  AllocateMemoryRange("BootArgs", gBootArgsAddr, gBootArgsSize);
  
  args = (boot_args_ptr)gBootArgsAddr;
  
  args->Revision = kBootArgsRevision;
  args->Version = kBootArgsVersion1;
  args->machineType = 0;
  
  // Check the Keyboard for 'cmd-s' and 'cmd-v'
  UpdateKeyMap();
  if ((gBootMode & kBootModeSecure) == 0) {
    sKey = TestForKey(kCommandKey) && TestForKey('s');
    vKey = TestForKey(kCommandKey) && TestForKey('v');
  } else {
    sKey = 0;
    vKey = 0;
  }
  
  // if 'cmd-s' or 'cmd-v' was pressed do a text boot.
  if (sKey || vKey) graphicsBoot = 0;
  
	// Create the command line.
	if (gOFVersion < kOFVersion3x) {
		ofBootArgs[0] = ' ';
		size = GetProp(gChosenPH, "machargs", ofBootArgs + 1, 126);
		if (size == -1) {
			size = GetProp(gOptionsPH, "boot-command", ofBootArgs, 127);
			if (size == -1) ofBootArgs[0] = '\0';
				else ofBootArgs[size] = '\0';
			// Look for " bootr" but skip the number.
			if (!strncmp(ofBootArgs + 1, " bootr", 6)) {
				strcpy(ofBootArgs, ofBootArgs + 7);
			} else ofBootArgs[0] = '\0';
			SetProp(gChosenPH, "machargs", ofBootArgs, strlen(ofBootArgs) + 1);
		} else ofBootArgs[size] = '\0';
		// Force boot-command to start with 0 bootr.
		sprintf(gTempStr, "0 bootr%s", ofBootArgs);
		SetProp(gOptionsPH, "boot-command", gTempStr, strlen(gTempStr));
	} else {
		size = GetProp(gOptionsPH, "boot-args", ofBootArgs, 127);
		if (size == -1) ofBootArgs[0] = '\0';
		else ofBootArgs[size] = '\0';
	}
  
  if (ofBootArgs[0] != '\0') {
    // Look for special options and copy the rest.
    dash = 0;
    ofArgs = ofBootArgs;
    while ((tc = *ofArgs) != '\0') { 
      tc = tolower(tc);
      
      // Check for entering a dash arg.
      if (tc == '-') {
	dash = 1;
	ofArgs++;
	continue;
      }
      
      // Do special stuff if in a dash arg.
      if (dash) {
	if        (tc == 's') {
	  graphicsBoot = 0;
	  ofArgs++;
	  sKey = 0;
	} else if (tc == 'v') {
	  graphicsBoot = 0;
	  ofArgs++;
	  vKey = 0;
	} else {
	  // Check for exiting dash arg
	  if (isspace(tc)) dash = 0;
	  
	  // Copy any non 's' or 'v'
	  ofArgs++;
	}
      } else {
	// Not a dash arg so just copy it.
	ofArgs++;
      }
    }
  }
  
  // Add any pressed keys (s, v, shift) to the command line
  keyPos = 0;
  if (sKey || vKey || (gBootMode & kBootModeSafe)) {
    keyStr[keyPos++] = '-';
    
    if (sKey) keyStr[keyPos++] = 's';
    if (vKey) keyStr[keyPos++] = 'v';
    if (gBootMode & kBootModeSafe) keyStr[keyPos++] = 'x';
    
    keyStr[keyPos++] = ' ';
  }
  keyStr[keyPos++] = '\0';
  
  sprintf(args->CommandLine, "%s%s", keyStr, ofBootArgs);
  
  // If the address or size cells are larger than 1, use page numbers
  // and signify Boot Args Version 2.
  if ((gRootAddrCells == 1) && (gRootSizeCells == 1)) bank_shift = 0;
  else {
    bank_shift = 12;
    args->Version = kBootArgsVersion2;
  }
  
  // Get the information about the memory banks
  memoryPH = FindDevice("/memory");
  if (memoryPH == -1) return -1;
  size = GetProp(memoryPH, "reg", mem_regs, kMaxDRAMBanks * 16);
  if (size == 0) return -1;
  mem_banks = size / (4 * (gRootAddrCells + gRootSizeCells));
  if (mem_banks > kMaxDRAMBanks) mem_banks = kMaxDRAMBanks;
  
  // Convert the reg properties to 32 bit values
  for (cnt = 0; cnt < mem_banks; cnt++) {
    if (gRootAddrCells == 1) {
      args->PhysicalDRAM[cnt].base =
	*(unsigned long *)(mem_regs + cnt * 4 * (gRootAddrCells + gRootSizeCells)) >> bank_shift;
    } else {
      args->PhysicalDRAM[cnt].base =
	*(unsigned long long *)(mem_regs + cnt * 4 * (gRootAddrCells + gRootSizeCells)) >> bank_shift;
      
    }
    
    if (gRootSizeCells == 1) {
      args->PhysicalDRAM[cnt].size =
	*(unsigned long *)(mem_regs + cnt * 4 * (gRootAddrCells + gRootSizeCells) + 4 * gRootAddrCells) >> bank_shift;
    } else {
      args->PhysicalDRAM[cnt].size =
	*(unsigned long long *)(mem_regs + cnt * 4 * (gRootAddrCells + gRootSizeCells) + 4 * gRootAddrCells) >> bank_shift;
      
    }
  }
  
  // Collapse the memory banks into contiguous chunks
  for (cnt = 0; cnt < mem_banks - 1; cnt++) {
    if ((args->PhysicalDRAM[cnt + 1].base != 0) &&
	((args->PhysicalDRAM[cnt].base + args->PhysicalDRAM[cnt].size) !=
	 args->PhysicalDRAM[cnt + 1].base)) continue;
    
    args->PhysicalDRAM[cnt].size += args->PhysicalDRAM[cnt + 1].size;
    bcopy(args->PhysicalDRAM + cnt + 2, args->PhysicalDRAM + cnt + 1, (mem_banks - cnt - 2) * sizeof(DRAMBank));
    mem_banks--;
    cnt--;
  }
  bzero(args->PhysicalDRAM + mem_banks, (kMaxDRAMBanks - mem_banks) * sizeof(DRAMBank));
  
  // Get the video info
  GetMainScreenPH(&args->Video);
  args->Video.v_display = graphicsBoot;
  
  // Add the DeviceTree to the memory-map.
  // The actuall address and size must be filled in later.
  AllocateMemoryRange("DeviceTree", 0, 0);
  
  ret = FlattenDeviceTree();
  if (ret != 0) return -1;
  
  // Fill in the address and size of the device tree.
  if (gDeviceTreeAddr) {
    gDeviceTreeMMTmp[0] = gDeviceTreeAddr;
    gDeviceTreeMMTmp[1] = gDeviceTreeSize;
  }
  
  args->deviceTreeP = (void *)gDeviceTreeAddr;
  args->deviceTreeLength = gDeviceTreeSize;
  args->topOfKernelData = AllocateKernelMemory(0);
  
  return 0;
}


static long CallKernel(void)
{
  unsigned long msr, cnt;
  
  Quiesce();
  
  printf("\nCall Kernel!\n");
  
  // Save SPRs for OF
  __asm__ volatile("mfmsr %0" : "=r" (gOFMSRSave));
  __asm__ volatile("mfsprg %0, 0" : "=r" (gOFSPRG0Save));
  __asm__ volatile("mfsprg %0, 1" : "=r" (gOFSPRG1Save));
  __asm__ volatile("mfsprg %0, 2" : "=r" (gOFSPRG2Save));
  __asm__ volatile("mfsprg %0, 3" : "=r" (gOFSPRG3Save));
  
  // Turn off translations
  msr = 0x00001000;
  __asm__ volatile("sync");
  __asm__ volatile("mtmsr %0" : : "r" (msr));
  __asm__ volatile("isync");
  
  // Save the OF's Exceptions Vectors
  bcopy(0x0, gOFVectorSave, kVectorSize);
  
  // Move the Exception Vectors
  bcopy(gVectorSaveAddr, 0x0, kVectorSize);
  for (cnt = 0; cnt < kVectorSize; cnt += 0x20) {
    __asm__ volatile("dcbf 0, %0" : : "r" (cnt));
    __asm__ volatile("icbi 0, %0" : : "r" (cnt));
  }
  
  // Move the Image1 save area for OF 1.x / 2.x
  if (gOFVersion < kOFVersion3x) {
    bcopy((char *)kImageAddr1Phys, (char *)kImageAddr1, kImageSize1);
    for (cnt = kImageAddr1; cnt < kImageSize1; cnt += 0x20) {
      __asm__ volatile("dcbf 0, %0" : : "r" (cnt));
      __asm__ volatile("icbi 0, %0" : : "r" (cnt));
    }
  }
  
  // Make sure everything get sync'd up.
  __asm__ volatile("isync");
  __asm__ volatile("sync");
  __asm__ volatile("eieio");
  
  // Call the Kernel's entry point
  (*(void (*)())gKernelEntryPoint)(gBootArgsAddr, kMacOSXSignature);
  
  // Restore OF's Exception Vectors
  bcopy(gOFVectorSave, 0x0, 0x3000);
  for (cnt = 0; cnt < kVectorSize; cnt += 0x20) {
    __asm__ volatile("dcbf 0, %0" : : "r" (cnt));
    __asm__ volatile("icbi 0, %0" : : "r" (cnt));
  }
  
  // Restore SPRs for OF
  __asm__ volatile("mtsprg 0, %0" : : "r" (gOFSPRG0Save));
  __asm__ volatile("mtsprg 1, %0" : : "r" (gOFSPRG1Save));
  __asm__ volatile("mtsprg 2, %0" : : "r" (gOFSPRG2Save));
  __asm__ volatile("mtsprg 3, %0" : : "r" (gOFSPRG3Save));
  
  // Restore translations
  __asm__ volatile("sync");
  __asm__ volatile("mtmsr %0" : : "r" (gOFMSRSave));
  __asm__ volatile("isync");
  
  return -1;
}

// I've changed FailToBoot to take a reason, rather than a number.
// More analogous to a panic, and easier for users to understand.
// I've also made it reset the output level if not already verbose
// and added some additional diagnostic info.
// ryan.rempel@utoronto.ca

static void FailToBoot (char *reason)
{
	int verbose = TestForKey (kCommandKey) && (TestForKey ('s') || TestForKey ('v'));
	if (!verbose) {
		DrawFailedBootPicture ();
		SetOutputLevel (kOutputLevelFull);
	}
	
	printf ("\n");
	printf ("FailToBoot: %s\n", reason);
    printf ("Custom BootX %s, based on BootX %s\n", kBootXVersionXPF, kBootXVersion);
	printf ("gBootSourceNumber = %ld\n", gBootSourceNumber);
	printf ("gBootDevice = %s\n", gBootDevice);
	printf ("gBootFile = %s\n", gBootFile);
	printf ("ofBootArgs = %s\n", ofBootArgs);
	printf ("gExtensionsSpec = %s\n", gExtensionsSpec);

	printf ("\n");
	printf ("To reboot into Mac OS 9, use command-control-powerkey to force reboot,\n");
	printf ("and then hold down the option key.\n");
	printf ("\n");

	if (verbose) {
		Enter ();
	} else {
		printf ("For additional diagnostic information, reboot with command-v held down\n");
		while (1) reason = 0;
	}
}

static long InitMemoryMap(void)
{
  long result;
  
  result = Interpret(0, 1,
		     " dev /chosen"
		     " new-device"
		     " \" memory-map\" device-name"
		     " active-package"
		     " device-end"
		     , &gMemoryMapPH);
    
  return result;
}


static long GetOFVersion(void)
{
  CICell ph;
  char   versStr[256], *tmpStr;
  long   vers, size;
  
  // Get the openprom package
  ph = FindDevice("/openprom");
  if (ph == -1) return 0;
  
  // Get it's model property
  size = GetProp(ph, "model", versStr, 255);
  if (size == -1) return -1;
  versStr[size] = '\0';
  
  // Find the start of the number.
  tmpStr = NULL;
  if (!strncmp(versStr, "Open Firmware, ", 15)) {
    tmpStr = versStr + 15;
  } else if (!strncmp(versStr, "OpenFirmware ", 13)) {
    tmpStr = versStr + 13;
  } else return -1;  
  
  // Clasify by each instance as needed...
  switch (*tmpStr) {
  case '1' :
    vers = kOFVersion1x;
    break;
    
  case '2' :
    vers = kOFVersion2x;
    break;
    
  case '3' :
    vers = kOFVersion3x;
    break;
    
  case '4' :
    vers = kOFVersion4x;
    break;
    
  default :
    vers = 0;
    break;
  }

  return vers;
}


static long TestForKey(long key)
{
  long keyNum;
  long bp;
  char tc;
  
  if (gOFVersion < kOFVersion3x) {
    switch(key) {
    case 'a' :         keyNum =   7; break;
    case 's' :         keyNum =   6; break;
    case 'v' :         keyNum =  14; break;
    case 'y' :         keyNum =  23; break;
	case 'r' :		   keyNum =   8; break;
    case kCommandKey : keyNum =  48; break;
    case kOptKey     : keyNum =  61; break;
    case kShiftKey   : keyNum =  63; break;
    case kControlKey : keyNum =  49; break;
    default : keyNum = -1; break;
    }
  } else {
    switch(key) {
    case 'a' :         keyNum =   3; break;
    case 's' :         keyNum =  17; break;
    case 'v' :         keyNum =  30; break;
    case 'y' :         keyNum =  27; break;
    case kCommandKey : keyNum = 228; break;
    case kOptKey     : keyNum = 229; break;
    case kShiftKey   : keyNum = 230; break;
    case kControlKey : keyNum = 231; break;
    default : keyNum = -1; break;
    }
    
    // Map the right modifier keys on to the left.
    gKeyMap[28] |= gKeyMap[28] << 4;
  }
  
  if (keyNum == -1) return 0;
  
  bp = keyNum & 7;
  tc = gKeyMap[keyNum >> 3];
  
  return (tc & (1 << bp)) != 0;
}


#define kBootpBootFileOffset (108)

static long GetBootPaths(void)
{
  long ret, cnt, cnt2, cnt3, cnt4, size, partNum, bootplen, bsdplen;
  unsigned long adler32;
  char *filePath, *buffer;
  const char *priv = "\\private";
  const char *tmp = ",\\tmp\\";
  gApparentBootFile[0] = 0;
  
  if (gBootSourceNumber == -1) {
    // Get the boot-device
    size = GetProp(gChosenPH, "bootpath", gBootDevice, 255);
    gBootDevice[size] = '\0';
    if (gBootDevice[0] == '\0') {
      size = GetProp(gOptionsPH, "boot-device", gBootDevice, 255);
      gBootDevice[size] = '\0';
    }
    gBootDeviceType = GetDeviceType(gBootDevice);
    
    // Get the boot-file
    size = GetProp(gChosenPH, "bootargs", gBootFile, 256);
    gBootFile[size] = '\0';

	// Added by ryan.rempel@utoronto.ca
	// Look for the -h, to see if we're using a helper
	if (strstr (gBootFile, "-h")) {
		char ofBootArgs[128];
		char *pos, *end;
		
		size = GetProp (gChosenPH, "machargs", ofBootArgs, 127);
		if (size == -1) size = GetProp (gOptionsPH, "boot-command", ofBootArgs, 127);
		if (size == -1) {
			ofBootArgs[0] = '\0';
		} else {
			ofBootArgs[size] = '\0';
		}
		pos = strstr (ofBootArgs, "rd=*");
		if (pos) {
			// isolate the rd variable
			pos += 4;
			end = strstr (pos, " ");
			if (end) *end = 0;
			
			// set the apparent boot-file
			sprintf (gApparentBootFile, "%s,\\mach_kernel", pos);
			
			// set the "real" boot-file to where XPostFacto has copied it to
			while (*pos == '/') pos++;
			end = pos;
			while (*end) {
				if (*end == '/') *end = '\\';
				if (*end == ':') *end = ';';
				end++;
			}
			sprintf (gBootFile, ",\\.XPostFacto\\%s\\mach_kernel", pos);
		}
	} else {
		// If not using a helper, then check for the old -i scheme
		if (strstr (gBootFile, "-i")) strcpy (gBootFile, ",\\private\\tmp\\mach_kernel");
	}
	
	// If it starts with /tmp, then add /private
	if (!strncmp (gBootFile, tmp, strlen (tmp))) {
		int privSize = strlen (priv);
		memcpy (&gBootFile[privSize + 1], &gBootFile[1], strlen (gBootFile));
		memcpy (&gBootFile[1], priv, privSize);
	}
	
	// If it starts with a comma, add the bootdevice
	if (gBootFile[0] == ',') {
		int bootDeviceSize = strlen (gBootDevice);
		memcpy (&gBootFile[bootDeviceSize], gBootFile, strlen (gBootFile) + 1);
		memcpy (gBootFile, gBootDevice, bootDeviceSize);
	}
	
    if (gBootFile[0] != '\0') {
      gBootFileType = GetDeviceType(gBootFile);
      gBootSourceNumberMax = 0;
    } else {
      gBootSourceNumber = 0;
      gBootFileType = gBootDeviceType;
      if (gBootFileType == kNetworkDeviceType) gBootSourceNumberMax = 1;
      else {
	if (gOFVersion < kOFVersion3x) {
	  gBootSourceNumberMax = 4;
	} else {
	  gBootSourceNumberMax = 6;
	}
      }
    }
    
    if (gBootFileType == kNetworkDeviceType) {
      SetProp(Peer(0), "net-boot", NULL, 0);
    }
  }
  
  if (gBootSourceNumber >= gBootSourceNumberMax) return -1;
  
  if (gBootSourceNumberMax != 0) {
    switch (gBootFileType) {
    case kNetworkDeviceType :
      // Find the end of the device spec.
      cnt = 0;
      while (gBootDevice[cnt] != ':') cnt++;
      
      // Copy the device spec with the ':'.
      strncpy(gBootFile, gBootDevice, cnt + 1);
      
      // Check for bootp-responce or bsdp-responce.
      bootplen = GetPropLen(gChosenPH, "bootp-response");
      bsdplen  = GetPropLen(gChosenPH, "bsdp-response");
      if ((bootplen > 0) || (bsdplen > 0)) {
	if (bootplen > 0) {
	  buffer = malloc(bootplen);
	  GetProp(gChosenPH, "bootp-response", buffer, bootplen);
	} else {
	  buffer = malloc(bsdplen);
	  GetProp(gChosenPH, "bsdp-response", buffer, bsdplen);
	}
	
	// Flip the slash's to back slash's while looking for the last one.
	cnt = cnt2 = kBootpBootFileOffset;
	while (buffer[cnt] != '\0') {
	  if (buffer[cnt] == '/') {
	    buffer[cnt] = '\\';
	    cnt2 = cnt + 1;
	  }
	  cnt++;
	}
	
	// Add a comma at the front.
	buffer[kBootpBootFileOffset - 1] = ',';
	
	// Append the the root dir to the device spec.
	strncat(gBootFile, buffer + kBootpBootFileOffset - 1,
		cnt2 - kBootpBootFileOffset + 1);
	
	free(buffer);
      } else {
	// Look for the start of the root dir path.
	cnt3 = cnt;
	while (gBootDevice[cnt3] != ',') cnt3++;
	
	// Find the end of the path.  Look for a comma or null.
	cnt2 = cnt3 + 1;
	while ((gBootDevice[cnt2] != '\0') && (gBootDevice[cnt2] != ',')) cnt2++;
	
	// Find the last back slash or comma in the path
	cnt4 = cnt2 - 1;
	while ((gBootDevice[cnt4] != ',') && (gBootDevice[cnt4] != '\\')) cnt4--;
	
	// Copy the IP addresses if needed.
	if (gOFVersion < kOFVersion3x) {
	  strncat(gBootFile, gBootDevice + cnt + 1, cnt3 - cnt - 1);
	}
	
	// Add on the directory path
	strncat(gBootFile, gBootDevice + cnt3, cnt4 - cnt3 + 1);
      }
      
      // Add on the kernel name
      strcat(gBootFile, "mach.macosx");
      
      // Add on postfix
      strcat(gBootFile, gBootDevice + cnt2);
      break;
      
    case kBlockDeviceType :
      // Find the first ':'.
      cnt = 0;
      while ((gBootDevice[cnt] != '\0') && (gBootDevice[cnt] != ':')) cnt++;
      if (gBootDevice[cnt] == '\0') {
	    printf ("BootX::GetBootPaths gBootDevice has no colon: %s\n", gBootDevice);
        return -1;
	  }
      
      // Find the comma after the ':'.
      cnt2 = cnt + 1;
      while ((gBootDevice[cnt2]  != '\0') && (gBootDevice[cnt] != ',')) cnt2++;
      
      // Get just the partition number
      strncpy(gBootFile, gBootDevice + cnt + 1, cnt2 - cnt - 1);
      partNum = strtol(gBootFile, 0, 10);
      if (partNum == 0) partNum = strtol(gBootFile, 0, 16);
      
      // Adjust the partition number.
      // Pass 0 & 1, no offset. Pass 2 & 3, offset 1, Pass 4 & 5, offset 2.
      partNum += gBootSourceNumber / 2;
      
      // Construct the boot-file
      strncpy(gBootFile, gBootDevice, cnt + 1);
      sprintf(gBootFile + cnt + 1, "%ld,%s\\mach_kernel",
	      partNum, ((gBootSourceNumber & 1) ? "" : "\\"));
      
      // and the cache file name
      
      bzero(gCacheNameAdler + 64, sizeof(gBootFile));
      strcpy(gCacheNameAdler + 64, gBootFile);
      adler32 = Alder32(gCacheNameAdler, sizeof(gCacheNameAdler));
      
      strncpy(gBootKernelCacheFile, gBootDevice, cnt + 1);
      sprintf(gBootKernelCacheFile + cnt + 1, 
		"%ld,\\System\\Library\\Caches\\com.apple.kernelcaches\\kernelcache.%08lX", partNum, adler32);
      break;
      
    default:
      printf("Failed to infer Boot Device Type.\n");
      return -1;
      break;
    }
  }
  
  // Figure out the root dir.
  ret = ConvertFileSpec(gBootFile, gExtensionsSpec, &filePath);
  if (ret == -1) {
	printf ("BootX::GetBootPaths error from ConvertFileSpec for gBootFile: %s gExtensionsSpec: %s\n", gBootFile, gExtensionsSpec);
	return -1;
  }
  
  strcat(gExtensionsSpec, ",");
  
  // Add in any extra path to gRootDir.
  cnt = 0;
  while (filePath[cnt] != '\0') cnt++;
  
  if (cnt != 0) {
    for (cnt2 = cnt - 1; cnt2 >= 0; cnt2--) {
      if (filePath[cnt2] == '\\') {
	strncat(gExtensionsSpec, filePath, cnt2 + 1);
	break;
      }
    }
  }

  // Figure out the extensions dir.
  if (gBootFileType == kBlockDeviceType) {
    cnt = strlen(gExtensionsSpec);
    if ((cnt > 2) && (gExtensionsSpec[cnt-1] == '\\') && (gExtensionsSpec[cnt-2] == '\\'))
	cnt--;
    strcpy(gExtensionsSpec + cnt, "System\\Library\\");
  }

  if (gApparentBootFile[0]) {
    SetProp(gChosenPH, "rootpath", gApparentBootFile, strlen(gApparentBootFile) + 1);
  } else {
	SetProp(gChosenPH, "rootpath", gBootFile, strlen(gBootFile) + 1);
  }
  
  gBootSourceNumber++;
  
  return 0;
}

// Public Functions

long GetDeviceType(char *devSpec)
{
  CICell ph;
  long   size;
  char   deviceType[32];
  
  ph = FindDevice(devSpec);
  if (ph == -1) return -1;
  
  size = GetProp(ph, "device_type", deviceType, 31);
  if (size != -1) deviceType[size] = '\0';
  else deviceType[0] = '\0';
  
  if (strcmp(deviceType, "network") == 0) return kNetworkDeviceType;
  if (strcmp(deviceType, "block") == 0) return kBlockDeviceType;
  
  return kUnknownDeviceType;
}


long ConvertFileSpec(char *fileSpec, char *devSpec, char **filePath)
{
  long cnt;
  
  // Find the first ':' in the fileSpec.
  cnt = 0;
  while ((fileSpec[cnt] != '\0') && (fileSpec[cnt] != ':')) cnt++;
  if (fileSpec[cnt] == '\0') {
    printf ("BootX::ConvertFileSpec could not find colon in: %s\n", fileSpec);
    return -1;
  }
  
  // Find the next ',' in the fileSpec.
  while ((fileSpec[cnt] != '\0') && (fileSpec[cnt] != ',')) cnt++;
  
  // Copy the string to devSpec.
  strncpy(devSpec, fileSpec, cnt);
  devSpec[cnt] = '\0';
  
  // If there is a filePath start it after the ',', otherwise NULL.
  if (filePath != NULL) {
    if (fileSpec[cnt] != '\0') {
      *filePath = fileSpec + cnt + 1;
    } else {
      *filePath = NULL;
    }
  }
  
  return 0;
}


long MatchThis(CICell phandle, char *string)
{
  long ret, length;
  char *name, *model, *compatible;
  
  ret = GetPackageProperty(phandle, "name", &name, &length);
  if ((ret == -1) || (length == 0)) name = NULL;
  
  ret = GetPackageProperty(phandle, "model", &model, &length);
  if ((ret == -1) || (length == 0)) model = NULL;
  
  ret = GetPackageProperty(phandle, "compatible", &compatible, &length);
  if ((ret == -1) || (length == 0)) model = NULL;
  
  if ((name != NULL) && strcmp(name, string) == 0) return 0;
  if ((model != NULL) && strcmp(model, string) == 0) return 0;
  
  if (compatible != NULL) {
    while (*compatible != '\0') { 
      if (strcmp(compatible, string) == 0) return 0;
      
      compatible += strlen(compatible) + 1;
    }
  }
  
  return -1;
}


void *AllocateBootXMemory(long size)
{
	long addr = gImageFirstBootXAddr - size;

	if (addr < gImageLastKernelAddr) FailToBoot ("BootX::AllocateBootXMemory () ran out of space");
	
	gImageFirstBootXAddr = addr;

	return (void *)addr;
}


long AllocateKernelMemory(long size)
{
  long addr = gImageLastKernelAddr;
  
  gImageLastKernelAddr += (size + 0xFFF) & ~0xFFF;
  
  if (gImageLastKernelAddr > gImageFirstBootXAddr) FailToBoot ("BootX::AllocateKernelMemory () ran out of space");
  
  return addr;
}


long AllocateMemoryRange(char *rangeName, long start, long length)
{
  long result, *buffer;
  
  buffer = AllocateBootXMemory(2 * sizeof(long));
  if (buffer == 0) return -1;
  
  buffer[0] = start;
  buffer[1] = length;
  
  result = SetProp(gMemoryMapPH, rangeName, (char *)buffer, 2 * sizeof(long));
  if (result == -1) {
    printf ("BootX::AllocateMemoryRange error from SetProp\n");
    return -1;
  }
  
  return 0;
}


unsigned long Alder32(unsigned char *buffer, long length)
{
  long          cnt;
  unsigned long result, lowHalf, highHalf;
  
  lowHalf = 1;
  highHalf = 0;
  
  for (cnt = 0; cnt < length; cnt++) {
    if ((cnt % 5000) == 0) {
      lowHalf  %= 65521L;
      highHalf %= 65521L;
    }
    
    lowHalf += buffer[cnt];
    highHalf += lowHalf;
  }

  lowHalf  %= 65521L;
  highHalf %= 65521L;
  
  result = (highHalf << 16) | lowHalf;
  
  return result;
}
