//
//  CRFilter.h
//  Convolution Reverb Core
//
//  Created by Richard Zhang on 4/4/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#include "CRFilter.h"
#include <math.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/algr/Base/MultiData.h>

namespace hear360_algr
{

CRFilter::CRFilter(unsigned int bufferLen, hear360_dsp_os_memory::MANAGER memorymanagerparam)
  : mIRFullLeft(NULL)
  , mIRFullRight(NULL)
  , mFilterLefts(NULL)
  , mFilterRights(NULL)
  , memorymanager(memorymanagerparam)
{
    mBufferLen = bufferLen;

    mHalfBufferLen = mBufferLen >> 1;
    mDoubleBufferLen = mBufferLen << 1;
    //mLogBufferLen = 11;
    mLogBufferLen = (unsigned int)(log( (double)mDoubleBufferLen ) / log( 2.0 ));
    //mLogBufferLen = (unsigned int)Log2(mDoubleBufferLen);

    mZero = 0.0f;
    mLen = (float)mDoubleBufferLen;

    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mInput, mDoubleBufferLen * sizeof(float));
    memset(mInput, 0, mDoubleBufferLen * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mOutputLeft, mDoubleBufferLen * sizeof(float));
    memset(mOutputLeft, 0, mDoubleBufferLen * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mOutputRight, mDoubleBufferLen * sizeof(float));
    memset(mOutputRight, 0, mDoubleBufferLen * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mOutputSumLeft, mDoubleBufferLen * sizeof(float));
    memset(mOutputSumLeft, 0, mDoubleBufferLen * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mOutputSumRight, mDoubleBufferLen * sizeof(float));
    memset(mOutputSumRight, 0, mDoubleBufferLen * sizeof(float));

	//CKFFT Initialization
	context = CkFftInit(mDoubleBufferLen, kCkFftDirection_Both, NULL, NULL);
}

CRFilter::~CRFilter(void)
{
  CkFftShutdown(context);
  /*
  if(mFilterLefts != NULL && mFilterRights != NULL)
  {
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterLefts[i]);
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterRights[i]);
    }

    delete[] mFilterLefts;
    delete[] mFilterRights;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterLefts);
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterRights);

    mFilterLefts = NULL;
    mFilterRights = NULL;
  }

  if(mFilterCompLs != NULL && mFilterCompRs != NULL)
  {
    delete[] mFilterCompLs;
    delete[] mFilterCompRs;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompLs);
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompRs);

    mFilterCompLs = NULL;
    mFilterCompRs = NULL;
  }
*/
/*
  if(mInput != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mInput);
  if(mOutputLeft != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputLeft);
  if(mOutputRight != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputRight);

  if(mOutputSumLeft != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputSumLeft);
  if(mOutputSumRight != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputSumRight);

  if(mSumCompL != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mSumCompL);
  if(mSumCompR != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mSumCompR);
  */
}

void CRFilter::InitIRsMem(unsigned int irFrames)
{
  unsigned int irPageMinus1 = irFrames / mBufferLen;
  unsigned int framesLastPage = irFrames % mBufferLen;
  mTotalIrPages = (framesLastPage == 0) ? irPageMinus1 : (irPageMinus1 + 1);
  unsigned int totalFrames = mTotalIrPages * mBufferLen;

  //Chop IRs into small containers
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mIRFullLeft, totalFrames * sizeof(float*));
  memset(mIRFullLeft, 0, totalFrames * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mIRFullRight, totalFrames * sizeof(float*));
  memset(mIRFullRight, 0, totalFrames * sizeof(float));

  mFilterLefts = new float*[mTotalIrPages];
  mFilterRights = new float*[mTotalIrPages];
  //memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterLefts, mTotalIrPages * sizeof(float*));
  //memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterRights, mTotalIrPages * sizeof(float*));

    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterLefts[i], mDoubleBufferLen * sizeof(float*));
        memset(mFilterLefts[i], 0, mDoubleBufferLen * sizeof(float));
        memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterRights[i], mDoubleBufferLen * sizeof(float*));
        memset(mFilterRights[i], 0, mDoubleBufferLen * sizeof(float));
    }

    mFilterCompLs = new CkFftComplex*[mTotalIrPages];
    mFilterCompRs = new CkFftComplex*[mTotalIrPages];
    //memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterCompLs, mTotalIrPages * sizeof(DSPSplitComplex));
    //memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterCompRs, mTotalIrPages * sizeof(DSPSplitComplex));

    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
		memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterCompLs[i], (mDoubleBufferLen + 8) * sizeof(CkFftComplex));
		memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilterCompRs[i], (mDoubleBufferLen + 8) * sizeof(CkFftComplex));
    }
}

void CRFilter::PrepareIRs()
{
	mLen = (float)mDoubleBufferLen * mTotalIrPages;

    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        unsigned int offset = mBufferLen * i;

        memset(mInput, 0, mDoubleBufferLen * sizeof(float));
        hear360_algr::CopyMonoSIMD(mInput, mIRFullLeft + offset, mBufferLen);
		CkFftRealForward(context, mDoubleBufferLen, mInput, mFilterCompLs[i]);

        memset(mInput, 0, mDoubleBufferLen * sizeof(float));
        hear360_algr::CopyMonoSIMD(mInput, mIRFullRight + offset, mBufferLen);
		CkFftRealForward(context, mDoubleBufferLen, mInput, mFilterCompRs[i]);

        memset(mInput, 0, mDoubleBufferLen * sizeof(float));
    }

	//Temp cache for CKFFT
	memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mTmpComp, (mBufferLen + 4) * sizeof(CkFftComplex));

    //FDL Caches
    mFDLs = new CkFftComplex*[mTotalIrPages];
	//memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFDLs, mTotalIrPages * sizeof(CkFftComplex*));
    //mFDLs = new float*[mTotalIrPages];

    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
		memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFDLs[i], (mBufferLen + 4) * sizeof(CkFftComplex));
		memset(mFDLs[i], 0, (mBufferLen + 4) * sizeof(CkFftComplex));
    }


	memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mSumCompL, (mBufferLen + 4) * sizeof(CkFftComplex));
	memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mSumCompR, (mBufferLen + 4) * sizeof(CkFftComplex));

	memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilteredCompL, (mBufferLen + 4) * sizeof(CkFftComplex));
	memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFilteredCompR, (mBufferLen + 4) * sizeof(CkFftComplex));

  mFDLComps = new DSPSplitComplex[mTotalIrPages];
	//memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&mFDLComps, mTotalIrPages * sizeof(DSPSplitComplex));
    for(unsigned int i = 0; i < mTotalIrPages; i++)
	{
		mFDLComps[i].comp = mFDLs[i];
    }

	mRingCache = new RingBuffer2<DSPSplitComplex>(mFDLComps, mTotalIrPages);

	mRingIterator = new RingBuffer2Iterator<DSPSplitComplex>(mRingCache);

    mRingIterator->ReSize(mTotalIrPages);

    mEffectiveIrPages = mTotalIrPages;
    mLastEffectiveIrPages = mTotalIrPages;

    memorymanager.pFree(memorymanager.pmanagerdata, mIRFullLeft);
    memorymanager.pFree(memorymanager.pmanagerdata, mIRFullRight);
}

void CRFilter::loadIRs(int* leftIRs, int* rightIRs, unsigned int irFrames)
{
    InitIRsMem(irFrames);

    //double mTotalIrPagesFloat = (double)mTotalIrPages * 65536.0f;
    double mTotalIrPagesFloat = (double)mTotalIrPages * 8388608.0f;

    for(unsigned int i = 0; i < irFrames; i++)
    {
        mIRFullLeft[i] = (float)((double)leftIRs[i] / mTotalIrPagesFloat);
        mIRFullRight[i] = (float)((double)rightIRs[i] / mTotalIrPagesFloat);
    }

    PrepareIRs();
}

void CRFilter::UnloadIRs()
{
  if(mFilterLefts != NULL || mFilterRights != NULL)
  {
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterLefts[i]);
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterRights[i]);
    }

    delete[] mFilterLefts;
    delete[] mFilterRights;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterLefts);
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterRights);

    mFilterLefts = NULL;
    mFilterRights = NULL;
  }

  if(mFilterCompLs != NULL && mFilterCompRs != NULL)
  {
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompLs[i]);
      memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompRs[i]);
    }

    delete[] mFilterCompLs;
    delete[] mFilterCompRs;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompLs);
    //memorymanager.pFree(memorymanager.pmanagerdata, mFilterCompRs);

    mFilterCompLs = NULL;
    mFilterCompRs = NULL;
  }

    delete mRingCache;
    delete mRingIterator;

  if(mFDLs != NULL)
  {
    for(unsigned int i = 0; i < mTotalIrPages; i++)
    {
        memorymanager.pFree(memorymanager.pmanagerdata, mFDLs[i]);
    }
    delete[] mFDLs;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFDLs);
    mFDLs = NULL;
  }

  if(mFDLComps != NULL)
  {
    delete[] mFDLComps;
    //memorymanager.pFree(memorymanager.pmanagerdata, mFDLComps);
    mFDLComps = NULL;
  }

  if(mInput != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mInput);
  if(mOutputLeft != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputLeft);
  if(mOutputRight != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputRight);

  if(mOutputSumLeft != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputSumLeft);
  if(mOutputSumRight != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mOutputSumRight);

  if(mSumCompL != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mSumCompL);
  if(mSumCompR != NULL)
    memorymanager.pFree(memorymanager.pmanagerdata, mSumCompR);
}

bool CRFilter::IsIRLoaded()
{
  return (mFilterLefts != NULL);
}

unsigned int CRFilter::getTotalIRFrames()
{
  unsigned int totalFrames = mTotalIrPages * mBufferLen;
  return totalFrames;
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

void CRFilter::fft_filter()
{
    if(mEffectiveIrPages != mLastEffectiveIrPages)
    {
        mLastEffectiveIrPages = mEffectiveIrPages;

        mRingIterator->ReSize(mEffectiveIrPages);
    }

	DSPSplitComplex* curCache = mRingIterator->GetCurNodeData();

	memset(curCache->comp, 0, (mBufferLen + 4) * sizeof(CkFftComplex));

	CkFftRealForward(context, mDoubleBufferLen, mInput, curCache->comp);

    memset(mInput, 0, mDoubleBufferLen * sizeof(float));
	memset(mSumCompL, 0, (mBufferLen + 4) * sizeof(CkFftComplex));
	memset(mSumCompR, 0, (mBufferLen + 4) * sizeof(CkFftComplex));

    for(unsigned int i = 0; i < mEffectiveIrPages; i++)
    {
		hear360_algr::ConvolveMonoComplex(curCache->comp, mFilterCompLs[i], mFilteredCompL, mBufferLen + 4);

		hear360_algr::AddMonoComplexSIMD(mFilteredCompL, mSumCompL, mSumCompL, mBufferLen + 4);

		hear360_algr::ConvolveMonoComplex(curCache->comp, mFilterCompRs[i], mFilteredCompR, mBufferLen + 4);

		hear360_algr::AddMonoComplexSIMD(mFilteredCompR, mSumCompR, mSumCompR, mBufferLen + 4);

        mRingIterator->Next();
        curCache = mRingIterator->GetCurNodeData();
    }

	CkFftRealInverse(context, mDoubleBufferLen, mSumCompL, mOutputLeft, mTmpComp);
	CkFftRealInverse(context, mDoubleBufferLen, mSumCompR, mOutputRight, mTmpComp);

    hear360_algr::DivMonoByScalarSIMD(mOutputLeft, mOutputLeft, mLen, mDoubleBufferLen);
    hear360_algr::DivMonoByScalarSIMD(mOutputRight, mOutputRight, mLen, mDoubleBufferLen);

/*
    hear360_algr::CopyMonoSIMD(mOutputLeft, mInput, mDoubleBufferLen);
    hear360_algr::CopyMonoSIMD(mOutputRight, mInput, mDoubleBufferLen);

    hear360_algr::DivMonoByScalarSIMD(mOutputLeft, mOutputLeft, 20, mDoubleBufferLen);
    hear360_algr::DivMonoByScalarSIMD(mOutputRight, mOutputRight, 20, mDoubleBufferLen);
*/
    hear360_algr::AddMonoSIMD(mOutputSumLeft, mOutputSumLeft, mOutputLeft, mDoubleBufferLen);
    hear360_algr::AddMonoSIMD(mOutputSumRight, mOutputSumRight, mOutputRight, mDoubleBufferLen);

    mRingIterator->Prev();
}

void CRFilter::fft_process(float* inputBuf, float* outputLBuf, float* outputRBuf)
{
    memset(mOutputSumLeft + mBufferLen, 0, mBufferLen * sizeof(float));
    memset(mOutputSumRight + mBufferLen, 0, mBufferLen * sizeof(float));

    hear360_algr::CopyMonoSIMD(mInput, inputBuf, mBufferLen);

    fft_filter();

    hear360_algr::AddMonoSIMD(outputLBuf, mOutputSumLeft, outputLBuf, mBufferLen);
    hear360_algr::AddMonoSIMD(outputRBuf, mOutputSumRight, outputRBuf, mBufferLen);

    hear360_algr::CopyMonoSIMD(mOutputSumLeft, mOutputSumLeft + mBufferLen, mBufferLen);
    hear360_algr::CopyMonoSIMD(mOutputSumRight, mOutputSumRight + mBufferLen, mBufferLen);
}

} // namespace
