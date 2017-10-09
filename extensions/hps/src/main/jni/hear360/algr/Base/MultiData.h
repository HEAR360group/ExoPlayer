//
//  CRFilter.h
//  Convolution Reverb Core
//
//  Created by Richard Zhang on 5/21/15.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#ifndef __MULTI_DATA__
#define __MULTI_DATA__

#define SIMD_OPTI (1)

#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#endif

#ifdef SIMD_OPTI
#include <AK/SoundEngine/Common/AkSimd.h>
#endif

#include <memory>
#include <libs/ckfft/inc/ckfft/ckfft.h>


namespace hear360_algr
{

inline static void CopyMonoSIMD (float *pdstaudio, const float *psrcaudio, long totalsamples)
{
  long framesProduced = 0;
  float zero = 0;

#if defined(__APPLE__)
  vDSP_vsadd(psrcaudio, 1, &zero, pdstaudio, 1, totalsamples);
  return;
#elif SIMD_OPTI
  long framesSIMD = totalsamples - totalsamples % 4;

  float* __restrict pfIn = (float* __restrict) psrcaudio;
  float* __restrict pfOut = (float* __restrict) pdstaudio;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn = AKSIMD_LOAD_V4F32(pfIn);
    AKSIMD_STORE_V4F32(pfOut, vIn);
    pfIn += 4;
    pfOut += 4;
    framesProduced += 4;
  }
#endif

  for (long sx = framesProduced ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrcaudio[sx];
  }
}

inline static void CopyMono (float *pdstaudio, const float *psrcaudio, long totalsamples)
{
  for (long sx = 0 ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrcaudio[sx];
  }

  return;
}

inline static void AddMonoSIMD (float *pdstaudio, const float *psrc1audio, const float *psrc2audio, long totalsamples)
{
  long framesProduced = 0;

  #if defined(__APPLE__)
    vDSP_vadd(psrc1audio, 1, psrc2audio, 1, pdstaudio, 1, totalsamples);
    return;
  #elif SIMD_OPTI
  long framesSIMD = totalsamples - totalsamples % 4;

  float* __restrict pfIn1 = (float* __restrict) psrc1audio;
  float* __restrict pfIn2 = (float* __restrict) psrc2audio;
  float* __restrict pfOut = (float* __restrict) pdstaudio;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn1 = AKSIMD_LOAD_V4F32(pfIn1);
    AKSIMD_V4F32 vIn2 = AKSIMD_LOAD_V4F32(pfIn2);
    AKSIMD_V4F32 vOut = AKSIMD_ADD_V4F32(vIn1, vIn2);
    AKSIMD_STORE_V4F32(pfOut, vOut);
    pfIn1 += 4;
    pfIn2 += 4;
    pfOut += 4;
    framesProduced += 4;
  }
#endif

  for (long sx = framesProduced ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrc1audio[sx] + psrc2audio[sx];
  }

  return;
}

inline static void AddMono (float *pdstaudio, const float *psrc1audio, const float *psrc2audio, long totalsamples)
{
  for (long sx = 0 ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrc1audio[sx] + psrc2audio[sx];
  }

  return;
}

inline static void MixMonoByScalarSIMD(float *pdstaudio, const float *psrcaudio, const float scalar, long totalsamples)
{
  long framesProduced = 0;

#if defined(__APPLE__)
  vDSP_vsma(psrcaudio, 1, &scalar, pdstaudio, 1, pdstaudio, 1, totalsamples);
  return;
#elif SIMD_OPTI
  long framesSIMD = totalsamples - totalsamples % 4;

  const AKSIMD_V4F32 vScalar = AKSIMD_LOAD1_V4F32(scalar);
  float* __restrict pfIn = (float* __restrict) psrcaudio;
  float* __restrict pfOut = (float* __restrict) pdstaudio;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn = AKSIMD_LOAD_V4F32(pfIn);
    AKSIMD_V4F32 vOut = AKSIMD_LOAD_V4F32(pfOut);
    vOut = AKSIMD_MADD_V4F32(vIn, vScalar, vOut);
    AKSIMD_STORE_V4F32(pfOut, vOut);
    pfIn += 4;
    pfOut += 4;
    framesProduced += 4;
  }
#endif

  for (long sx = framesProduced ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] += (psrcaudio[sx] * scalar);
  }
}

inline static void MixMonoByScalar(float *pdstaudio, const float *psrcaudio, const float scalar, long totalsamples)
{
  for (long sx = 0 ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] += (psrcaudio[sx] * scalar);
  }
}

inline static void DivMonoByScalarSIMD (float *pdstaudio, const float *psrcaudio, const float scalar, long totalsamples)
{
  long framesProduced = 0;

#if defined(__APPLE__)
  vDSP_vsdiv(psrcaudio, 1, &scalar, pdstaudio, 1, totalsamples);
  return;
#elif SIMD_OPTI
  #ifndef __ANDROID__
  long framesSIMD = totalsamples - totalsamples % 4;

  const AKSIMD_V4F32 vScalar = AKSIMD_LOAD1_V4F32(scalar);
  float* __restrict pfIn = (float* __restrict) psrcaudio;
  float* __restrict pfOut = (float* __restrict) pdstaudio;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn = AKSIMD_LOAD_V4F32(pfIn);
    AKSIMD_V4F32 vOut = AKSIMD_DIV_V4F32(vIn, vScalar);
    AKSIMD_STORE_V4F32(pfOut, vOut);
    pfIn += 4;
    pfOut += 4;
    framesProduced += 4;
  }
  #endif
#endif

  for (long sx = framesProduced ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrcaudio[sx] / scalar;
  }

  return;
}

inline static void DivMonoByScalar (float *pdstaudio, const float *psrcaudio, const float scalar, long totalsamples)
{
  for (long sx = 0 ; sx < totalsamples ; sx++)
  {
    pdstaudio[sx] = psrcaudio[sx] / scalar;
  }

  return;
}

inline static void ConvolveMonoComplexSIMD(float* pdata1r, float* pdata1i, float* pdata2r, float* pdata2i, float* presultr, float* presulti, unsigned int len)
{
  long framesProduced = 0;

#if defined(__APPLE__)
  vDSP_vmmsb(pdata1r, 1, pdata2r, 1, pdata1i, 1, pdata2i, 1, presultr, 1, len);
  vDSP_vmma(pdata1r, 1, pdata2i, 1, pdata1i, 1, pdata2r, 1, presultr, 1, len);
  return;
#elif SIMD_OPTI
  long framesSIMD = len - len % 4;

  float* __restrict pfResultR = (float* __restrict) presultr;
  float* __restrict pfResultI = (float* __restrict) presulti;
  float* __restrict pfIn1R = (float* __restrict) pdata1r;
  float* __restrict pfIn1I = (float* __restrict) pdata1i;
  float* __restrict pfIn2R = (float* __restrict) pdata2r;
  float* __restrict pfIn2I = (float* __restrict) pdata2i;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn1R = AKSIMD_LOAD_V4F32(pfIn1R);
    AKSIMD_V4F32 vIn1I = AKSIMD_LOAD_V4F32(pfIn1I);
    AKSIMD_V4F32 vIn2R = AKSIMD_LOAD_V4F32(pfIn2R);
    AKSIMD_V4F32 vIn2I = AKSIMD_LOAD_V4F32(pfIn2I);

    AKSIMD_V4F32 vOutR = AKSIMD_SUB_V4F32(AKSIMD_MUL_V4F32(vIn1R, vIn2R), AKSIMD_MUL_V4F32(vIn1I, vIn2I));
    AKSIMD_STORE_V4F32(pfResultR, vOutR);
    AKSIMD_V4F32 vOutI = AKSIMD_ADD_V4F32(AKSIMD_MUL_V4F32(vIn1R, vIn2I), AKSIMD_MUL_V4F32(vIn1I, vIn2R));
    AKSIMD_STORE_V4F32(pfResultI, vOutI);

    pfIn1R += 4;
    pfIn1I += 4;
    pfIn2R += 4;
    pfIn2I += 4;
    pfResultR += 4;
    pfResultI += 4;
    framesProduced += 4;
  }
#endif

  for(unsigned int i = framesProduced; i < len; i++)
  {
    presultr[i] = pdata1r[i] * pdata2r[i] - pdata1i[i] * pdata2i[i];
    presulti[i] = pdata1r[i] * pdata2i[i] + pdata1i[i] * pdata2r[i];
  }
}

inline static void ConvolveMonoComplex(CkFftComplex* pdata1, CkFftComplex* pdata2, CkFftComplex* presult, unsigned int len)
{
  for(unsigned int i = 0; i < len; i++)
  {
    presult[i].real = pdata1[i].real * pdata2[i].real - pdata1[i].imag * pdata2[i].imag;
    presult[i].imag = pdata1[i].real * pdata2[i].imag + pdata1[i].imag * pdata2[i].real;
  }
}

inline static void AddMonoComplexSIMD(CkFftComplex* pdata1, CkFftComplex* pdata2, CkFftComplex* presult, unsigned int len)
{
  unsigned int framesProduced = 0;

#if defined(__APPLE__)
  vDSP_vadd((float*)pdata1, 2, (float*)pdata2, 2, (float*)presult, 2, len);
  vDSP_vadd(((float*)pdata1) + 1, 2, ((float*)pdata2) + 1, 2, ((float*)presult) + 1, 2, len);
  return;
#elif SIMD_OPTI
  unsigned int length = len * 2;
  unsigned int framesSIMD = length - length % 4;

  float* __restrict pfResult = (float* __restrict) presult;
  float* __restrict pfIn1 = (float* __restrict) pdata1;
  float* __restrict pfIn2 = (float* __restrict) pdata2;

  while (framesProduced < framesSIMD)
  {
    AKSIMD_V4F32 vIn1 = AKSIMD_LOAD_V4F32(pfIn1);
    AKSIMD_V4F32 vIn2 = AKSIMD_LOAD_V4F32(pfIn2);

    AKSIMD_V4F32 vOut = AKSIMD_ADD_V4F32(vIn1, vIn2);
    AKSIMD_STORE_V4F32(pfResult, vOut);

    pfIn1 += 4;
    pfIn2 += 4;
    pfResult += 4;
    framesProduced += 4;
  }

  framesProduced = framesProduced / 2;
#endif

  for(unsigned int i = framesProduced; i < len; i++)
  {
    presult[i].real = pdata1[i].real + pdata2[i].real;
    presult[i].imag = pdata1[i].imag + pdata2[i].imag;
  }
}

inline static void AddMonoComplex(CkFftComplex* pdata1, CkFftComplex* pdata2, CkFftComplex* presult, unsigned int len)
{
  for(unsigned int i = 0; i < len; i++)
  {
    presult[i].real = pdata1[i].real + pdata2[i].real;
    presult[i].imag = pdata1[i].imag + pdata2[i].imag;
  }

}

}

#endif
