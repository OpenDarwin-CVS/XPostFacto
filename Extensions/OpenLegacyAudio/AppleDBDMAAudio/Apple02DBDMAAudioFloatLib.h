/*
 *  Apple02DBDMAAudioFloatLib.h
 *  Apple02Audio
 *
 * 	Public interface for floating point library.
 *  The library includes input and output conversion 
 *  routines with different bit depths and processing
 *  requirements.  It also contains utilities that 
 *  require floating point.
 *	
 *  Created by Aram Lindahl on Thu Nov 14 2002.
 *  Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 */

// #include "iSubTypes.h"

extern "C" {

#pragma mark ----------------------------- 
#pragma mark ��� Processing Functions
#pragma mark ----------------------------- 

void delayRightChannel(float* inFloatBufferPtr, UInt32 numSamples, float * inLastSample) ;
void balanceAdjust(float* inFloatBufferPtr, UInt32 numSamples, float* leftSoftVolume, float* rightSoftVolume);
void invertRightChannel(float* inFloatBufferPtr, UInt32 numSamples) ;
void mixAndMuteRightChannel(float* inFloatBufferPtr, UInt32 numSamples); 

#pragma mark ----------------------------- 
#pragma mark ��� Integer to Float
#pragma mark ----------------------------- 

void Int8ToFloat32( SInt8 *src, float *dest, unsigned int count );

void NativeInt16ToFloat32( signed short *src, float *dest, unsigned int count, int bitDepth );
void NativeInt24ToFloat32( long *src, float *dest, unsigned int count, int bitDepth );
void NativeInt32ToFloat32( long *src, float *dest, unsigned int count, int bitDepth );

void NativeInt16ToFloat32Gain( signed short *src, float *dest, unsigned int count, int bitDepth, float* inGainLPtr, float* inGainRPtr );
void NativeInt16ToFloat32CopyRightToLeft( signed short *src, float *dest, unsigned int count, int bitDepth );
void NativeInt16ToFloat32CopyLeftToRight( signed short *src, float *dest, unsigned int count, int bitDepth ); // [3306493]
void NativeInt32ToFloat32Gain( signed long *src, float *dest, unsigned int count, int bitDepth, float* inGainLPtr, float* inGainRPtr );

void SwapInt16ToFloat32( signed short *src, float *dest, unsigned int count, int bitDepth );
void SwapInt24ToFloat32( long *src, float *dest, unsigned int count, int bitDepth );
void SwapInt32ToFloat32( long *src, float *dest, unsigned int count, int bitDepth );

#pragma mark ----------------------------- 
#pragma mark ��� Float to Integer
#pragma mark ----------------------------- 

void Float32ToInt8( float *src, SInt8 *dst, unsigned int count );
void Float32ToNativeInt16( float *src, signed short *dst, unsigned int count );
void Float32ToNativeInt24( float *src, signed long *dst, unsigned int count );
void Float32ToNativeInt32( float *src, signed long *dst, unsigned int count );
void Float32ToSwapInt16( float *src, signed short *dst, unsigned int count );
void Float32ToSwapInt24( float *src, signed long *dst, unsigned int count );
void Float32ToSwapInt32( float *src, signed long *dst, unsigned int count );

//void NativeInt16ToFloat32CopyLeftToRight( signed short *src, float *dest, unsigned int count, int bitDepth );
//void NativeInt16ToFloat32CopyLeftToRightGain( signed short *src, float *dest, unsigned int count, int bitDepth, float inGain );
//void NativeInt16ToFloat32CopyRightToLeftGain( signed short *src, float *dest, unsigned int count, int bitDepth, float inGain );
//void NativeInt32ToFloat32CopyLeftToRight( signed long *src, float *dest, unsigned int count, int bitDepth );
//void NativeInt32ToFloat32CopyRightToLeft( signed long *src, float *dest, unsigned int count, int bitDepth );
//void NativeInt32ToFloat32CopyLeftToRightGain( signed long *src, float *dest, unsigned int count, int bitDepth, float inGain );
//void NativeInt32ToFloat32CopyRightToLeftGain( signed long *src, float *dest, unsigned int count, int bitDepth, float inGain );
//void Float32ToNativeInt16MixAndMuteRight( float *src, signed short *dst, unsigned int count );
//void Float32ToNativeInt32MixAndMuteRight( float *src, signed long *dst, unsigned int count );

#pragma mark ---------------------------------------- 
#pragma mark ��� Utilities
#pragma mark ---------------------------------------- 

UInt32 	CalculateOffset (UInt64 nanoseconds, UInt32 sampleRate);
void 	dBfixed2float(UInt32 indBfixed, float* ioGainPtr);
void 	inputGainConverter(UInt32 inGainIndex, float* ioGainPtr);

};
