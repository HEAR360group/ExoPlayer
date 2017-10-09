//
//  CRFilter.h
//  Convolution Reverb Core
//
//  Created by Richard Zhang on 4/4/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#ifndef hear360_dsp_low_convolution_H
#define hear360_dsp_low_convolution_H

//######################################################################################################################

#include "RTTVector.h"
#include "RingBuffer2.hpp"
#include <Accelerate/Accelerate.h>
#include <AudioToolbox/ExtendedAudioFile.h>

//######################################################################################################################

class CRFilter
{
public:
	CRFilter(unsigned int bufferLen, unsigned int samplerate);
	~CRFilter(void);
    
    //Loading 24bit PCM Data
    void loadIRs(int* leftIRs, int* rightIRs, unsigned int irFrames);
    //Loading 16bit PCM Data
    void loadIRs(short* leftIRs, short* rightIRs, unsigned int irFrames);
    //Loading 32bit Float Point PCM Data
    void loadIRs(float* leftIRs, float* rightIRs, unsigned int irFrames);
    
    unsigned int loadIRFromWavFile(const char* urlStr);
    unsigned int loadIRFromWavFile(CFURLRef url);
    
    void UnloadIRs();
    
    void setEffectiveIRFrames(unsigned int irFrames);
    void setEffectiveIRLength(float lengthSec);
    
    void fft_process(float* inputBuf, float* outputLBuf, float* outputRBuf);
    
private:
	unsigned int mBufferLen;
	unsigned int mSampleRate;
	unsigned int mHalfBufferLen;
    unsigned int mDoubleBufferLen;
    unsigned int mLogBufferLen;
	unsigned int mFFTHRTFRatio;
	unsigned int mHRTFSize;
    unsigned int mTotalIrPages;
    unsigned int mEffectiveIrPages;
    unsigned int mLastEffectiveIrPages;
    
    float* mIRFullLeft;
    float* mIRFullRight;
    float** mFilterLefts;
    float** mFilterRights;
    float* mInput;
    float* mOutputLeft;
    float* mOutputRight;
    float* mOutputSumLeft;
    float* mOutputSumRight;
    float* mSumLeft;
    float* mSumRight;
    float mZero;
    float mLen;
    
    float** mFDLs;
    
    float* mFilteredLeft;
    float* mFilteredRight;
    
    FFTSetup mFFTSetup;
    DSPSplitComplex mSumCompL;
    DSPSplitComplex mSumCompR;
    DSPSplitComplex mFilteredCompL;
    DSPSplitComplex mFilteredCompR;
    DSPSplitComplex* mFilterCompLs;
    DSPSplitComplex* mFilterCompRs;
    DSPSplitComplex* mFDLComps;
    
    RingBuffer2<DSPSplitComplex>* mRingCache;
    RingBuffer2Iterator<DSPSplitComplex>* mRingIterator;
    
    void InitIRsMem(unsigned int irFrames);
    void PrepareIRs();
    
	inline void fft_filter();
};

//######################################################################################################################

#endif // include guard

//######################################################################################################################
