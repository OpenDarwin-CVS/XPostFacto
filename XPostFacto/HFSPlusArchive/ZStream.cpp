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

#include "ZStream.h"
#include <iostream.h>
#include <Files.h>

#ifdef __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#define WRITE_BYTES write
#define READ_BYTES read
#define FLUSH flush
#else
#define WRITE_BYTES WriteBytes
#define READ_BYTES ReadBytes
#define FLUSH flush

#endif

ZStreamRep::ZStreamRep (FILE_STREAM_TYPE *stream)
{
	fStream = stream;

	fZStream.data_type = Z_UNKNOWN;
	fZStream.zalloc = Z_NULL;
	fZStream.zfree = Z_NULL;
	fZStream.opaque = Z_NULL;
	
	fInitState = kZStreamNoInit;
}

ZStreamRep::~ZStreamRep ()
{
	switch (fInitState) {
		case kZStreamInflateInit:
			inflateEnd (&fZStream);
			break;
			
		case kZStreamDeflateInit:
			deflateEnd (&fZStream);
			break;

		default:
			break;
	}
}

void
ZStreamRep::fillBuffer ()
{
	if (fBytesLeft > 0) {
		try {
			fStream->READ_BYTES (fBuffer, fBytesLeft < sizeof(fBuffer) ? fBytesLeft : sizeof(fBuffer));
		}
		catch (...) {
			// Mainly to catch EOF, but overbroad at this point.
			// I'm just ignoring EOF, on the theory that zlib will
			// know when the stream actually ends. Of course, I am 
			// being very trusting of the stream's validity, which
			// isn't ideal.
		}
		fZStream.next_in = fBuffer;
		fZStream.avail_in = fBytesLeft < sizeof(fBuffer) ? fBytesLeft : sizeof(fBuffer);
		fBytesLeft -= sizeof(fBuffer);
	}
}

void
ZStreamRep::Flush ()
{
	// The idea is that we're finished now.
	fZStream.next_in = 0;
	fZStream.avail_in = 0;
	int result = deflate (&fZStream, Z_FINISH);
	switch (result) {
		case Z_STREAM_END:
			fStream->WRITE_BYTES (fBuffer, sizeof (fBuffer) - fZStream.avail_out);
			break;
	
		case Z_OK:
			fStream->WRITE_BYTES (fBuffer, sizeof (fBuffer));
			fZStream.next_out = fBuffer;
			fZStream.avail_out = sizeof (fBuffer);
			Flush ();
			break;
			
		default:
			throw (OSErr) result;
	}
}

void 
ZStreamRep::WriteBytes(const void* inPtr, long amt)
{
	int result;

	if (amt == 0) return;

	switch (fInitState) {
		case kZStreamInflateInit:
			throw -1;
			
		case kZStreamNoInit:
			fZStream.next_out = fBuffer;
			fZStream.avail_out = sizeof (fBuffer);
			
			result = deflateInit (&fZStream, Z_DEFAULT_COMPRESSION);
			if (result != Z_OK) throw result;
			
			fInitState = kZStreamDeflateInit;
			break;

		default:
			break;
	}

	fZStream.next_in = (unsigned char *) inPtr;
	fZStream.avail_in = amt;
	result = deflate (&fZStream, Z_NO_FLUSH);
	if (result != Z_OK) throw result;
	if (fZStream.avail_out == 0) {
		fStream->WRITE_BYTES (fBuffer, sizeof (fBuffer));
		fZStream.next_out = fBuffer;
		fZStream.avail_out = sizeof (fBuffer);
		WriteBytes (fZStream.next_in, fZStream.avail_in);
	}
}

void
ZStreamRep::ReadBytes(void* inPtr, long amt)
{
	int result;
	if (amt == 0) return;

#ifdef __GNUC__
	struct stat streamStat;
#endif
	
	switch (fInitState) {
		case kZStreamDeflateInit:
			throw -1;
			
		case kZStreamNoInit:
#ifdef __GNUC__
			fstat (fStream->rdbuf()->fd(), &streamStat);
			fBytesLeft = streamStat.st_size;
#else
			fBytesLeft = fStream->GetSize ();
#endif
			fillBuffer ();
			result = inflateInit (&fZStream);
			if (result != Z_OK) {
				throw result;
			}
			fInitState = kZStreamInflateInit;
			break;

		default:
			break;
	}
	
	fZStream.next_out = (unsigned char *) inPtr;
	fZStream.avail_out = amt;
	result = inflate (&fZStream, Z_SYNC_FLUSH);
	if (result != Z_OK) {
		throw result;
	}
	if ((fZStream.avail_out != 0) && (fZStream.avail_in == 0)) {
		fillBuffer ();
		ReadBytes (fZStream.next_out, fZStream.avail_out);
	}
}


ZStream::ZStream (FILE_STREAM_TYPE *stream)
#ifndef __GNUC__
	: CStream_AC (new ZStreamRep (stream))
#endif
{
	
}

ZStream::~ZStream ()
{

}