/*
 *  DSPUtils.h
 *  AstoundAPI
 *
 *  Copyright 2016 Hear360, Inc. All rights reserved.
 */

#ifndef __DSP_UTILS__
#define __DSP_UTILS__

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/*
#if (defined (_MSC_VER) || defined (ANDROID))

  inline float log2( float n )  
  {  
    return log( n ) / log( 2.0 );  
  }

#endif
  */
//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

	float **DestroyBus(float **data, long numChannels);
	float **CreateBus(long numChannels, long numFrames);

/** ***************************************************************************************
 */
void MergeBus(float **data1, float fac1, float **data2, float fac2, long numChannels, long numFrames);

/** ***************************************************************************************
 */
void MergeBus2(float **data1, float **data2, long numChannels, long numFrames);

/** ***************************************************************************************
 */
void AddSrcBusToDestBusWithGain(float **destBus, float **srcBus, float gainFac, long numChannels, long numFrames);

/** ***************************************************************************************
 */
void CloneChannelData(float **data, long channel, long numChannels, long numFrames);

/** ***************************************************************************************
 */
void CopyBusDataSubtractingBFromA(float **destination, float **dataA, float **dataB, long numChannels, long numFrames);

/** ***************************************************************************************
 */
void ZeroBusData(register float **destination, register long numChannels, register long numFrames);

/** ***************************************************************************************
 */
void CopyBusDataWithGain(float **destination, float **data, register long numChannels, register long numFrames, register float gain);

/** ***************************************************************************************
 */
void CopyBusData(register float **destination, register float **data, register long numChannels, register long numFrames);

/** ***************************************************************************************
 */
void MergeStereoWithOutputsReversed(register float **dest, register float **source, register long numFrames);

/** ***************************************************************************************
 */
void MergeStereo(register float **dest, register float **source, register long numFrames);

/** ***************************************************************************************
 */
float gainDbToFactor(float gain);

/** ***************************************************************************************
 */
float gainFactorToDb(float fac);

/** ***************************************************************************************
 */
float limit(float value, float min, float max);

/** ***************************************************************************************
 */
enum eSampleRateIndex
{
    eSampleRateUnsupported=-1,
    eSampleRate441k=0,
    eSampleRate48k=1,
    eSampleRate882k=2,
    eSampleRate96k=3,
    eSampleRate176k=4,
    eSampleRate192k=5
};

/** ***************************************************************************************
    sample rate enumeration with max pull up taken into consideration
 */
enum eMaxSampleRate // 
{
    eMaxSampleRate441k=46000,
    eMaxSampleRate48k=51000,
    eMaxSampleRate882k=92000,
    eMaxSampleRate96k=102000,
    eMaxSampleRate176k=184000,
    eMaxSampleRate192k=204000
};

/** ***************************************************************************************
 */
int GetSampleRateIndex(int iSampleRate);

/** ***************************************************************************************
 */
void mixStereoLocalizationResults(float ** destBus, float ** locBusL, float ** locBusR, unsigned long nFrames);

/** ***************************************************************************************
 */
void mixStereoPair(float ** destBus, float ** srcBus, register float gainFac1, register float gainFac2, unsigned long nFrames);

/** ***************************************************************************************
 */
void gainStereoPair(float ** destBus, register float gainFac, unsigned long nFrames);

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif /* __DSP_UTILS__ */

