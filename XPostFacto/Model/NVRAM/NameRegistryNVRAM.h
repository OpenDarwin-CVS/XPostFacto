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

#ifndef __NAMEREGISTRYNVRAM_H__
#define __NAMEREGISTRYNVRAM_H__

#include <NVRAM.h>

class NVRAMVariables {

	public:

		void read ();
		int write ();
			
#if qDebug	
		friend ostream& operator<< (ostream& os, NVRAMVariables& var);
#endif
	
		bool getLittleEndian () {return buffer.littleEndian;}
		bool getRealMode () {return buffer.realMode;}
		bool getAutoBoot () {return buffer.autoBoot;}
		bool getDiagSwitch () {return buffer.diagSwitch;}
		bool getFCodeDebug () {return buffer.fcodeDebug;}
		bool getUseOEMLogo () {return buffer.useOEMLogo;}
		bool getUseOEMBanner () {return buffer.useOEMBanner;}
		bool getUseNVRAMRC () {return buffer.useNVRAMRC;}
					
		void setLittleEndian (bool val) {setNVRAMValue (buffer.littleEndian, val);}
		void setRealMode (bool val) {setNVRAMValue (buffer.realMode, val);}
		void setAutoBoot (bool val) {setNVRAMValue (buffer.autoBoot, val);}
		void setDiagSwitch (bool val) {setNVRAMValue (buffer.diagSwitch, val);}
		void setFCodeDebug (bool val) {setNVRAMValue (buffer.fcodeDebug, val);}
		void setUseOEMLogo (bool val) {setNVRAMValue (buffer.useOEMLogo, val);}
		void setUseOEMBanner (bool val) {setNVRAMValue (buffer.useOEMBanner, val);}
		void setUseNVRAMRC (bool val) {setNVRAMValue (buffer.useNVRAMRC, val);}

		UInt32 getRealBase () {return buffer.realBase;}
		UInt32 getRealSize () {return buffer.realSize;}
		UInt32 getVirtBase () {return buffer.virtBase;}
		UInt32 getVirtSize () {return buffer.virtSize;}
		UInt32 getLoadBase () {return buffer.loadBase;}
		UInt32 getPCIProbeList () {return buffer.pciProbeList;}
		UInt32 getScreenColumns () {return buffer.screenColumns;}
		UInt32 getScreenRows () {return buffer.screenRows;}
		UInt32 getSelftestMegs () {return buffer.selftestMegs;}
	
		void setRealBase (UInt32 val) {setNVRAMValue (buffer.realBase, val);}
		void setRealSize (UInt32 val) {setNVRAMValue (buffer.realSize, val);}
		void setVirtBase (UInt32 val) {setNVRAMValue (buffer.virtBase, val);}
		void setVirtSize (UInt32 val) {setNVRAMValue (buffer.virtSize, val);}
		void setLoadBase (UInt32 val) {setNVRAMValue (buffer.loadBase, val);}
		void setPCIProbeList (UInt32 val) {setNVRAMValue (buffer.pciProbeList, val);}
		void setScreenColumns (UInt32 val) {setNVRAMValue (buffer.screenColumns, val);}
		void setScreenRows (UInt32 val) {setNVRAMValue (buffer.screenRows, val);}
		void setSelftestMegs (UInt32 val) {setNVRAMValue (buffer.selftestMegs, val);}

		const char* getBootDevice () {return cacheBootDevice;}
		const char* getBootFile () {return cacheBootFile;}
		const char* getDiagDevice () {return cacheDiagDevice;}
		const char* getDiagFile () {return cacheDiagFile;}
		const char* getInputDevice () {return cacheInputDevice;}
		const char* getOutputDevice () {return cacheOutputDevice;}
		const char* getOEMBanner () {return cacheOEMBanner;}
		const char* getOEMLogo () {return cacheOEMLogo;}
		const char* getNVRAMRC () {return cacheNVRAMRC;}
		const char* getBootCommand () {return cacheBootCommand;}

		void setBootDevice (const char *val) {setString (&cacheBootDevice, val);}
		void setBootFile (const char *val) {setString (&cacheBootFile, val);}
		void setDiagDevice (const char *val) {setString (&cacheDiagDevice, val);}
		void setDiagFile (const char *val) {setString (&cacheDiagFile, val);}
		void setInputDevice (const char *val) {setString (&cacheInputDevice, val);}
		void setOutputDevice (const char *val) {setString (&cacheOutputDevice, val);}
		void setOEMBanner (const char *val) {setString (&cacheOEMBanner, val);}
		void setOEMLogo (const char *val) {setString (&cacheOEMLogo, val);}
		void setNVRAMRC (const char *val) {setString (&cacheNVRAMRC, val);}
		void setBootCommand (const char *val) {setString (&cacheBootCommand, val);}

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

		void setString (char **string, const char *val);
		void cacheString (NVRAMString string, char **cache);
		void cacheStrings ();
		void packStrings ();
		void packString (NVRAMString &string, char *cache);
		void packStringInOSBuffer (NVRAMString &string, char *cache);
		OSErr findNVRAMRegEntry ();
		
		bool fUseIOMem;
				
		// Here is the OWOF partition buffer

		NVRAMBuffer buffer;
		
		char osBuffer [0xC00];
		unsigned osBufferOffset;

		bool hasChanged;
		
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