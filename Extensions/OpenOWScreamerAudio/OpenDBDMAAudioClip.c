#include <libkern/OSTypes.h>
#include <IOKit/IOReturn.h>

#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/audio/IOAudioDebug.h>

#include "iSubTypes.h"	// aml 3.1.02

//	floating point types
typedef	float				Float32;
typedef double				Float64;

#define FLOATLIB			FALSE

#define kMaxSInt32ValueInFloat 				(2.1474836480000e9)
#define kInvMaxSInt32ValueInFloat			(4.6566128730774e-10)
#define kMaxSInt16ValueInFloat				(3.2768000000000e4)
#define kInvMaxSInt16ValueInFloat			(3.0517578125000e-5)

#define kMaxSInt8ValueInFloat				(127.0)
#define kInvMaxSInt8ValueInFloat			(7.8740157e-3)


#define kMaxFloatMinusLSBSInt32				(0.99999999953434)
#define kMaxFloatMinusLSBSInt16				(0.99996948242188)
#define kMaxFloatMinusLSBSInt8				(0.9921259843)

const UInt32 kNumChannels = 2;

// aml 6.17.02
typedef enum {							
    e_Mode_Disabled = 0,
    e_Mode_CopyLeftToRight,
    e_Mode_CopyRightToLeft
} DualMonoModeType;

typedef struct _sPreviousValues {
    Float32	xl_1;
    Float32	xr_1;
    Float32	xl_2;
    Float32	xr_2;
    Float32	yl_1;
    Float32	yr_1;
    Float32	yl_2;
    Float32	yr_2;
} PreviousValues;

// aml 2.18.02 added structure for 1st order phase compensator
// use in case of 2nd order crossover filter
typedef struct _sPreviousValues1stOrder {
    Float32	xl_1;
    Float32	xr_1;
    Float32	yl_1;
    Float32	yr_1;
} PreviousValues1stOrder;


float gOldSample = 0;	// [3134221] aml
float gOldInputSample = 0;

// aml 4.30.02
// -24 dB to +24 dB in 1 dB steps
const UInt16 kZeroGaindBConvTableOffset = 24;
const UInt16 kMaxZeroGain = 24;
const UInt16 kMinZeroGain = 24;
static float zeroGaindBConvTable[] = {
    0.0631,  0.0708,  0.0794,  0.0891,  0.1000,  0.1122,  0.1259, 
    0.1413,  0.1585,  0.1778,  0.1995,  0.2239,  0.2512,  0.2818,
    0.3162,  0.3548,  0.3981,  0.4467,  0.5012,  0.5623,  0.6310, 
    0.7079,  0.7943,  0.8913,  1.0000,  1.1220,  1.2589,  1.4125, 
    1.5849,  1.7783,  1.9953,  2.2387,  2.5119,  2.8184,  3.1623, 
    3.5481,  3.9811,  4.4668,  5.0119,  5.6234,  6.3096,  7.0795, 
    7.9433,  8.9125, 10.0000, 11.2202, 12.5893, 14.1254, 15.8489
};

// aml 5.13.02
// -12 dB to +12 dB in 0.5 dB steps
const UInt16 kInputGaindBConvTableOffset = 24;
static float inputGaindBConvTable[] = {
	0.251189,	0.266073,	0.281838,	0.298538,
	0.316228,	0.334965,	0.354813,	0.375837,
	0.398107,	0.421697,	0.446684,	0.473151,
	0.501187,	0.530884,	0.562341,	0.595662,
	0.630957,	0.668344,	0.707946,	0.749894,
	0.794328,	0.841395,	0.891251,	0.944061,
	1.000000,	1.059254,	1.122018,	1.188502,
	1.258925,	1.333521,	1.412538,	1.496236,
	1.584893,	1.678804,	1.778279,	1.883649,
	1.995262,	2.113489,	2.238721,	2.371374,
	2.511886,	2.660725,	2.818383,	2.985383,
	3.162278,	3.349654,	3.548134,	3.758374,
	3.981072
};

#if FLOATLIB
void CoeffsFilterOrder2 (Float32 *Coeff, Float32 CutOffFreq, Float32 AttAtCutOffFreq , Float64 SamplingRate);
#else
Boolean CoeffsFilterOrder2Table (Float32 *Coeff, UInt32 samplingRate);
#endif
Boolean Set4thOrderCoefficients (Float32 *b0, Float32 *b1, Float32 *b2, Float32 *a1, Float32 *a2, UInt32 samplingRate);
Boolean Set4thOrderPhaseCompCoefficients (Float32 *b0, Float32 *b1, Float32 *a1, Float32 *a2, UInt32 samplingRate);
Boolean Set2ndOrderPhaseCompCoefficients (Float32 *b0, Float32 *a1, UInt32 samplingRate);
void MonoFilter (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 samplingRate);
void StereoFilter (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 samplingRate, PreviousValues *theValue);
// aml 2.15.02 added new 4th order stereo filter
void StereoFilter4thOrder (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 samplingRate, PreviousValues *section1State, PreviousValues *section2State);
// aml 2.18.02 added new 4th order stereo filter with phase compensation
void StereoFilter4thOrderPhaseComp (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 samplingRate, PreviousValues *section1State, PreviousValues *section2State, PreviousValues *phaseCompState);

void delayRightChannelInput(float* inFloatBufferPtr, UInt32 numSamples) 
{
    register float* inPtr;
    register float* outPtr;
	register UInt32 i, numFrames;
	register float inSampleR;
	register float oldSample;
	 
    numFrames = numSamples >> 1;
	inPtr = inFloatBufferPtr;
	inPtr++;
	outPtr = inPtr;
	oldSample = gOldInputSample;
	
	for (i = 0; i < numFrames; i++) 
    {
		inSampleR = *inPtr;
		inPtr += 2;
		*outPtr = oldSample;
		outPtr += 2;
		oldSample = inSampleR;
	}
	
	gOldInputSample = oldSample;
}

UInt32 CalculateOffset (UInt64 nanoseconds, UInt32 sampleRate) {
	return ((double)sampleRate / 1000000000.0) * nanoseconds;
}

void dBfixed2float(UInt32 indBfixed, float* ioGainPtr) {
    float out, temp, frac;
    // get integer part
    int index = (SInt16)(indBfixed >> 16);		
    // if we're out of bounds, saturate both integer and fraction		
    if (index >= kMaxZeroGain) {
        index = kMaxZeroGain;
        indBfixed = 0;
    } else if (index <= -kMinZeroGain) {
        index = -kMinZeroGain;
        indBfixed = 0;
    }    
#ifdef _AML_LOG_INPUT_GAIN // aml XXX testing
    IOLog("dBfixed2float - table index 0 = %d\n", index+kZeroGaindBConvTableOffset);    
#endif                    

    // get fractional part
    frac = ((float)((UInt32)(indBfixed & 0x0000FFFF)))/65535.0;		
#ifdef _AML_LOG_INPUT_GAIN // aml XXX testing
    IOLog("dBfixed2float - frac*32767 = %ld\n", (long)(frac*32767));    
#endif                    

    // get the base dB converted value
    out = zeroGaindBConvTable[index + kZeroGaindBConvTableOffset];
    // if we have a fractional part, do linear interpolation on our table
    // this is accurate to about 2 decimal places, which is okay but not great
    if (frac > 0) {
        if (index >= 0) {
#ifdef _AML_LOG_INPUT_GAIN // aml XXX testing
            IOLog("dBfixed2float - table index 1 = %d\n", index+kZeroGaindBConvTableOffset+1);    
#endif                    
            temp = zeroGaindBConvTable[index + kZeroGaindBConvTableOffset + 1];
            out = out + frac*(temp - out);		// out*(1-frac) + temp*frac;
        } else {
#ifdef _AML_LOG_INPUT_GAIN // aml XXX testing
            IOLog("dBfixed2float - table index 1 = %d\n", index+kZeroGaindBConvTableOffset-1);    
#endif                    
            temp = zeroGaindBConvTable[index + kZeroGaindBConvTableOffset - 1];
            out = out + frac*(temp - out);
        }
    }
#ifdef _AML_LOG_INPUT_GAIN // aml XXX testing
    IOLog("dBfixed2float, zero-gain*32767 = %ld\n\n", (long)(out*32767));    
#endif                    

    *ioGainPtr = out;
    
    return;
}

void inputGainConverter(UInt32 inGainIndex, float* ioGainPtr) {
    float out = 1.0;
    // check bounds		
    if (inGainIndex > (2*kInputGaindBConvTableOffset)) {
        inGainIndex = 2*kInputGaindBConvTableOffset;
    }

#ifdef _AML_LOG_INPUT_GAIN 	
//    IOLog("inputGainConverter, inGainIndex = %d\n", inGainIndex);    
#endif                    

    // get the base dB converted value
    out = inputGaindBConvTable[inGainIndex];

#ifdef _AML_LOG_INPUT_GAIN 	
//    IOLog("inputGainConverter, gain*32767 = %ld\n\n", (UInt32)(out*32767));    
#endif                    

    *ioGainPtr = out;
    
    return;
}

IOReturn clipOpenDBDMAToOutputStream(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
/*
    UInt32 sampleIndex, maxSampleIndex;
    float *floatMixBuf;
    SInt16 *outputBuf;
*/
    float	*inFloatBufferPtr;
    SInt16	*outSInt16BufferPtr;
    UInt32	numSamples;
    long i;
    
    int straglers;
     
    float minusOnePointOh = -1.0;
    float maxSInt16 = kMaxSInt16ValueInFloat;
    float maxFloatMinusLSBSInt16 = kMaxFloatMinusLSBSInt16;

    inFloatBufferPtr = (float *)mixBuf+firstSampleFrame*streamFormat->fNumChannels;
    outSInt16BufferPtr = (SInt16 *)sampleBuf+firstSampleFrame * streamFormat->fNumChannels;
    inFloatBufferPtr--;
    outSInt16BufferPtr--;
    numSamples = numSampleFrames*streamFormat->fNumChannels;
    straglers = numSamples % 4;

    for(i = 0; i < numSamples / 4 ; i++) 
	{
		float tempFloat1 = *(++inFloatBufferPtr);
		float tempFloat2 = *(++inFloatBufferPtr);
		float tempFloat3 = *(++inFloatBufferPtr);
		float tempFloat4 = *(++inFloatBufferPtr);

		
		if(tempFloat1 > maxFloatMinusLSBSInt16)
			tempFloat1 = maxFloatMinusLSBSInt16;
		else if(tempFloat1 < minusOnePointOh)
			tempFloat1 = minusOnePointOh;
                        
		if(tempFloat2 > maxFloatMinusLSBSInt16)
			tempFloat2 = maxFloatMinusLSBSInt16;
		else if(tempFloat2 < minusOnePointOh)
			tempFloat2 = minusOnePointOh;
                        
                if(tempFloat3 > maxFloatMinusLSBSInt16)
			tempFloat3 = maxFloatMinusLSBSInt16;
		else if(tempFloat3 < minusOnePointOh)
			tempFloat3 = minusOnePointOh;
                        		
		if(tempFloat4 > maxFloatMinusLSBSInt16)
			tempFloat4 = maxFloatMinusLSBSInt16;
		else if(tempFloat4 < minusOnePointOh)
			tempFloat4 = minusOnePointOh;
                        
                        
		*(++outSInt16BufferPtr) = (short)(tempFloat1*maxSInt16);
		*(++outSInt16BufferPtr) = (short)(tempFloat2*maxSInt16);
		*(++outSInt16BufferPtr) = (short)(tempFloat3*maxSInt16);
		*(++outSInt16BufferPtr) = (short)(tempFloat4*maxSInt16);
		
	}
        
	switch (straglers)
	{
		case 3:
		{
			float tempFloat;
			tempFloat = *(++inFloatBufferPtr);
			
			if(tempFloat > kMaxFloatMinusLSBSInt16)
				tempFloat = kMaxFloatMinusLSBSInt16;
			else if(tempFloat < -1.0)
				tempFloat = -1.0;
		
			*(++outSInt16BufferPtr) = (short)(tempFloat*kMaxSInt16ValueInFloat);
		}
		case 2:
		{	
			float tempFloat;
			tempFloat = *(++inFloatBufferPtr);
		
			if(tempFloat > kMaxFloatMinusLSBSInt16)
				tempFloat = kMaxFloatMinusLSBSInt16;
			else if(tempFloat < -1.0)
				tempFloat = -1.0;
		
			*(++outSInt16BufferPtr) = (short)(tempFloat*kMaxSInt16ValueInFloat);
		}
		case 1:
		{
			float tempFloat;
			tempFloat = *(++inFloatBufferPtr);
		
			if(tempFloat > kMaxFloatMinusLSBSInt16)
				tempFloat = kMaxFloatMinusLSBSInt16;
			else if(tempFloat < -1.0)
				tempFloat = -1.0;
		
			*(++outSInt16BufferPtr) = (short)(tempFloat*kMaxSInt16ValueInFloat);
		}
	}        

    return kIOReturnSuccess;
}

// [3134221] aml
IOReturn clipOpenDBDMAToOutputStreamDelayRight(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{

    UInt32 		sampleIndex, maxSampleIndex;
    float *		floatMixBuf;
    SInt16 *	outputBuf;
    
    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
  
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;
    
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex += 2) 
    {
        float inSampleL, inSampleR;
        
        inSampleL = floatMixBuf[sampleIndex];
		inSampleR = gOldSample;
		gOldSample = floatMixBuf[sampleIndex + 1];
		
        if (inSampleL > 1.0) 
        {
            inSampleL = 1.0;
        }
        else if (inSampleL < -1.0) 
        {
            inSampleL = -1.0;
        }

        if (inSampleR > 1.0) 
        {
            inSampleR = 1.0;
        }
        else if (inSampleR < -1.0) 
        {
            inSampleR = -1.0;
        }
        
		outputBuf[sampleIndex] = (SInt16) (inSampleL * 32767.0);
		outputBuf[sampleIndex + 1] = (SInt16) (inSampleR * 32767.0);
    }

    return kIOReturnSuccess;
}


// [3134221] aml
IOReturn clipOpenDBDMAToOutputStreamDelayRightBalance(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, float* inLeftVolumePtr, float* inRightVolumePtr)
{

    UInt32 		sampleIndex, maxSampleIndex;
    float *		floatMixBuf;
    SInt16 *	outputBuf;
	float		leftVolume, rightVolume;
    
    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
  
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;
    
	leftVolume = *inLeftVolumePtr;
	rightVolume = *inRightVolumePtr;
		
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex += 2) 
    {
        float inSampleL, inSampleR;
        
        inSampleL = floatMixBuf[sampleIndex];
		inSampleR = gOldSample;
		gOldSample = floatMixBuf[sampleIndex + 1];
	
		inSampleL *= leftVolume;	
		inSampleR *= rightVolume;	

        if (inSampleL > 1.0) 
        {
            inSampleL = 1.0;
        }
        else if (inSampleL < -1.0) 
        {
            inSampleL = -1.0;
        }

        if (inSampleR > 1.0) 
        {
            inSampleR = 1.0;
        }
        else if (inSampleR < -1.0) 
        {
            inSampleR = -1.0;
        }
        
		outputBuf[sampleIndex] = (SInt16) (inSampleL * 32767.0f);
		outputBuf[sampleIndex + 1] = (SInt16) (inSampleR * 32767.0f);
    }

    return kIOReturnSuccess;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Clip the data and invert the right channel
// This makes the iMac's speakers back in phase
IOReturn clipOpenDBDMAToOutputStreamInvertRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 		sampleIndex, maxSampleIndex;
    float *		floatMixBuf;
    SInt16 *	outputBuf;
    Boolean 	isTwoChannel;
    
    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
  
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;
    isTwoChannel = (2 == streamFormat->fNumChannels); // save a register indirection in a tight loop
    
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) 
    {
        float inSample;
        
        inSample = floatMixBuf[sampleIndex];
        
        if (inSample > 1.0) 
        {
            inSample = 1.0;
        }
        else if (inSample < -1.0) 
        {
            inSample = -1.0;
        }
        
        outputBuf[sampleIndex] = (SInt16) (inSample * 32767.0);
            
        // if right channel invert it
        if (isTwoChannel && sampleIndex % 2)
        {
             outputBuf[sampleIndex] ^= (SInt16)0xFFFFFFFF;
        }
    }
    
    return kIOReturnSuccess;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Clip the data, mix the right channel to the left, and mute the right channel
IOReturn clipOpenDBDMAToOutputStreamMixRightChannel(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat)
{
    UInt32 		sampleIndex, maxSampleIndex;
    float *		floatMixBuf;
    SInt16 *	outputBuf;
    
    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
  
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;
    
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex += 2) 
    {
        float inSample;
        
		// Mix the left sample with the right sample
        inSample = (floatMixBuf[sampleIndex] + floatMixBuf[sampleIndex + 1]) * 0.5;
		
        if (inSample > 1.0) 
        {
            inSample = 1.0;
        }
        else if (inSample < -1.0) 
        {
            inSample = -1.0;
        }
        
		outputBuf[sampleIndex] = (SInt16) (inSample * 32767.0);

		// Mute the right channel
		outputBuf[sampleIndex + 1] = 0;
    }
    
    return kIOReturnSuccess;
}
// aml 2.14.02 added second filter state for 4th order filter
// aml 2.18.02 added more filter state for phase compensator
// aml 3.5.02 added src phase parameter
IOReturn clipOpenDBDMAToOutputStreamiSub(const void *mixBuf, void *sampleBuf, PreviousValues * filterState, PreviousValues * filterState2, PreviousValues * phaseCompState, Float32 *low, Float32 *high, UInt32 firstSampleFrame, UInt32 numSampleFrames, UInt32 sampleRate, const IOAudioStreamFormat *streamFormat, SInt16 *iSubBufferMemory, UInt32 *loopCount, SInt32 *iSubBufferOffset, UInt32 iSubBufferLen, iSubAudioFormatType* iSubFormat, float* srcPhase, float* srcState, UInt32 adaptiveSampleRate)
{
    UInt32 sampleIndex, maxSampleIndex;
    float *	floatMixBuf;
    SInt16 *	outputBuf;
    float 	highSample;
    Float32	iSubSampleFloat;
    SInt16	iSubSampleInt;
    // aml 3.6.02 storage for src
    float 	x0, x1, temp;

    // aml 3.6.02 src variables - should calculate phaseInc somewhere else, change only if the SR changes
    float 	phaseInc = ((float)adaptiveSampleRate)/((float)(iSubFormat->outputSampleRate));		// phase increment = Fs_in/Fs_out
    float 	phase = *srcPhase;								// current phase location

    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
 
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;

    // Filter out the highs and lows for use with the iSub
    if (1 == streamFormat->fNumChannels) {
		MonoFilter (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate);
    } else if (2 == streamFormat->fNumChannels) {
		// aml 2.15.02 changed to 4th order version
		// aml 2.18.02 changed to 4th order version with phase compensation
//		StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
		StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
    }
    //
    // high side loop 
    //
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
        highSample = high[sampleIndex];
        if (highSample > 1.0) {
            highSample = 1.0;
        } else if (highSample < -1.0) {
            highSample = -1.0;
        }

		outputBuf[sampleIndex] = (SInt16) (highSample * 32767.0);
    }
    
    // aml 2.28.02 added new loop for mixing iSub data to mono
    // increment by two, mix each pair of samples
    // aml 3.1.02 added iSub format check
    //
    // low side loops - currently only handles stereo->stereo, mono->mono, and stereo->mono
    //
    // aml XXX: should move this into an inline function - there's too much code copying!
    if ((iSubFormat->numChannels == 1) && (streamFormat->fNumChannels == 2)) {
        // aml 3.6.02 linear interpolation src (takes the edge of the zoh version, without wasting too many 
        // cycles since we have a 4th order lp in front of us, down -90 dB at Nyquist for 6kHz sample rate)
        // STEREO->MONO
        sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);
        while (sampleIndex < maxSampleIndex) {
                    
            if (phase >= 1.0) {	
                phase -= 1.0;
                sampleIndex+=2;
            } else {   
                // check for beginning of frame case, use saved last sample if needed
                if (sampleIndex == (firstSampleFrame * streamFormat->fNumChannels)) { 
                    x0 = *srcState;
                } else {
                    // mix x[n-1] to mono
                    x0 = low[sampleIndex-2];
                    temp = low[sampleIndex-1];
                    x0 = 0.5*(x0 + temp);
                }
                
                // mix x[n] to mono
                x1 = low[sampleIndex];
                temp = low[sampleIndex+1];
                x1 = 0.5*(x1 + temp);
                                
                // linearly interpolate between x0 and x1
                iSubSampleFloat = x0 + phase*(x1 - x0);
                
                // clip
                if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
                } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
                }
                
                // convert to fixed
                iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);
                
                // check for end of buffer condition
                if (*iSubBufferOffset >= (SInt32)iSubBufferLen) {
                    *iSubBufferOffset = 0;
                    (*loopCount)++;
                }
                
                // byteswap to USB format and copy to iSub buffer
                iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);

                // increment phase and update input buffer pointer
                phase += phaseInc;		
            }
		}
        if (phase < 1) {
            // mix and save last sample in buffer to mono if it will be needed for the next loop
            x1 = low[maxSampleIndex-2];
            temp = low[maxSampleIndex-1];
            *srcState = 0.5*(x1 + temp);
        } else {
            *srcState = 0;
        }
        // cache current phase for use next time we enter the clip loop
       *srcPhase = phase;
    } else {
        // STEREO->STEREO, MONO->MONO
        for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
                    
            iSubSampleFloat = low[sampleIndex];
            if (iSubSampleFloat > 1.0) {
				iSubSampleFloat = 1.0;
            } else if (iSubSampleFloat < -1.0) {
				iSubSampleFloat = -1.0;
            }

            iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);

            if (*iSubBufferOffset >= iSubBufferLen) {
				*iSubBufferOffset = 0;
				(*loopCount)++;
            }

            iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);
        }
    }
    return kIOReturnSuccess;
}

// [3134221] aml
IOReturn clipOpenDBDMAToOutputStreamiSubDelayRight(const void *mixBuf, void *sampleBuf, PreviousValues * filterState, PreviousValues * filterState2, PreviousValues * phaseCompState, Float32 *low, Float32 *high, UInt32 firstSampleFrame, UInt32 numSampleFrames, UInt32 sampleRate, const IOAudioStreamFormat *streamFormat, SInt16 *iSubBufferMemory, UInt32 *loopCount, SInt32 *iSubBufferOffset, UInt32 iSubBufferLen, iSubAudioFormatType* iSubFormat, float* srcPhase, float* srcState, UInt32 adaptiveSampleRate)
{
    UInt32 sampleIndex, maxSampleIndex;
    float *	floatMixBuf;
    SInt16 *	outputBuf;
    Float32	iSubSampleFloat;
    SInt16	iSubSampleInt;
    // aml 3.6.02 storage for src
    float 	x0, x1, temp;

    // aml 3.6.02 src variables - should calculate phaseInc somewhere else, change only if the SR changes
    float 	phaseInc = ((float)adaptiveSampleRate)/((float)(iSubFormat->outputSampleRate));		// phase increment = Fs_in/Fs_out
    float 	phase = *srcPhase;								// current phase location

    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
 
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;

    // Filter out the highs and lows for use with the iSub
    if (1 == streamFormat->fNumChannels) {
		MonoFilter (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate);
    } else if (2 == streamFormat->fNumChannels) {
		// aml 2.15.02 changed to 4th order version
		// aml 2.18.02 changed to 4th order version with phase compensation
		StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
    }
    //
    // high side loop 
    //
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex += 2) {


        float inSampleL, inSampleR;
        
        inSampleL = high[sampleIndex];
		inSampleR = gOldSample;
		gOldSample = high[sampleIndex + 1];
		
        if (inSampleL > 1.0) 
        {
            inSampleL = 1.0;
        }
        else if (inSampleL < -1.0) 
        {
            inSampleL = -1.0;
        }

        if (inSampleR > 1.0) 
        {
            inSampleR = 1.0;
        }
        else if (inSampleR < -1.0) 
        {
            inSampleR = -1.0;
        }
        
		outputBuf[sampleIndex] = (SInt16) (inSampleL * 32767.0);
		outputBuf[sampleIndex + 1] = (SInt16) (inSampleR * 32767.0);
    }
    
    // aml 2.28.02 added new loop for mixing iSub data to mono
    // increment by two, mix each pair of samples
    // aml 3.1.02 added iSub format check
    //
    // low side loops - currently only handles stereo->stereo, mono->mono, and stereo->mono
    //
    // aml XXX: should move this into an inline function - there's too much code copying!
    if ((iSubFormat->numChannels == 1) && (streamFormat->fNumChannels == 2)) {
        // aml 3.6.02 linear interpolation src (takes the edge of the zoh version, without wasting too many 
        // cycles since we have a 4th order lp in front of us, down -90 dB at Nyquist for 6kHz sample rate)
        // STEREO->MONO
        sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);
        while (sampleIndex < maxSampleIndex) {
                    
            if (phase >= 1.0) {	
                phase -= 1.0;
                sampleIndex+=2;
            } else {   
                // check for beginning of frame case, use saved last sample if needed
                if (sampleIndex == (firstSampleFrame * streamFormat->fNumChannels)) { 
                    x0 = *srcState;
                } else {
                    // mix x[n-1] to mono
                    x0 = low[sampleIndex-2];
                    temp = low[sampleIndex-1];
                    x0 = 0.5*(x0 + temp);
                }
                
                // mix x[n] to mono
                x1 = low[sampleIndex];
                temp = low[sampleIndex+1];
                x1 = 0.5*(x1 + temp);
                                
                // linearly interpolate between x0 and x1
                iSubSampleFloat = x0 + phase*(x1 - x0);
                
                // clip
                if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
                } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
                }
                
                // convert to fixed
                iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);
                
                // check for end of buffer condition
                if (*iSubBufferOffset >= (SInt32)iSubBufferLen) {
                    *iSubBufferOffset = 0;
                    (*loopCount)++;
                }
                
                // byteswap to USB format and copy to iSub buffer
                iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);

                // increment phase and update input buffer pointer
                phase += phaseInc;		
            }
		}
        if (phase < 1) {
            // mix and save last sample in buffer to mono if it will be needed for the next loop
            x1 = low[maxSampleIndex-2];
            temp = low[maxSampleIndex-1];
            *srcState = 0.5*(x1 + temp);
        } else {
            *srcState = 0;
        }
        // cache current phase for use next time we enter the clip loop
       *srcPhase = phase;
    } else {
        // STEREO->STEREO, MONO->MONO
        for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
                    
            iSubSampleFloat = low[sampleIndex];
            if (iSubSampleFloat > 1.0) {
				iSubSampleFloat = 1.0;
            } else if (iSubSampleFloat < -1.0) {
				iSubSampleFloat = -1.0;
            }

            iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);

            if (*iSubBufferOffset >= iSubBufferLen) {
				*iSubBufferOffset = 0;
				(*loopCount)++;
            }

            iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);
        }
    }
    return kIOReturnSuccess;
}

IOReturn clipOpenDBDMAToOutputStreamiSubDelayRightBalance(const void *mixBuf, void *sampleBuf, PreviousValues * filterState, PreviousValues * filterState2, PreviousValues * phaseCompState, Float32 *low, Float32 *high, UInt32 firstSampleFrame, UInt32 numSampleFrames, UInt32 sampleRate, const IOAudioStreamFormat *streamFormat, SInt16 *iSubBufferMemory, UInt32 *loopCount, SInt32 *iSubBufferOffset, UInt32 iSubBufferLen, iSubAudioFormatType* iSubFormat, float* srcPhase, float* srcState, UInt32 adaptiveSampleRate, float* inLeftVolumePtr, float* inRightVolumePtr)
{
    UInt32 sampleIndex, maxSampleIndex;
    float *	floatMixBuf;
    SInt16 *	outputBuf;
    Float32		iSubSampleFloat;
    SInt16		iSubSampleInt;
    // aml 3.6.02 storage for src
    float 		x0, x1, temp;
	float		leftVolume, rightVolume;
	float 		phaseInc, phase;
    
	leftVolume = *inLeftVolumePtr;
	rightVolume = *inRightVolumePtr;

    // aml 3.6.02 src variables - should calculate phaseInc somewhere else, change only if the SR changes
    phaseInc = ((float)adaptiveSampleRate)/((float)(iSubFormat->outputSampleRate));		// phase increment = Fs_in/Fs_out
    phase = *srcPhase;								// current phase location

    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
 
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;

    // Filter out the highs and lows for use with the iSub
    if (1 == streamFormat->fNumChannels) {
		MonoFilter (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate);
    } else if (2 == streamFormat->fNumChannels) {
		// aml 2.15.02 changed to 4th order version
		// aml 2.18.02 changed to 4th order version with phase compensation
		StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
    }
    //
    // high side loop 
    //
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex += 2) {

        float inSampleL, inSampleR;
        
        inSampleL = high[sampleIndex];
		inSampleR = gOldSample;
		gOldSample = high[sampleIndex + 1];

		inSampleL *= leftVolume;
		inSampleR *= rightVolume;
		
        if (inSampleL > 1.0) 
        {
            inSampleL = 1.0;
        }
        else if (inSampleL < -1.0) 
        {
            inSampleL = -1.0;
        }

        if (inSampleR > 1.0) 
        {
            inSampleR = 1.0;
        }
        else if (inSampleR < -1.0) 
        {
            inSampleR = -1.0;
        }
 		       
		outputBuf[sampleIndex] = (SInt16) (inSampleL * 32767.0);
		outputBuf[sampleIndex + 1] = (SInt16) (inSampleR * 32767.0);
    }
    
    // aml 2.28.02 added new loop for mixing iSub data to mono
    // increment by two, mix each pair of samples
    // aml 3.1.02 added iSub format check
    //
    // low side loops - currently only handles stereo->stereo, mono->mono, and stereo->mono
    //
    // aml XXX: should move this into an inline function - there's too much code copying!
    if ((iSubFormat->numChannels == 1) && (streamFormat->fNumChannels == 2)) {
        // aml 3.6.02 linear interpolation src (takes the edge of the zoh version, without wasting too many 
        // cycles since we have a 4th order lp in front of us, down -90 dB at Nyquist for 6kHz sample rate)
        // STEREO->MONO
        sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);
        while (sampleIndex < maxSampleIndex) {
                    
            if (phase >= 1.0) {	
                phase -= 1.0;
                sampleIndex+=2;
            } else {   
                // check for beginning of frame case, use saved last sample if needed
                if (sampleIndex == (firstSampleFrame * streamFormat->fNumChannels)) { 
                    x0 = *srcState;
                } else {
                    // mix x[n-1] to mono
                    x0 = low[sampleIndex-2];
                    temp = low[sampleIndex-1];
                    x0 = 0.5*(x0 + temp);
                }
                
                // mix x[n] to mono
                x1 = low[sampleIndex];
                temp = low[sampleIndex+1];
                x1 = 0.5*(x1 + temp);
                                
                // linearly interpolate between x0 and x1
                iSubSampleFloat = x0 + phase*(x1 - x0);
                
                // clip
                if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
                } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
                }
                
                // convert to fixed
                iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);
                
                // check for end of buffer condition
                if (*iSubBufferOffset >= (SInt32)iSubBufferLen) {
                    *iSubBufferOffset = 0;
                    (*loopCount)++;
                }
                
                // byteswap to USB format and copy to iSub buffer
                iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);

                // increment phase and update input buffer pointer
                phase += phaseInc;		
            }
		}
        if (phase < 1) {
            // mix and save last sample in buffer to mono if it will be needed for the next loop
            x1 = low[maxSampleIndex-2];
            temp = low[maxSampleIndex-1];
            *srcState = 0.5*(x1 + temp);
        } else {
            *srcState = 0;
        }
        // cache current phase for use next time we enter the clip loop
       *srcPhase = phase;
    } else {
        // STEREO->STEREO, MONO->MONO
        for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
                    
            iSubSampleFloat = low[sampleIndex];
            if (iSubSampleFloat > 1.0) {
				iSubSampleFloat = 1.0;
            } else if (iSubSampleFloat < -1.0) {
				iSubSampleFloat = -1.0;
            }

            iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);

            if (*iSubBufferOffset >= iSubBufferLen) {
				*iSubBufferOffset = 0;
				(*loopCount)++;
            }

            iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);
        }
    }
    return kIOReturnSuccess;
}

// aml 2.14.02 added second filter state for 4th order filter
// aml 2.18.02 changed to 4th order version with phase compensation
// aml 3.5.02 added src phase parameter
IOReturn clipOpenDBDMAToOutputStreamiSubInvertRightChannel(const void *mixBuf, void *sampleBuf, PreviousValues * filterState, PreviousValues * filterState2, PreviousValues * phaseCompState, Float32 *low, Float32 *high, UInt32 firstSampleFrame, UInt32 numSampleFrames, UInt32 sampleRate, const IOAudioStreamFormat *streamFormat, SInt16 *iSubBufferMemory, UInt32 *loopCount, SInt32 *iSubBufferOffset, UInt32 iSubBufferLen, iSubAudioFormatType* iSubFormat, float* srcPhase, float* srcState, UInt32 adaptiveSampleRate)
{
    UInt32 sampleIndex, maxSampleIndex;
    float *floatMixBuf;
    SInt16 *outputBuf;
	float highSample;
	Float32	iSubSampleFloat;
	SInt16	iSubSampleInt;
    register Boolean isTwoChannel;
    // aml 3.6.02 storage for src
    float 	x0, x1, temp;

    // aml 3.6.02 src variables - should calculate phaseInc somewhere else, change only if the SR changes
    float 	phaseInc = ((float)adaptiveSampleRate)/((float)(iSubFormat->outputSampleRate));		// phase increment = Fs_in/Fs_out
    float 	phase = *srcPhase;								// current phase location

    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
    
    isTwoChannel = (2 == streamFormat->fNumChannels); // save a register indirection in a tight loop
    
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;

    // Filter out the highs and lows for use with the iSub
    if (1 == streamFormat->fNumChannels) {
            MonoFilter (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate);
    } else if (2 == streamFormat->fNumChannels) {
            // aml 2.15.02 changed to 4th order version
            // aml 2.18.02 changed to 4th order version with phase compensation
            StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
    }
    //
    // high side loop
    //
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
        highSample = high[sampleIndex];
        if (highSample > 1.0) {
            highSample = 1.0;
        } else if (highSample < -1.0) {
            highSample = -1.0;
        }

        outputBuf[sampleIndex] = (SInt16) (highSample * 32767.0);
    
        // if right channel then invert it
        if (isTwoChannel && sampleIndex % 2)
        {
            outputBuf[sampleIndex] ^= (SInt16)0xFFFFFFFF;
        }
    }
    
    // aml 2.28.02 added new loop for mixing iSub data to mono
    // increment by two, mix each pair of samples
    // aml 3.1.02 added iSub format check
    //
    // low side loops - currently only handles stereo->stereo, mono->mono, and stereo->mono
    //
    // aml XXX: should move this into an inline function - there's too much code copying!
    if ((iSubFormat->numChannels == 1) && (streamFormat->fNumChannels == 2)) {
        // aml 3.6.02 linear interpolation src (takes the edge of the zoh version, without wasting too many 
        // cycles since we have a 4th order lp in front of us, down -90 dB at Nyquist for 6kHz sample rate)
        // STEREO->MONO
        sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);
        while (sampleIndex < maxSampleIndex) {
                    
            if (phase >= 1.0) {	
                phase -= 1.0;
                sampleIndex+=2;
            } else {   
                // check for beginning of frame case, use saved last sample if needed
                if (sampleIndex == (firstSampleFrame * streamFormat->fNumChannels)) { 
                    x0 = *srcState;
                } else {
                    // mix x[n-1] to mono
                    x0 = low[sampleIndex-2];
                    temp = low[sampleIndex-1];
                    x0 = 0.5*(x0 + temp);
                }
                
                // mix x[n] to mono
                x1 = low[sampleIndex];
                temp = low[sampleIndex+1];
                x1 = 0.5*(x1 + temp);
                                
                // linearly interpolate between x0 and x1
                iSubSampleFloat = x0 + phase*(x1 - x0);
                
                // clip
                if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
                } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
                }
                
                // convert to fixed
                iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);
                
                // check for end of buffer condition
                if (*iSubBufferOffset >= (SInt32)iSubBufferLen) {
					*iSubBufferOffset = 0;
					(*loopCount)++;
                }
                
                // byteswap to USB format and copy to iSub buffer
                iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);

                // increment phase and update input buffer pointer
                phase += phaseInc;		
            }
		}
        if (phase < 1) {
            // mix and save last sample in buffer to mono if it will be needed for the next loop
            x1 = low[maxSampleIndex-2];
            temp = low[maxSampleIndex-1];
            *srcState = 0.5*(x1 + temp);
        } else {
            *srcState = 0;
        }
        // cache current phase for use next time we enter the clip loop
       *srcPhase = phase;
    } else {
        // STEREO->STEREO, MONO->MONO
        for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
                    
            iSubSampleFloat = low[sampleIndex];
            if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
            } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
            }

            iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);

            if (*iSubBufferOffset >= iSubBufferLen) {
				*iSubBufferOffset = 0;
				(*loopCount)++;
            }

            iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);
        }
    }
    return kIOReturnSuccess;
}
// aml 2.14.02 added second filter state for 4th order filter
// aml 2.18.02 changed to 4th order version with phase compensation
// aml 3.5.02 added src phase parameter
IOReturn clipOpenDBDMAToOutputStreamiSubMixRightChannel(const void *mixBuf, void *sampleBuf, PreviousValues * filterState, PreviousValues * filterState2, PreviousValues * phaseCompState, Float32 *low, Float32 *high, UInt32 firstSampleFrame, UInt32 numSampleFrames, UInt32 sampleRate, const IOAudioStreamFormat *streamFormat, SInt16 *iSubBufferMemory, UInt32 *loopCount, SInt32 *iSubBufferOffset, UInt32 iSubBufferLen, iSubAudioFormatType* iSubFormat, float* srcPhase, float* srcState, UInt32 adaptiveSampleRate)
{
    UInt32 sampleIndex, maxSampleIndex;
    float *floatMixBuf;
    SInt16 *outputBuf;
    float highSample;
    Float32	iSubSampleFloat;
    SInt16	iSubSampleInt;
    // aml 3.6.02 storage for src
    float 	x0, x1, temp;

    // aml 3.6.02 src variables - should calculate phaseInc somewhere else, change only if the SR changes
    float 	phaseInc = ((float)adaptiveSampleRate)/((float)(iSubFormat->outputSampleRate));		// phase increment = Fs_in/Fs_out
    float 	phase = *srcPhase;								// current phase location

    floatMixBuf = (float *)mixBuf;
    outputBuf = (SInt16 *)sampleBuf;
    
    maxSampleIndex = (firstSampleFrame + numSampleFrames) * streamFormat->fNumChannels;

    // Filter out the highs and lows for use with the iSub
    if (1 == streamFormat->fNumChannels) {
        MonoFilter (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate);
    } else if (2 == streamFormat->fNumChannels) {
        // aml 2.15.02 changed to 4th order version
        // aml 2.18.02 changed to 4th order version with phase compensation
        StereoFilter4thOrderPhaseComp (&floatMixBuf[firstSampleFrame * streamFormat->fNumChannels], &low[firstSampleFrame * streamFormat->fNumChannels], &high[firstSampleFrame * streamFormat->fNumChannels], numSampleFrames, sampleRate, filterState, filterState2, phaseCompState);
    }
    //
    // high side loop
    //
    for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
        // Mix the left and right channels together
        highSample = (high[sampleIndex] + high[sampleIndex + 1]) * 0.5;
        if (highSample > 1.0) {
            highSample = 1.0;
        } else if (highSample < -1.0) {
            highSample = -1.0;
        }

        // Mute right channel
        if (sampleIndex % 2) {
            outputBuf[sampleIndex] = 0;
        } else {
			outputBuf[sampleIndex] = (SInt16) (highSample * 32767.0);
        }
    }
       
    // aml 2.28.02 added new loop for mixing iSub data to mono
    // increment by two, mix each pair of samples
    // aml 3.1.02 added iSub format check
    //
    // low side loops - currently only handles stereo->stereo, mono->mono, and stereo->mono
    //
    // aml XXX: should move this into an inline function - there's too much code copying!
    if ((iSubFormat->numChannels == 1) && (streamFormat->fNumChannels == 2)) {
        // aml 3.6.02 linear interpolation src (takes the edge of the zoh version, without wasting too many 
        // cycles since we have a 4th order lp in front of us, down -90 dB at Nyquist for 6kHz sample rate)
        // STEREO->MONO
        sampleIndex = (firstSampleFrame * streamFormat->fNumChannels);
        while (sampleIndex < maxSampleIndex) {
                    
            if (phase >= 1.0) {	
                phase -= 1.0;
                sampleIndex+=2;
            } else {   
                // check for beginning of frame case, use saved last sample if needed
                if (sampleIndex == (firstSampleFrame * streamFormat->fNumChannels)) { 
                    x0 = *srcState;
                } else {
                    // mix x[n-1] to mono
                    x0 = low[sampleIndex-2];
                    temp = low[sampleIndex-1];
                    x0 = 0.5*(x0 + temp);
                }
                
                // mix x[n] to mono
                x1 = low[sampleIndex];
                temp = low[sampleIndex+1];
                x1 = 0.5*(x1 + temp);
                                
                // linearly interpolate between x0 and x1
                iSubSampleFloat = x0 + phase*(x1 - x0);
                
                // clip
                if (iSubSampleFloat > 1.0) {
                    iSubSampleFloat = 1.0;
                } else if (iSubSampleFloat < -1.0) {
                    iSubSampleFloat = -1.0;
                }
                
                // convert to fixed
                iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);
                
                // check for end of buffer condition
                if (*iSubBufferOffset >= (SInt32)iSubBufferLen) {
                    *iSubBufferOffset = 0;
                    (*loopCount)++;
                }
                
                // byteswap to USB format and copy to iSub buffer
                iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);

                // increment phase and update input buffer pointer
                phase += phaseInc;		
            }
		}
        if (phase < 1) {
            // mix and save last sample in buffer to mono if it will be needed for the next loop
            x1 = low[maxSampleIndex-2];
            temp = low[maxSampleIndex-1];
            *srcState = 0.5*(x1 + temp);
        } else {
            *srcState = 0;
        }
        // cache current phase for use next time we enter the clip loop
       *srcPhase = phase;
    } else {
        // STEREO->STEREO, MONO->MONO
        for (sampleIndex = (firstSampleFrame * streamFormat->fNumChannels); sampleIndex < maxSampleIndex; sampleIndex++) {
                    
            iSubSampleFloat = low[sampleIndex];
            if (iSubSampleFloat > 1.0) {
                iSubSampleFloat = 1.0;
            } else if (iSubSampleFloat < -1.0) {
				iSubSampleFloat = -1.0;
            }

            iSubSampleInt = (SInt16) (iSubSampleFloat * 32767.0);

            if (*iSubBufferOffset >= iSubBufferLen) {
                *iSubBufferOffset = 0;
                (*loopCount)++;
            }
            iSubBufferMemory[(*iSubBufferOffset)++] = ((((UInt16)iSubSampleInt) << 8) & 0xFF00) | ((((UInt16)iSubSampleInt) >> 8) & 0x00FF);
        }
    }
    return kIOReturnSuccess;
}

// aml 5.10.02, added function
// aml 6.17.02, added dual mono, cleaned up a bit
IOReturn convertOpenDBDMAFromInputStreamWithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, float* inGainLPtr, float* inGainRPtr, DualMonoModeType inDualMonoMode)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf;
	float temp, gainL, gainR;
	
	temp = 0.0;    
	gainL = 1.0;
	gainR = 1.0;

	if (inGainLPtr)
		gainL = *inGainLPtr;
	if (inGainRPtr)
		gainR = *inGainRPtr;
    
    floatDestBuf = (float *)destBuf;
    inputBuf = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
    
	if (streamFormat->fNumChannels == 1) {
		//
		// Mono
		//
		while (numSamplesLeft > 0) {
			
			temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
			*floatDestBuf = gainL*temp;
				
			++inputBuf;
			++floatDestBuf;
			--numSamplesLeft;
		}
	} else {
		if (inDualMonoMode == e_Mode_Disabled) {
			//
			// Stereo
			//
			while (numSamplesLeft > 0) {

				// convert left sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = gainL * temp;
					
				++inputBuf;
				++floatDestBuf;

				// convert right sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = gainR*temp;
					
				++inputBuf;
				++floatDestBuf;
				numSamplesLeft -= 2;
			}
		} else {
			if (inDualMonoMode == e_Mode_CopyLeftToRight) {
				//
				// Dual Mono - Left to Right
				//
				while (numSamplesLeft > 0) {

					// convert left sample
					temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
					*floatDestBuf = gainL * temp;
						
					// skip right sample
					++inputBuf;

					*(++floatDestBuf) = gainR * temp;
						
					++inputBuf;
					++floatDestBuf;
					numSamplesLeft -= 2;
				}
			} else if (inDualMonoMode == e_Mode_CopyRightToLeft) {
				//
				// Dual Mono - Right to Left
				//
				while (numSamplesLeft > 0) {
					
					// skip left sample
					++inputBuf;

					// convert right sample
					temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
					*floatDestBuf = gainL * temp;
					*(++floatDestBuf) = gainR * temp;
						
					++inputBuf;
					++floatDestBuf;
					numSamplesLeft -= 2;
				}
			}	
		}	
	}
        
    return kIOReturnSuccess;
}

IOReturn convertOpenDBDMAFromInputStreamDelayRightWithGain(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, float* inGainLPtr, float* inGainRPtr, DualMonoModeType inDualMonoMode)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf;
	float temp, gainL, gainR;
	
	temp = 0.0;    
	gainL = 1.0;
	gainR = 1.0;

	if (inGainLPtr)
		gainL = *inGainLPtr;
	if (inGainRPtr)
		gainR = *inGainRPtr;
    
    floatDestBuf = (float *)destBuf;
    inputBuf = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;
    
	if (streamFormat->fNumChannels == 1) {
		//
		// Mono
		//
		while (numSamplesLeft > 0) {
			
			temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
			*floatDestBuf = gainL*temp;
				
			++inputBuf;
			++floatDestBuf;
			--numSamplesLeft;
		}
	} else {
		if (inDualMonoMode == e_Mode_Disabled) {
			//
			// Stereo
			//
			while (numSamplesLeft > 0) {

			// convert left sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = gainL * temp;
					
				++inputBuf;
				++floatDestBuf;

				// convert right sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = gainR*temp;
					
				++inputBuf;
				++floatDestBuf;
				numSamplesLeft -= 2;
			}
			
//			delayRightChannelInput ((float *)destBuf, numSampleFrames * streamFormat->fNumChannels);

		} else {
			if (inDualMonoMode == e_Mode_CopyLeftToRight) {
				//
				// Dual Mono - Left to Right
				//
				while (numSamplesLeft > 0) {

					// convert left sample
					temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
					*floatDestBuf = gainL * temp;
						
					// skip right sample
					++inputBuf;

					*(++floatDestBuf) = gainR * temp;
						
					++inputBuf;
					++floatDestBuf;
					numSamplesLeft -= 2;
				}
			} else if (inDualMonoMode == e_Mode_CopyRightToLeft) {
				//
				// Dual Mono - Right to Left
				//
				while (numSamplesLeft > 0) {
					
					// skip left sample
					++inputBuf;

					// convert right sample
					temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
					*floatDestBuf = gainL * temp;
					*(++floatDestBuf) = gainR * temp;
						
					++inputBuf;
					++floatDestBuf;
					numSamplesLeft -= 2;
				}
			}	
		}	
	}
        
    return kIOReturnSuccess;
}

// aml 6.17.02, added dual mono, cleaned up a bit
IOReturn convertOpenDBDMAFromInputStream(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, DualMonoModeType inDualMonoMode)
{
    UInt32 numSamplesLeft;
    float *floatDestBuf;
    SInt16 *inputBuf;
	Float32 temp;
	
    temp = 0;    
    floatDestBuf = (float *)destBuf;
    inputBuf = &(((SInt16 *)sampleBuf)[firstSampleFrame * streamFormat->fNumChannels]);
    
    numSamplesLeft = numSampleFrames * streamFormat->fNumChannels;

	if ((streamFormat->fNumChannels == 2) && (inDualMonoMode != e_Mode_Disabled)) {
		//
		// Dual Mono Mode
		//
		if (inDualMonoMode == e_Mode_CopyLeftToRight) {
			//
			// Dual Mono - Left to Right
			//
			while (numSamplesLeft > 0) {

				// convert left sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = temp;
					
				// skip right sample
				++inputBuf;

				*(++floatDestBuf) = temp;
					
				++inputBuf;
				++floatDestBuf;
				numSamplesLeft -= 2;
			}
		} else if (inDualMonoMode == e_Mode_CopyRightToLeft) {
			//
			// Dual Mono - Right to Left
			//
			while (numSamplesLeft > 0) {
				
				// skip left sample
				++inputBuf;

				// convert right sample
				temp = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				*floatDestBuf = temp;
				*(++floatDestBuf) = temp;
					
				++inputBuf;
				++floatDestBuf;
				numSamplesLeft -= 2;
			}
		}	
    } else {
		while (numSamplesLeft > 0) {
			
			*floatDestBuf = (*inputBuf) * kInvMaxSInt16ValueInFloat;
				
			++inputBuf;
			++floatDestBuf;
			--numSamplesLeft;
		}
	}
	
    return kIOReturnSuccess;
}

#if FLOATLIB
/*
	***CoeffsFilterOrder2***

	This function fills in the order2 filter coefficients table used in the MonoFilter & StereoFilter functions
	Float32  *Coeff			: is a pointer to the coefficients table
	Float32  CutOffFreq		: is the cut off frequency of the filter (in Hertz)
	Float32  AttAtCutOffFreq: is the attenuation at the cut off frequency (in linear)
	Float64  SamplingRate	: is the sampling rate frequency (in Hertz)
*/
void CoeffsFilterOrder2 (Float32 *Coeff, Float32 CutOffFreq, Float32 AttAtCutOffFreq , Float64 SamplingRate)
{
	Float32	k, nu0, pi=3.14159, Att, norm;

	nu0 = (Float32) (CutOffFreq / SamplingRate);
	Att = 1 / AttAtCutOffFreq;
	k = 1/(tan(pi*nu0));
	norm = k*(k+Att)+1;

	/*
	the first 3 coefficients are Num[0], Num[1] & Num[2] in that order
	the last 2 coeffients are Den[1] & Den[2]
	where [.] is the z exposant
	*/
	Coeff[0] = 1.0 / norm;
	Coeff[1] = 2.0 / norm;
	Coeff[2] = 1.0 / norm;
	Coeff[3] = 2*(1-k*k) / norm;
	Coeff[4] = (k*(k-Att)+1) / norm;

	return;
}
#else
/*
    ***CoeffsFilterOrder2Table***

    This function choose an order2 filter coefficients table used in the MonoFilter & StereoFilter functions
    The coefficients table depend on the sampling rate
    Float32  *Coeff		: is a pointer on the coefficients table
    Float64  SamplingRate	: is the sampling rate frequency (in Hertz)
    return: - FALSE if the sampling rate frequency doesn't exist
            - TRUE  otherwise...
*/
Boolean CoeffsFilterOrder2Table (Float32 *Coeff, UInt32 samplingRate)
{
    Boolean 	success = TRUE;

    switch ( samplingRate )
    {
        case 8000:  Coeff[0] =  0.00208054389804601669;
                    Coeff[1] =  0.00416108779609203339;
                    Coeff[2] =  0.00208054389804601669;
                    Coeff[3] = -1.86687481403350830078;
                    Coeff[4] =  0.87519699335098266602;
                    break;
        case 11025: Coeff[0] =  0.00111490569543093443;
                    Coeff[1] =  0.00222981139086186886;
                    Coeff[2] =  0.00111490569543093443;
                    Coeff[3] = -1.90334117412567138672;
                    Coeff[4] =  0.90780085325241088867;
                    break;
        case 22050: Coeff[0] =  0.00028538206242956221;
                    Coeff[1] =  0.00057076412485912442;
                    Coeff[2] =  0.00028538206242956221;
                    Coeff[3] = -1.95164430141448974609;
                    Coeff[4] =  0.95278578996658325195;
                    break;
        case 44100: Coeff[0] =  0.00007220284896902740;
                    Coeff[1] =  0.00014440569793805480;
                    Coeff[2] =  0.00007220284896902740;
                    Coeff[3] = -1.97581851482391357422;
                    Coeff[4] =  0.97610741853713989258;
                    break;
        case 48000: Coeff[0] =  0.00006100598693592474;
                    Coeff[1] =  0.00012201197387184948;
                    Coeff[2] =  0.00006100598693592474;
                    Coeff[3] = -1.97778332233428955078;
                    Coeff[4] =  0.97802722454071044922;
                    break;
        case 96000: Coeff[0] =  0.00001533597242087126;
                    Coeff[1] =  0.00003067194484174252;
                    Coeff[2] =  0.00001533597242087126;
                    Coeff[3] = -1.98889136314392089844;
                    Coeff[4] =  0.98895263671875000000;
                    break;
        default:    // IOLog("\nNot a registered frequency...\n");
                    success = FALSE;
                    break;
    }

    return(success);
}

#endif


// aml 2.14.02 adding fourth order coefficient setting functions
Boolean Set4thOrderCoefficients (Float32 *b0, Float32 *b1, Float32 *b2, Float32 *a1, Float32 *a2, UInt32 samplingRate)
{
    Boolean 	success = TRUE;

    switch ( samplingRate )
    {
        case 8000:  *b0 =  0.00782020803350;
                    *b1 =  0.01564041606699;
                    *b2 =  0.00782020803350;
                    *a1 = -1.73472576880928;
                    *a2 =  0.76600660094326;
                    break;
       case 11025:  *b0 =  0.00425905333005;
                    *b1 =  0.00851810666010;
                    *b2 =  0.00425905333005;
                    *a1 = -1.80709136077571;
                    *a2 =  0.82412757409590;
                    break;
       case 22050:  *b0 =  0.00111491512001;
                    *b1 =  0.00222983024003;
                    *b2 =  0.00111491512001;
                    *a1 = -1.90335434048751;
                    *a2 =  0.90781400096756;
                    break;
        case 44100: *b0 =  0.00028538351548666;
                    *b1 =  0.00057076703097332;
                    *b2 =  0.00028538351548666;
                    *a1 = -1.95165117996464;
                    *a2 =  0.95279271402659;
                    break;
       case 48000:  *b0 =  0.00024135904904198;
                    *b1 =  0.00048271809808396;
                    *b2 =  0.00024135904904198;
                    *a1 = -1.95557824031504;
                    *a2 =  0.95654367651120;
                    break;
        case 96000: *b0 =  0.00006100617875806425;
                    *b1 =  0.0001220123575161285;
                    *b2 =  0.00006100617875806425;
                    *a1 = -1.977786483776763;
                    *a2 =  0.9780305084917958;
                    break;
        default:    // IOLog("\nNot a registered frequency...\n");
                    success = FALSE;
                    break;
    }

    return(success);
}

// aml 2.18.02 adding 4th order phase compensator coefficient setting function
// this function sets the parameters of a second order all-pass filter that is used to compensate for the phase
// shift of the 4th order lowpass IIR filter used in the iSub crossover.  Note that a0 and b2 are both 1.0.
Boolean Set4thOrderPhaseCompCoefficients (Float32 *b0, Float32 *b1, Float32 *a1, Float32 *a2, UInt32 samplingRate)
{
    Boolean 	success = TRUE;

    switch ( samplingRate )
    {
        case 8000:  *a1 = -1.734725768809275;
                    *a2 =  0.7660066009432638;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        case 11025: *a1 = -1.807091360775707;
                    *a2 =  0.8241275740958973;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        case 22050: *a1 = -1.903354340487510;
                    *a2 =  0.9078140009675627;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        case 44100: *a1 = -1.951651179964643;
                    *a2 =  0.9527927140265903;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        case 48000: *a1 = -1.955578240315035;
                    *a2 =  0.9565436765112033;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        case 96000: *a1 = -1.977786483776763;
                    *a2 =  0.9780305084917958;
                    *b0 =  *a2;
                    *b1 =  *a1;
                    break;
        default:    // IOLog("\nNot a registered frequency...\n");
                    success = FALSE;
                    break;
    }

    return(success);
}

// aml 2.18.02 adding 2nd order phase compensator coefficient setting function
// this function sets the parameters of a first order all-pass filter that is used to compensate for the phase
// shift when using a 2nd order lowpass IIR filter for the iSub crossover.  Note that a0 and b1 are both 1.0.
Boolean Set2ndOrderPhaseCompCoefficients (float *b0, float *a1, UInt32 samplingRate)
{
    Boolean 	success = TRUE;

    switch ( samplingRate )
    {
        case 8000:  *a1 = -0.7324848836653277;
                    *b0 =  *a1;
                    break;
        case 11025: *a1 = -0.7985051758519318;
                    *b0 =  *a1;
                    break;
        case 22050: *a1 = -0.8939157008398341;
                    *b0 =  *a1;
                    break;
        case 44100: *a1 = -0.9455137594199962;
                    *b0 =  *a1;
                    break;
        case 48000: *a1 = -0.9498297607998617;
                    *b0 =  *a1;
                    break;
        case 96000: *a1 = -0.9745963490718829;
                    *b0 =  *a1;
                    break;
        default:    // IOLog("\nNot a registered frequency...\n");
                    success = FALSE;
                    break;
    }

    return(success);
}


// aml 2.14.02 notes on the existing filter implementation:
// ***doesn't look like mono filter will work***, it doesn't save any state, and should produce an audible click each time it is called
// inefficiencies in mono, stereo below: 
// 1) accessing filter storage through structure in inner loop
// solution: copy to locals and back around inner loop
// 2) reloading filter coefficients in every call
// solution: create a function to update filter coefficients when needed (eg. sample rate changes)
// 3) accessing coefficients via a structure
// solution: copy to locals before inner loop

/*
	***MonoFilter***

	Mono Order2 Filter
	Float32  *in	: is a pointer to the entry array -> signal to filter...
	Float32  *low	: is a pointer to the low-pass filtered signal
	Float32  *high 	: is a pointer to the high-pass filtered signal
	UInt32   samples: is the number of samples in each array
	Float64  SamplingRate	: is the sampling rate frequency (in Hertz)
	At the n instant: x is x[n], xx is x[n-1], xxx is x[n-2] (it's the same for y)
*/
void MonoFilter (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 samplingRate)
{
	UInt32	idx;
#if !FLOATLIB
	Boolean	success;
#endif
	Float32	LP_Coeff[5];
	Float32	x, xx, xxx, y, yy, yyy;

	// init
#if FLOATLIB
	CoeffsFilterOrder2 (LP_Coeff, 120, 1/sqrt(2), 44100);
#else
	success = CoeffsFilterOrder2Table (LP_Coeff, samplingRate);
    if (success == FALSE) goto End;
#endif
	x=xx=xxx=y=yy=yyy=0;
	// convolution
	for ( idx = 0 ; idx < frames ; idx++ )
	{
		x = in[idx];
		// Low-pass filter
		y = (LP_Coeff[0]*x + LP_Coeff[1]*xx + LP_Coeff[2]*xxx - LP_Coeff[3]*yy - LP_Coeff[4]*yyy);
		// Update
		xxx = xx;
		xx = x;
		yyy = yy;
		yy = y;
		// Storage
		low[idx] = y;
		high[idx] = x-y;
	}

#if !FLOATLIB
End:
#endif
	return;
}

/*
    ***StereoFilter***

    Stereo Order2 Filter
    Float32  *in		: is a pointer on the entry array -> signal to filter...
    Float32  *low		: is a pointer on the low-pass filtered signal
    Float32  *high 		: is a pointer on the high-pass filtered signal
    UInt32   samples		: is the number of samples in each array
    Float64  SamplingRate	: is the sampling rate frequency (in Hertz)
    At the n instant: x is x[n], x_1 is x[n-1], x_2 is x[n-2] (it's the same for y)
*/
void StereoFilter (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 SamplingRate, PreviousValues *theValue)
{
    UInt32	idx;
    Boolean	success;
    Float32	LP_Coeff[5];
    Float32	xl, xr, yl, yr;

    // Get the filter coefficents
    //CoeffsFilterOrder2 (&LP_Coeff, 120, 0.707, SamplingRate); //don't used because of the tan() function
    success = CoeffsFilterOrder2Table (LP_Coeff, SamplingRate);
    if (success == FALSE)  goto End;
    // convolution
    for ( idx = 0 ; idx < frames ; idx ++ )
    {
        xl = in[2*idx];
        xr = in[2*idx+1];
     // Low-pass filter
        yl = (LP_Coeff[0]*xl + LP_Coeff[1]*theValue->xl_1 + LP_Coeff[2]*theValue->xl_2 - LP_Coeff[3]*theValue->yl_1 - LP_Coeff[4]*theValue->yl_2);
        yr = (LP_Coeff[0]*xr + LP_Coeff[1]*theValue->xr_1 + LP_Coeff[2]*theValue->xr_2 - LP_Coeff[3]*theValue->yr_1 - LP_Coeff[4]*theValue->yr_2);
     // Update
        theValue->xl_2 = theValue->xl_1;
        theValue->xr_2 = theValue->xr_1;
        theValue->xl_1 = xl;
        theValue->xr_1 = xr;
        theValue->yl_2 = theValue->yl_1;
        theValue->yr_2 = theValue->yr_1;
        theValue->yl_1 = yl;
        theValue->yr_1 = yr;
     // Storage
        low[2*idx] = yl;
        low[2*idx+1] = yr;
        high[2*idx] = xl-yl;
        high[2*idx+1] = xr-yr;
    }
End:
    return;
}

// aml 2.15.02, stereo 4th order filter, twice the roll off of the 2nd order version
// tried to make this more efficient and readable than previous filter
// ideally all this will move into a class which maintains it's own state and coefficients, etc.
void StereoFilter4thOrder (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 SamplingRate, PreviousValues *theValue, PreviousValues *theValue2)
{
    UInt32	i;
    Float32	inL, inR, outL1, outR1, outL, outR;

    Float32	b0, b1, b2, a1, a2;
    Float32	inLTap1, inLTap2, inRTap1, inRTap2;
    Float32	outLTap1, outLTap2, outRTap1, outRTap2;
    Float32	inLTap1_2, inLTap2_2, inRTap1_2, inRTap2_2;
    Float32	outLTap1_2, outLTap2_2, outRTap1_2, outRTap2_2;

    // copy to local variables to avoid structure referencing during inner loop
    inLTap1 = theValue->xl_1;
    inLTap2 = theValue->xl_2;
    inRTap1 = theValue->xr_1;
    inRTap2 = theValue->xr_2;

    outLTap1 = theValue->yl_1;
    outLTap2 = theValue->yl_2;
    outRTap1 = theValue->yr_1;
    outRTap2 = theValue->yr_2;

    inLTap1_2 = theValue2->xl_1;
    inLTap2_2 = theValue2->xl_2;
    inRTap1_2 = theValue2->xr_1;
    inRTap2_2 = theValue2->xr_2;

    outLTap1_2 = theValue2->yl_1;
    outLTap2_2 = theValue2->yl_2;
    outRTap1_2 = theValue2->yr_1;
    outRTap2_2 = theValue2->yr_2;

    // set all coefficients
    if (Set4thOrderCoefficients (&b0, &b1, &b2, &a1, &a2, SamplingRate) == FALSE)
        return;

    for ( i = 0 ; i < frames ; i ++ )
    {
        inL = in[2*i];
        inR = in[2*i+1];
        // Low-pass filter first pass
        outL1 = (b0*inL + b1*inLTap1 + b2*inLTap2 - a1*outLTap1 - a2*outLTap2);
        outR1 = (b0*inR + b1*inRTap1 + b2*inRTap2 - a1*outRTap1 - a2*outRTap2);
        // update filter taps
        inLTap2 = inLTap1;
        inRTap2 = inRTap1;
        inLTap1 = inL;
        inRTap1 = inR;
        outLTap2 = outLTap1;
        outRTap2 = outRTap1;
        outLTap1 = outL1;
        outRTap1 = outR1;
        // Low-pass filter second pass
        outL = (b0*outL1 + b1*inLTap1_2 + b2*inLTap2_2 - a1*outLTap1_2 - a2*outLTap2_2);
        outR = (b0*outR1 + b1*inRTap1_2 + b2*inRTap2_2 - a1*outRTap1_2 - a2*outRTap2_2);
        // update filter taps
        inLTap2_2 = inLTap1_2;
        inRTap2_2 = inRTap1_2;
        inLTap1_2 = outL1;
        inRTap1_2 = outR1;
        outLTap2_2 = outLTap1_2;
        outRTap2_2 = outRTap1_2;
        outLTap1_2 = outL;
        outRTap1_2 = outR;

        // Storage
        low[2*i] = outL;
        low[2*i+1] = outR;
        high[2*i] = inL-outL;
        high[2*i+1] = inR-outR;
    }

    // update state structures
    theValue->xl_1 = inLTap1;
    theValue->xl_2 = inLTap2;
    theValue->xr_1 = inRTap1;
    theValue->xr_2 = inRTap2;

    theValue->yl_1 = outLTap1;
    theValue->yl_2 = outLTap2;
    theValue->yr_1 = outRTap1;
    theValue->yr_2 = outRTap2;

    theValue2->xl_1 = inLTap1_2;
    theValue2->xl_2 = inLTap2_2;
    theValue2->xr_1 = inRTap1_2;
    theValue2->xr_2 = inRTap2_2;

    theValue2->yl_1 = outLTap1_2;
    theValue2->yl_2 = outLTap2_2;
    theValue2->yr_1 = outRTap1_2;
    theValue2->yr_2 = outRTap2_2;

    return;
}


// aml 2.18.02, stereo 4th order filter with phase compensation
// tried to make this more efficient and readable that previous filter
// ideally all this will move into a class which maintains it's own state and coefficients, etc.
void StereoFilter4thOrderPhaseComp (Float32 *in, Float32 *low, Float32 *high, UInt32 frames, UInt32 SamplingRate, PreviousValues *section1State, PreviousValues *section2State, PreviousValues *phaseCompState)
{
    UInt32	i;
    Float32	inL, inR, outL1, outR1, outL, outR, inPhaseCompL, inPhaseCompR;

    // shared coefficients for second order sections
    Float32	b0, b1, b2, a1, a2;
    // coefficients for phase compensator
    Float32	bp0, bp1, ap1, ap2;
    // taps for second order section 1
    Float32	inLTap1, inLTap2, inRTap1, inRTap2;
    Float32	outLTap1, outLTap2, outRTap1, outRTap2;
    // taps for second order section 2
    Float32	inLTap1_2, inLTap2_2, inRTap1_2, inRTap2_2;
    Float32	outLTap1_2, outLTap2_2, outRTap1_2, outRTap2_2;
    // taps for phase compensator
    Float32	inLTap1_p, inLTap2_p, inRTap1_p, inRTap2_p;
    Float32	outLTap1_p, outLTap2_p, outRTap1_p, outRTap2_p;

    // copy to state local variables to avoid structure referencing during inner loop
    // section 1
    inLTap1 = section1State->xl_1;
    inLTap2 = section1State->xl_2;
    inRTap1 = section1State->xr_1;
    inRTap2 = section1State->xr_2;

    outLTap1 = section1State->yl_1;
    outLTap2 = section1State->yl_2;
    outRTap1 = section1State->yr_1;
    outRTap2 = section1State->yr_2;
    
    // section 2
    inLTap1_2 = section2State->xl_1;
    inLTap2_2 = section2State->xl_2;
    inRTap1_2 = section2State->xr_1;
    inRTap2_2 = section2State->xr_2;

    outLTap1_2 = section2State->yl_1;
    outLTap2_2 = section2State->yl_2;
    outRTap1_2 = section2State->yr_1;
    outRTap2_2 = section2State->yr_2;
    
    // phase compensator
    inLTap1_p = phaseCompState->xl_1;
    inLTap2_p = phaseCompState->xl_2;
    inRTap1_p = phaseCompState->xr_1;
    inRTap2_p = phaseCompState->xr_2;

    outLTap1_p = phaseCompState->yl_1;
    outLTap2_p = phaseCompState->yl_2;
    outRTap1_p = phaseCompState->yr_1;
    outRTap2_p = phaseCompState->yr_2;
 
    // set all coefficients
    if (Set4thOrderCoefficients (&b0, &b1, &b2, &a1, &a2, SamplingRate) == FALSE)
        return;
    if (Set4thOrderPhaseCompCoefficients (&bp0, &bp1, &ap1, &ap2, SamplingRate) == FALSE)
        return;

    // could unroll this loop a bit, but asm optimization is going to improve speed the most
    for ( i = 0 ; i < frames ; i ++ )
    {
        inL = in[2*i];
        inR = in[2*i+1];
        
        // Low-pass filter first pass
        outL1 = b0*inL + b1*inLTap1 + b2*inLTap2 - a1*outLTap1 - a2*outLTap2;
        outR1 = b0*inR + b1*inRTap1 + b2*inRTap2 - a1*outRTap1 - a2*outRTap2;
 
        // update section 1 filter taps
        inLTap2 = inLTap1;
        inRTap2 = inRTap1;
        inLTap1 = inL;
        inRTap1 = inR;
        outLTap2 = outLTap1;
        outRTap2 = outRTap1;
        outLTap1 = outL1;
        outRTap1 = outR1;
        
        // Low-pass filter second pass
        outL = b0*outL1 + b1*inLTap1_2 + b2*inLTap2_2 - a1*outLTap1_2 - a2*outLTap2_2;
        outR = b0*outR1 + b1*inRTap1_2 + b2*inRTap2_2 - a1*outRTap1_2 - a2*outRTap2_2;
        
        // update section 2 filter taps
        inLTap2_2 = inLTap1_2;
        inRTap2_2 = inRTap1_2;
        inLTap1_2 = outL1;
        inRTap1_2 = outR1;
        outLTap2_2 = outLTap1_2;
        outRTap2_2 = outRTap1_2;
        outLTap1_2 = outL;
        outRTap1_2 = outR;

        // phase compensate the input, note that b2 is 1.0
        inPhaseCompL = bp0*inL + bp1*inLTap1_p + inLTap2_p - ap1*outLTap1_p - ap2*outLTap2_p;
        inPhaseCompR = bp0*inR + bp1*inRTap1_p + inRTap2_p - ap1*outRTap1_p - ap2*outRTap2_p;
        
        // update phase compensate filter taps
        inLTap2_p = inLTap1_p;
        inRTap2_p = inRTap1_p;
        inLTap1_p = inL;
        inRTap1_p = inR;
        outLTap2_p = outLTap1_p;
        outRTap2_p = outRTap1_p;
        outLTap1_p = inPhaseCompL;
        outRTap1_p = inPhaseCompR;

        // Storage
        low[2*i] = outL;
        low[2*i+1] = outR;
        high[2*i] = inPhaseCompL-outL;
        high[2*i+1] = inPhaseCompR-outR;
    }

    // update state structures
    
    // section 1 state
    section1State->xl_1 = inLTap1;
    section1State->xl_2 = inLTap2;
    section1State->xr_1 = inRTap1;
    section1State->xr_2 = inRTap2;

    section1State->yl_1 = outLTap1;
    section1State->yl_2 = outLTap2;
    section1State->yr_1 = outRTap1;
    section1State->yr_2 = outRTap2;
    
    // section 2 state
    section2State->xl_1 = inLTap1_2;
    section2State->xl_2 = inLTap2_2;
    section2State->xr_1 = inRTap1_2;
    section2State->xr_2 = inRTap2_2;

    section2State->yl_1 = outLTap1_2;
    section2State->yl_2 = outLTap2_2;
    section2State->yr_1 = outRTap1_2;
    section2State->yr_2 = outRTap2_2;
    
    // phase compensator state
    phaseCompState->xl_1 = inLTap1_p;
    phaseCompState->xl_2 = inLTap2_p;
    phaseCompState->xr_1 = inRTap1_p;
    phaseCompState->xr_2 = inRTap2_p;

    phaseCompState->yl_1 = outLTap1_p;
    phaseCompState->yl_2 = outLTap2_p;
    phaseCompState->yr_1 = outRTap1_p;
    phaseCompState->yr_2 = outRTap2_p;

    return;
}
