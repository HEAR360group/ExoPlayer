//
//  Convolution.cpp
//  Convolution Reverb Core
//
//  Created by Richard Zhang on 4/4/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#include "convolution.h"
#include "StereoWavSampler.h"

CRFilter::CRFilter(unsigned int bufferLen, unsigned int samplerate)
{
	mBufferLen = bufferLen;
	mSampleRate = samplerate;
    
	mHalfBufferLen = mBufferLen >> 1;
    mDoubleBufferLen = mBufferLen << 1;
    mLogBufferLen = log2l(mDoubleBufferLen);
    
    mZero = 0.0f;
    mLen = (float)mDoubleBufferLen;
    
    mFilteredLeft = new float[mDoubleBufferLen]();
    mFilteredRight = new float[mDoubleBufferLen]();
    mFilteredCompL.realp = mFilteredLeft;
    mFilteredCompL.imagp = mFilteredLeft + mBufferLen;
    mFilteredCompR.realp = mFilteredRight;
    mFilteredCompR.imagp = mFilteredRight + mBufferLen;
    
    mInput = new float[mBufferLen]();
    mOutputLeft = new float[mDoubleBufferLen]();
    mOutputRight = new float[mDoubleBufferLen]();
    mOutputSumLeft = new float[mDoubleBufferLen]();
    mOutputSumRight = new float[mDoubleBufferLen]();
    
    mSumLeft = new float[mDoubleBufferLen]();
    mSumRight = new float[mDoubleBufferLen]();
    mSumCompL.realp = mSumLeft;
    mSumCompL.imagp = mSumLeft + mBufferLen;
    mSumCompR.realp = mSumRight;
    mSumCompR.imagp = mSumRight + mBufferLen;
    
    mFFTSetup = vDSP_create_fftsetup(mLogBufferLen, FFT_RADIX2);
    
    }

CRFilter::~CRFilter(void)
{
    delete[] mInput;
    delete[] mOutputLeft;
    delete[] mOutputRight;
    
    delete[] mOutputSumLeft;
    delete[] mOutputSumRight;
    
    delete[] mSumLeft;
    delete[] mSumRight;
    
    vDSP_destroy_fftsetup(mFFTSetup);
}

void CRFilter::InitIRsMem(unsigned int irFrames)
{
    unsigned int irPageMinus1 = irFrames / mBufferLen;
    unsigned int framesLastPage = irFrames % mBufferLen;
    mTotalIrPages = (framesLastPage == 0) ? irPageMinus1 : (irPageMinus1 + 1);
    unsigned int totalFrames = mTotalIrPages * mBufferLen;
    
    //Chop IRs into small containers
    mIRFullLeft = new float[totalFrames + mBufferLen]();
    mIRFullRight = new float[totalFrames + mBufferLen]();
    
    mFilterLefts = new float*[mTotalIrPages]();
    mFilterRights = new float*[mTotalIrPages]();
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        mFilterLefts[i] = new float[mDoubleBufferLen]();
        mFilterRights[i] = new float[mDoubleBufferLen]();
    }
    
    mFilterCompLs = new DSPSplitComplex[mTotalIrPages];
    mFilterCompRs = new DSPSplitComplex[mTotalIrPages];
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        mFilterCompLs[i].realp = mFilterLefts[i];
        mFilterCompLs[i].imagp = mFilterLefts[i] + mBufferLen;
        mFilterCompRs[i].realp = mFilterRights[i];
        mFilterCompRs[i].imagp = mFilterRights[i] + mBufferLen;
    }
}

void CRFilter::PrepareIRs()
{
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        vDSP_ctoz((DSPComplex *)(mIRFullLeft + mBufferLen * i), 2, &mFilterCompLs[i], 1, mHalfBufferLen);
        vDSP_ctoz((DSPComplex *)(mIRFullRight + mBufferLen * i), 2, &mFilterCompRs[i], 1, mHalfBufferLen);
        
        vDSP_fft_zrip( mFFTSetup, &mFilterCompLs[i], 1, mLogBufferLen, FFT_FORWARD);
        vDSP_fft_zrip( mFFTSetup, &mFilterCompRs[i], 1, mLogBufferLen, FFT_FORWARD);
    }
    
    //FDL Caches
    mFDLs = new float*[mTotalIrPages];
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        mFDLs[i] = new float[mDoubleBufferLen]();
    }
    
    mFDLComps = new DSPSplitComplex[mTotalIrPages];
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        mFDLComps[i].realp = mFDLs[i];
        mFDLComps[i].imagp = mFDLs[i] + mBufferLen;
    }
    
    mRingCache = new RingBuffer2<DSPSplitComplex>(mFDLComps, mTotalIrPages);

    mRingIterator = new RingBuffer2Iterator<DSPSplitComplex>(mRingCache);
    
    mRingIterator->ReSize(mTotalIrPages);
    
    mEffectiveIrPages = mTotalIrPages;
    mLastEffectiveIrPages = mTotalIrPages;
    
    delete[] mIRFullLeft;
    delete[] mIRFullRight;
}

void CRFilter::loadIRs(int* leftIRs, int* rightIRs, unsigned int irFrames)
{
    InitIRsMem(irFrames);
    
    float mTotalIrPagesFloat = (float)mTotalIrPages * 8388608.0f;
    
    vDSP_vflt32(leftIRs, 1, mIRFullLeft, 1, irFrames);
    vDSP_vflt32(rightIRs, 1, mIRFullRight, 1, irFrames);
    
    vDSP_vsdiv(mIRFullLeft, 1, &mTotalIrPagesFloat, mIRFullLeft, 1, irFrames);
    vDSP_vsdiv(mIRFullRight, 1, &mTotalIrPagesFloat, mIRFullRight, 1, irFrames);
    
    PrepareIRs();
}

void CRFilter::loadIRs(short* leftIRs, short* rightIRs, unsigned int irFrames)
{
    InitIRsMem(irFrames);
    
    float mTotalIrPagesFloat = (float)mTotalIrPages * 32768.0f;
    
    vDSP_vflt16(leftIRs, 1, mIRFullLeft, 1, irFrames);
    vDSP_vflt16(rightIRs, 1, mIRFullRight, 1, irFrames);
    
    vDSP_vsdiv(mIRFullLeft, 1, &mTotalIrPagesFloat, mIRFullLeft, 1, irFrames);
    vDSP_vsdiv(mIRFullRight, 1, &mTotalIrPagesFloat, mIRFullRight, 1, irFrames);
    
    PrepareIRs();
}

void CRFilter::loadIRs(float* leftIRs, float* rightIRs, unsigned int irFrames)
{
    InitIRsMem(irFrames);
    
    vDSP_vsadd(leftIRs, 1, &mZero, mIRFullLeft, 1, irFrames);
    vDSP_vsadd(rightIRs, 1, &mZero, mIRFullRight, 1, irFrames);
    
    PrepareIRs();
}

unsigned int CRFilter::loadIRFromWavFile(const char* urlStr)
{
    CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), __CFStringMakeConstantString(urlStr), CFSTR("wav"), NULL);
    
    return loadIRFromWavFile(url);
}

unsigned int CRFilter::loadIRFromWavFile(CFURLRef url)
{
    StereoWavSampler sampler;
    unsigned int len = sampler.TouchWavForLen(url);
    
    InitIRsMem(len);
    
    sampler.LoadWav(url, mIRFullLeft, mIRFullRight);
    
    PrepareIRs();
    
    return len;
}

void CRFilter::UnloadIRs()
{
    delete[] mIRFullLeft;
    delete[] mIRFullRight;
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        delete mFilterLefts[i];
        delete mFilterRights[i];
    }
    
    delete[] mFilterLefts;
    delete[] mFilterRights;
    
    delete[] mFilterCompLs;
    delete[] mFilterCompRs;
    
    delete mRingCache;
    delete mRingIterator;
    
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        delete mFDLs[i];
    }
    
    delete[] mFDLs;
}

void CRFilter::setEffectiveIRFrames(unsigned int irFrames)
{
    unsigned int totalFrames = mTotalIrPages * mBufferLen;
    if(irFrames <= totalFrames)
    {
        //Snap to the IR block size
        unsigned int irPageMinus1 = irFrames / mBufferLen;
        unsigned int framesLastPage = irFrames % mBufferLen;
        mEffectiveIrPages = (framesLastPage == 0) ? irPageMinus1 : (irPageMinus1 + 1);
    }
}

void CRFilter::setEffectiveIRLength(float lengthSec)
{
    setEffectiveIRFrames(lengthSec * mSampleRate);
}

void CRFilter::fft_filter()
{
    if(mEffectiveIrPages != mLastEffectiveIrPages)
    {
        mLastEffectiveIrPages = mEffectiveIrPages;
        
        mRingIterator->ReSize(mEffectiveIrPages);
    }
    
    DSPSplitComplex* curCache = mRingIterator->GetCurNodeData();
    
    vDSP_vclr(curCache->realp, 1, mBufferLen);
    vDSP_vclr(curCache->imagp, 1, mBufferLen);
    
    vDSP_ctoz((DSPComplex *) mInput, 2, curCache, 1, mHalfBufferLen);
    
    vDSP_fft_zrip( mFFTSetup, curCache, 1, mLogBufferLen, FFT_FORWARD);
    
    vDSP_vclr(mSumCompL.realp, 1, mBufferLen);
    vDSP_vclr(mSumCompL.imagp, 1, mBufferLen);
    vDSP_vclr(mSumCompR.realp, 1, mBufferLen);
    vDSP_vclr(mSumCompR.imagp, 1, mBufferLen);
    
    for(unsigned int i = 0; i < mEffectiveIrPages; i++)
    {
        vDSP_zvmul(curCache, 1, &mFilterCompLs[i], 1, &mFilteredCompL, 1, mBufferLen, 1);
        
        vDSP_zvadd(&mFilteredCompL, 1, &mSumCompL, 1, &mSumCompL, 1, mBufferLen);
        
        vDSP_zvmul(curCache, 1, &mFilterCompRs[i], 1, &mFilteredCompR, 1, mBufferLen, 1);
        
        vDSP_zvadd(&mFilteredCompR, 1, &mSumCompR, 1, &mSumCompR, 1, mBufferLen);
        
        mRingIterator->Next();
        curCache = mRingIterator->GetCurNodeData();
    }
    
    vDSP_fft_zrip( mFFTSetup, &mSumCompL, 1, mLogBufferLen, FFT_INVERSE);
    vDSP_fft_zrip( mFFTSetup, &mSumCompR, 1, mLogBufferLen, FFT_INVERSE);
    
    vDSP_ztoc(&mSumCompL, 1, (DSPComplex *) mOutputLeft, 2, mBufferLen);
    vDSP_ztoc(&mSumCompR, 1, (DSPComplex *) mOutputRight, 2, mBufferLen);
    
    vDSP_vsdiv(mOutputLeft, 1, &mLen, mOutputLeft, 1, mDoubleBufferLen);
    vDSP_vsdiv(mOutputRight, 1, &mLen, mOutputRight, 1, mDoubleBufferLen);
    
    vDSP_vadd(mOutputLeft, 1, mOutputSumLeft, 1, mOutputSumLeft, 1, mDoubleBufferLen);
    vDSP_vadd(mOutputRight, 1, mOutputSumRight, 1, mOutputSumRight, 1, mDoubleBufferLen);
    
    mRingIterator->Prev();
}

void CRFilter::fft_process(float* inputBuf, float* outputLBuf, float* outputRBuf)
{
    vDSP_vclr(mOutputSumLeft + mBufferLen, 1, mBufferLen);
    vDSP_vclr(mOutputSumRight + mBufferLen, 1, mBufferLen);
    
    vDSP_vsadd(inputBuf, 1, &mZero, mInput, 1, mBufferLen);
    
    fft_filter();

    vDSP_vsadd(mOutputSumLeft, 1, &mZero, outputLBuf, 1, mBufferLen);
    vDSP_vsadd(mOutputSumRight, 1, &mZero, outputRBuf, 1, mBufferLen);
    
    vDSP_vswap(mOutputSumLeft + mBufferLen, 1, mOutputSumLeft, 1, mBufferLen);
    vDSP_vswap(mOutputSumRight + mBufferLen, 1, mOutputSumRight, 1, mBufferLen);
     
}