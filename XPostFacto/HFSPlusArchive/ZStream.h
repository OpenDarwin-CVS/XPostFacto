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

#ifndef __ZSTREAM_H__
#define __ZSTREAM_H__

#ifdef __GNUC__
#define FILE_STREAM_TYPE fstream
#define ZSTREAM_TYPE ZStreamRep
#include <fstream.h>
#else
#define FILE_STREAM_TYPE 	CRandomAccessStream_AC
#define ZSTREAM_TYPE ZStream
#include "CRandomAccessStream_AC.h"
#endif

#include "zlib.h"

enum ZStreamInitState {
	kZStreamNoInit,
	kZStreamDeflateInit,
	kZStreamInflateInit
};

class ZStream
#ifndef __GNUC__
: public CStream_AC
#endif
{

	public:
	
		ZStream (FILE_STREAM_TYPE *stream);
		~ZStream ();	
		
};

class ZStreamRep
#ifndef __GNUC__
: public CStreamRep_AC
#endif
{

public:

	ZStreamRep (FILE_STREAM_TYPE *stream);
	virtual ~ZStreamRep ();

	virtual void WriteBytes(const void* inPtr, long amt);
	virtual void ReadBytes(void* inPtr, long amt);
	virtual void Flush();

private:

	void fillBuffer ();

	FILE_STREAM_TYPE *fStream;
	z_stream fZStream;
	unsigned char fBuffer [16 * 1024];
	ZStreamInitState fInitState;
	long fBytesLeft;

};

#endif