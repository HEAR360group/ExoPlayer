//
//  CRFilter.h
//  Convolution Reverb Core
//
//  Created by Richard Zhang on 4/4/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#ifndef __CRFILTER_H__
#define __CRFILTER_H__

#include "RTTVector.h"
#include "RingBuffer2.hpp"
//#include <Accelerate/Accelerate.h>
//#include <AudioToolbox/ExtendedAudioFile.h>
#include <hear360/dsp/os/memory.h>
//#include <libs/fftw3.h>
#include <libs/ckfft/inc/ckfft/ckfft.h>

namespace hear360_algr
{

typedef struct DSPSplitComplex
    {
	CkFftComplex* comp;
      //float realp;
      //float imagp;
    } DSPSplitComplex;

class CRFilter
{
public:
	CRFilter(unsigned int bufferLen, hear360_dsp_os_memory::MANAGER memorymanagerparam);
	~CRFilter(void);

    //Loading 24bit PCM Data
    void loadIRs(int* leftIRs, int* rightIRs, unsigned int irFrames);
    void UnloadIRs();
    bool IsIRLoaded();

    unsigned int getTotalIRFrames();
    void setEffectiveIRFrames(unsigned int irFrames);
    void setEffectiveIRLength(float lengthSec);

    void fft_process(float* inputBuf, float* outputLBuf, float* outputRBuf);
private:
	unsigned int mBufferLen;
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
    float mZero;
    float mLen;

	CkFftComplex** mFDLs;

    hear360_dsp_os_memory::MANAGER memorymanager;

	CkFftContext* context;

	CkFftComplex* mSumCompL;
	CkFftComplex* mSumCompR;
	CkFftComplex* mFilteredCompL;
	CkFftComplex* mFilteredCompR;
	CkFftComplex** mFilterCompLs;
	CkFftComplex** mFilterCompRs;
	DSPSplitComplex* mFDLComps;
	CkFftComplex* mTmpComp;

	RingBuffer2<DSPSplitComplex>* mRingCache;
	RingBuffer2Iterator<DSPSplitComplex>* mRingIterator;

    void InitIRsMem(unsigned int irFrames);
    void PrepareIRs();

    inline void fft_filter();
};

} // namespace

#endif
