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


// =============
//  NVRAM
// =============

#ifndef __NVRAM_H__
#define __NVRAM_H__

#include <iostream.h>

struct NVRAMString {
	UInt16 offset;
	UInt16 length;
};

#define kOFStringCapacity 1956

struct NVRAMBuffer {
	UInt16   magic;   	// 0x1275
	UInt8    version;	// 0x05
	UInt8    pages;		// 0x08
	UInt16   checksum;
	UInt16   here;
	UInt16   top;		
	UInt16   next;		// 0x0000
	
	unsigned littleEndian:1;
	unsigned realMode:1;
	unsigned autoBoot:1;
	unsigned diagSwitch:1;
	unsigned fcodeDebug:1;
	unsigned useOEMBanner:1;
	unsigned useOEMLogo:1;
	unsigned useNVRAMRC:1;	
	unsigned pad:24;
	
	UInt32 realBase;
	UInt32 realSize;
	UInt32 virtBase;
	UInt32 virtSize;
	UInt32 loadBase;
	UInt32 pciProbeList;
	UInt32 screenColumns;
	UInt32 screenRows;
	UInt32 selftestMegs;

	NVRAMString bootDevice;
	NVRAMString bootFile;
	NVRAMString diagDevice;
	NVRAMString diagFile;
	NVRAMString inputDevice;		
	NVRAMString outputDevice;	
	NVRAMString oemBanner;
	NVRAMString oemLogo;
	NVRAMString nvramrc;
	NVRAMString bootCommand;

	char strings[kOFStringCapacity];
};

#define setNVRAMValue(value,newValue)	\
	if (value != newValue) {			\
		hasChanged = true; 				\
		value = newValue;				\
	}

class NVRAMVariables {

	public:

		virtual void read ();
		virtual int write ();
		
		char* getCompatible () {return fCompatible;}
		bool getCanWritePatches () {return fCanWriteNVRAMPatches;}
			
#if qDebug	
		friend ostream& operator<< (ostream& os, NVRAMVariables& var);
#endif
	
		virtual bool getLittleEndian () {return buffer.littleEndian;}
		virtual bool getRealMode () {return buffer.realMode;}
		virtual bool getAutoBoot () {return buffer.autoBoot;}
		virtual bool getDiagSwitch () {return buffer.diagSwitch;}
		virtual bool getFCodeDebug () {return buffer.fcodeDebug;}
		virtual bool getUseOEMLogo () {return buffer.useOEMLogo;}
		virtual bool getUseOEMBanner () {return buffer.useOEMBanner;}
		virtual bool getUseNVRAMRC () {return buffer.useNVRAMRC;}
					
		virtual void setLittleEndian (bool val) {setNVRAMValue (buffer.littleEndian, val);}
		virtual void setRealMode (bool val) {setNVRAMValue (buffer.realMode, val);}
		virtual void setAutoBoot (bool val) {setNVRAMValue (buffer.autoBoot, val);}
		virtual void setDiagSwitch (bool val) {setNVRAMValue (buffer.diagSwitch, val);}
		virtual void setFCodeDebug (bool val) {setNVRAMValue (buffer.fcodeDebug, val);}
		virtual void setUseOEMLogo (bool val) {setNVRAMValue (buffer.useOEMLogo, val);}
		virtual void setUseOEMBanner (bool val) {setNVRAMValue (buffer.useOEMBanner, val);}
		virtual void setUseNVRAMRC (bool val) {setNVRAMValue (buffer.useNVRAMRC, val);}

		virtual UInt32 getRealBase () {return buffer.realBase;}
		virtual UInt32 getRealSize () {return buffer.realSize;}
		virtual UInt32 getVirtBase () {return buffer.virtBase;}
		virtual UInt32 getVirtSize () {return buffer.virtSize;}
		virtual UInt32 getLoadBase () {return buffer.loadBase;}
		virtual UInt32 getPCIProbeList () {return buffer.pciProbeList;}
		virtual UInt32 getScreenColumns () {return buffer.screenColumns;}
		virtual UInt32 getScreenRows () {return buffer.screenRows;}
		virtual UInt32 getSelftestMegs () {return buffer.selftestMegs;}
	
		virtual void setRealBase (UInt32 val) {setNVRAMValue (buffer.realBase, val);}
		virtual void setRealSize (UInt32 val) {setNVRAMValue (buffer.realSize, val);}
		virtual void setVirtBase (UInt32 val) {setNVRAMValue (buffer.virtBase, val);}
		virtual void setVirtSize (UInt32 val) {setNVRAMValue (buffer.virtSize, val);}
		virtual void setLoadBase (UInt32 val) {setNVRAMValue (buffer.loadBase, val);}
		virtual void setPCIProbeList (UInt32 val) {setNVRAMValue (buffer.pciProbeList, val);}
		virtual void setScreenColumns (UInt32 val) {setNVRAMValue (buffer.screenColumns, val);}
		virtual void setScreenRows (UInt32 val) {setNVRAMValue (buffer.screenRows, val);}
		virtual void setSelftestMegs (UInt32 val) {setNVRAMValue (buffer.selftestMegs, val);}

		virtual const char* getBootDevice () {return cacheBootDevice;}
		virtual const char* getBootFile () {return cacheBootFile;}
		virtual const char* getDiagDevice () {return cacheDiagDevice;}
		virtual const char* getDiagFile () {return cacheDiagFile;}
		virtual const char* getInputDevice () {return cacheInputDevice;}
		virtual const char* getOutputDevice () {return cacheOutputDevice;}
		virtual const char* getOEMBanner () {return cacheOEMBanner;}
		virtual const char* getOEMLogo () {return cacheOEMLogo;}
		virtual const char* getNVRAMRC () {return cacheNVRAMRC;}
		virtual const char* getBootCommand () {return cacheBootCommand;}

		virtual void setBootDevice (const char *val) {setString (&cacheBootDevice, val);}
		virtual void setBootFile (const char *val) {setString (&cacheBootFile, val);}
		virtual void setDiagDevice (const char *val) {setString (&cacheDiagDevice, val);}
		virtual void setDiagFile (const char *val) {setString (&cacheDiagFile, val);}
		virtual void setInputDevice (const char *val) {setString (&cacheInputDevice, val);}
		virtual void setOutputDevice (const char *val) {setString (&cacheOutputDevice, val);}
		virtual void setOEMBanner (const char *val) {setString (&cacheOEMBanner, val);}
		virtual void setOEMLogo (const char *val) {setString (&cacheOEMLogo, val);}
		virtual void setNVRAMRC (const char *val) {setString (&cacheNVRAMRC, val);}
		virtual void setBootCommand (const char *val) {setString (&cacheBootCommand, val);}
		
		virtual void appendToNVRAMRC (const char *val);

		virtual UInt8 readByte (unsigned offset);
		virtual void writeByte (unsigned offset, UInt8 byte);

		static NVRAMVariables* GetVariables ();

	protected:
	
		NVRAMVariables ();
		virtual ~NVRAMVariables ();	

		static NVRAMVariables *gVariables;
		
#if qDebug
		static void testNameRegistry ();
#endif
	
		static NVRAMVariables* chooseSubClass ();

		virtual void setString (char **string, const char *val);
		void cacheString (NVRAMString string, char **cache);
		void cacheStrings ();
		void packStrings ();
		void packString (NVRAMString &string, char *cache);
		void packStringInOSBuffer (NVRAMString &string, char *cache);
		OSErr findNVRAMRegEntry ();
		
		void loadNVRAMPatch ();
		void processNVRAMPatchLine (char *line);
		void processNVRAMPatch (char *settings);
					
		// Here is the OWOF partition buffer

		NVRAMBuffer buffer;
		
		char osBuffer [0xC00];
		unsigned osBufferOffset;

		bool hasChanged;
		
		bool fCanWriteNVRAMPatches;
		bool fCanWriteNVRAM;
	
		char *fCompatible;
		
		// Caches for the string values.
		
		char *cacheBootDevice;
		char *cacheBootFile;
		char *cacheDiagDevice;
		char *cacheDiagFile;
		char *cacheInputDevice;
		char *cacheOutputDevice;
		char *cacheOEMBanner;
		char *cacheOEMLogo;
		char *cacheNVRAMRC;
		char *cacheBootCommand;
};

#endif