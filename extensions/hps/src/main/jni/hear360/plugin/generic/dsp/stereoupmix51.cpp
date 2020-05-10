
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/dsp/os/memory.h>
#include <hear360/plugin/generic/dsp/vsheadtracking.h>
#include <hear360/algr/Base/MultiData.h>
#include <hear360/algr/Equalizer/IIRFilterFirstOrder.h>
//#include <hear360/algr/Equalizer/HPSEqualizerBand.h>
#include <hear360/algr/Delay/HPSStaticDelay.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_stereoupmix51
{

#define LOG (0)

#if (LOG)
  #define LOGFILE "d:\\generichrirfolddown.txt"
#endif

#if (LOG)
  #include <cstdio>
#endif

//######################################################################################################################

struct BUFFER
{
  float* leftInput;
  float* rightInput;

  float* leftSampleA;
  float* rightSampleA;

  float* leftSampleB;
  float* rightSampleB;

  float* leftSampleC;
  float* rightSampleC;

  float* lsrSample;
  float* rsrSample;

  float* centerSample;
};

//######################################################################################################################

struct PRIVATE
{
  BUFFER buffer;

  hear360_dsp_os_memory::MANAGER memorymanager;

  //channel numbers
  const int leftChannelNum = 0;
  const int rightChannelNum = 1;
  const int centerChannelNum = 2;
  const int lfeChannelNum = 3;
  const int lsrChannelNum = 4;
  const int rsrChannelNum = 5;

  //filter for each output channel
  hear360_algr::IIRFilterFirstOrder* centerHighPass;

  hear360_algr::IIRFilterFirstOrder* lsrHighPass;
  hear360_algr::IIRFilterFirstOrder* rsrHighPass;

  hear360_algr::IIRFilterFirstOrder* leftLowPass;
  hear360_algr::IIRFilterFirstOrder* rightLowPass;
  // hear360_algr::HPSEqualizerBand* centerHighPass;
  //
  // hear360_algr::HPSEqualizerBand* lsrHighPass;
  // hear360_algr::HPSEqualizerBand* rsrHighPass;
  //
  // hear360_algr::HPSEqualizerBand* leftLowPass;
  // hear360_algr::HPSEqualizerBand* rightLowPass;

  //constants converted from decibel into gain (voltage)
  const float negSixDB = 0.501187f;
  const float negThreeDB = 0.707946f;
  const float negFourteenDB = 0.199526f;
  const float negSevenDB = 0.446684f;
  const float negOne = -1.0f;

  //circular buffers for delays
  HPSStaticDelay* lsrDDL;
  HPSStaticDelay* rsrDDL;

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
  ~PRIVATE();
};

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate)
: memorymanager (memorymanagerparam)
, centerHighPass (new hear360_algr::IIRFilterFirstOrder())
, lsrHighPass (new hear360_algr::IIRFilterFirstOrder())
, rsrHighPass (new hear360_algr::IIRFilterFirstOrder())
, leftLowPass (new hear360_algr::IIRFilterFirstOrder())
, rightLowPass (new hear360_algr::IIRFilterFirstOrder())
// , centerHighPass (new hear360_algr::HPSEqualizerBand(samplerate))
// , lsrHighPass (new hear360_algr::HPSEqualizerBand(samplerate))
// , rsrHighPass (new hear360_algr::HPSEqualizerBand(samplerate))
// , leftLowPass (new hear360_algr::HPSEqualizerBand(samplerate))
// , rightLowPass (new hear360_algr::HPSEqualizerBand(samplerate))
, lsrDDL (new HPSStaticDelay(samplerate, 0.02f))
, rsrDDL (new HPSStaticDelay(samplerate, 0.03f))
{
  //initialize filters
  centerHighPass->reset();

  lsrHighPass->reset();
  rsrHighPass->reset();

  leftLowPass->reset();
  rightLowPass->reset();

  //create coeffs
  //NOTE: make slope 6dB/oct
  hear360_algr::IIRCoefficientsSinglePole centerHPFCoeffs = hear360_algr::IIRCoefficientsSinglePole::makeHighPass(samplerate, 200.0f);

  hear360_algr::IIRCoefficientsSinglePole rearHPFCoeffs = hear360_algr::IIRCoefficientsSinglePole::makeHighPass(samplerate, 871.6f);

  hear360_algr::IIRCoefficientsSinglePole frontLPFCoeffs = hear360_algr::IIRCoefficientsSinglePole::makeLowPass(samplerate, 200.0f);

  //initialize filters
  centerHighPass->setCoefficients(centerHPFCoeffs);

  lsrHighPass->setCoefficients(rearHPFCoeffs);
  rsrHighPass->setCoefficients(rearHPFCoeffs);

  leftLowPass->setCoefficients(frontLPFCoeffs);
  rightLowPass->setCoefficients(frontLPFCoeffs);

  // leftLowPass->Reset();
  // leftLowPass->SetLowpass(200.0f, 0.0f, 1.0f / sqrt(2.0f));
  // rightLowPass->Reset();
  // rightLowPass->SetLowpass(200.0f, 0.0f, 1.0f / sqrt(2.0f));
  //
  // centerHighPass->Reset();
  // centerHighPass->SetHighpass(200.0f, 0.0f, 1.0f / sqrt(2.0f));
  //
  // lsrHighPass->Reset();
  // lsrHighPass->SetHighpass(871.6f, 0.0f, 1.0f / sqrt(2.0f));
  // rsrHighPass->Reset();
  // rsrHighPass->SetHighpass(871.6f, 0.0f, 1.0f / sqrt(2.0f));

  //this sets the size of (and variables for) the circular buffers used for delays
  lsrDDL->SetDelaySamples(samplerate * 0.02f);
  rsrDDL->SetDelaySamples(samplerate * 0.03f);

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.leftInput, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rightInput, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.leftSampleA, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rightSampleA, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.leftSampleB, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rightSampleB, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.leftSampleC, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rightSampleC, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.lsrSample, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rsrSample, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.centerSample, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));

  memset(buffer.leftInput, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rightInput, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.leftSampleA, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rightSampleA, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.leftSampleB, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rightSampleB, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.leftSampleC, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rightSampleC, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.lsrSample, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rsrSample, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.centerSample, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
}

//######################################################################################################################

PRIVATE::~PRIVATE ()
{
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.leftInput);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rightInput);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.leftSampleA);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rightSampleA);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.leftSampleB);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rightSampleB);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.leftSampleC);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rightSampleC);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.lsrSample);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rsrSample);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.centerSample);

  delete rsrDDL;
  delete lsrDDL;

  delete rightLowPass;
  delete leftLowPass;

  delete rsrHighPass;
  delete lsrHighPass;

  delete centerHighPass;
}

//######################################################################################################################

struct GLOBALDATA
{
  //unsigned int nextID;
  //std::map<unsigned int, PRIVATE*> hrirfolddownInstances;

  #if (LOG)
  FILE *pfile;
  #endif

  GLOBALDATA ();
  ~GLOBALDATA();
};

static GLOBALDATA gdata;

GLOBALDATA::GLOBALDATA()
{
  #if (LOG)

  gdata.pfile = std::fopen (LOGFILE, "a");

  std::fprintf (gdata.pfile, "Opening log.\n\n");

  std::fflush (gdata.pfile);

  #endif
}

GLOBALDATA::~GLOBALDATA()
{
  #if (LOG)

  std::fprintf (gdata.pfile, "Closing log.\n\n");

  std::fclose (gdata.pfile);

  #endif
}

//######################################################################################################################

void* CreateInstance(int samplerate)
{
  PRIVATE *pprivate;

  pprivate = new PRIVATE(hear360_dsp_os_memory::MANAGER(), samplerate);

  return (void*)pprivate;
}

bool DeleteInstance(void* handle)
{
  if (handle == NULL)
    return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
    return false;

  delete pprivate;
  handle = NULL;

  return true;
}

//######################################################################################################################

bool ProcessOutOfPlaceInterleaved(void* handle, const float* pInBuf, float* pOutBuf, int srcChannels, int dstChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	return false;

  if (pOutBuf == NULL)
	  return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	  return false;

  hear360_algr::CopyMonoWithStride(pprivate->buffer.leftInput, pInBuf, totalsamples, 1, srcChannels, 0, pprivate->leftChannelNum);
  hear360_algr::CopyMonoWithStride(pprivate->buffer.rightInput, pInBuf, totalsamples, 1, srcChannels, 0, pprivate->rightChannelNum);

  //PATH 1: C
  // centerSample = (leftReader[i] + rightReader[i]) * negThreeDB;
  hear360_algr::AddMonoSIMD(pprivate->buffer.centerSample, pprivate->buffer.leftInput, pprivate->buffer.rightInput, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.centerSample, pprivate->buffer.centerSample, pprivate->negThreeDB, totalsamples);

  // centerSample = centerHighPass.processSingleSampleRaw(centerSample);
  pprivate->centerHighPass->processInPlace(pprivate->buffer.centerSample, totalsamples);
  // pprivate->centerHighPass->Process(pprivate->buffer.centerSample, totalsamples);

  //PATH 2: L + R
  // subpath A
  // leftSampleA = leftReader[i] * negFourteenDB;
  // rightSampleA = rightReader[i] * negFourteenDB;
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftInput, pprivate->negFourteenDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightInput, pprivate->negFourteenDB, totalsamples);

  //subpath B
  // leftSampleB = leftReader[i] * negSevenDB;
  // rightSampleB = rightReader[i] * negSevenDB;
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleB, pprivate->buffer.leftInput, pprivate->negSevenDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleB, pprivate->buffer.rightInput, pprivate->negSevenDB, totalsamples);

  // leftSampleB = leftLowPass.processSingleSampleRaw(leftSampleB);
  // rightSampleB = rightLowPass.processSingleSampleRaw(rightSampleB);
  pprivate->leftLowPass->processInPlace(pprivate->buffer.leftSampleB, totalsamples);
  pprivate->rightLowPass->processInPlace(pprivate->buffer.rightSampleB, totalsamples);
  // pprivate->leftLowPass->Process(pprivate->buffer.leftSampleB, totalsamples);
  // pprivate->rightLowPass->Process(pprivate->buffer.rightSampleB, totalsamples);

  //sum A + B
  // leftSampleA += leftSampleB;
  // rightSampleA += rightSampleB;
  hear360_algr::AddMonoSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleB, totalsamples);
  hear360_algr::AddMonoSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleB, totalsamples);

  //subpath C
  // leftSampleC = leftReader[i] - rightReader[i];
  // leftSampleC *= negSixDB;
  // rightSampleC = leftSampleC * -1.f;
  hear360_algr::SubMonoSIMD(pprivate->buffer.leftSampleC, pprivate->buffer.leftInput, pprivate->buffer.rightInput, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleC, pprivate->buffer.leftSampleC, pprivate->negSixDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleC, pprivate->buffer.leftSampleC, pprivate->negOne, totalsamples);

  //sum path (A+B) w/ C
  // leftSampleA += leftSampleC;
  // rightSampleA += rightSampleC;
  hear360_algr::AddMonoSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleC, totalsamples);
  hear360_algr::AddMonoSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleC, totalsamples);

  //PATH 3: LSR & RSR - start with new L & R
  // lsrSample = lsrHighPass.processSingleSampleRaw(leftSampleA);
  // rsrSample = rsrHighPass.processSingleSampleRaw(rightSampleA);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.lsrSample, pprivate->buffer.leftSampleA, totalsamples);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.rsrSample, pprivate->buffer.rightSampleA, totalsamples);
  pprivate->lsrHighPass->processInPlace(pprivate->buffer.lsrSample, totalsamples);
  pprivate->rsrHighPass->processInPlace(pprivate->buffer.rsrSample, totalsamples);
  // pprivate->lsrHighPass->Process(pprivate->buffer.lsrSample, totalsamples);
  // pprivate->rsrHighPass->Process(pprivate->buffer.rsrSample, totalsamples);

  ////////////////////////////DDL START//////////////////////////////
  // float lsrOrigSample = lsrSample;
  // float rsrOrigSample = rsrSample;

  // //read delayed sample into current audio buffer
  // lsrSample = lsrDDLReader[lsrDDLReadIndex];
  // rsrSample = rsrDDLReader[rsrDDLReadIndex];
  //
  // //get current audio and store in delay line
  // lsrDDLWriter[lsrDDLWriteIndex] = lsrOrigSample;
  // rsrDDLWriter[rsrDDLWriteIndex] = rsrOrigSample;

  pprivate->lsrDDL->Process(pprivate->buffer.lsrSample, totalsamples);
  pprivate->rsrDDL->Process(pprivate->buffer.rsrSample, totalsamples);

  //routing of processed samples to output
  hear360_algr::CopyMonoSIMDWithStride(pOutBuf, pprivate->buffer.leftSampleA, totalsamples, dstChannels, 1, pprivate->leftChannelNum, 0);
  hear360_algr::CopyMonoSIMDWithStride(pOutBuf, pprivate->buffer.rightSampleA, totalsamples, dstChannels, 1, pprivate->rightChannelNum, 0);
  // hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->leftChannelNum);
  // hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->rightChannelNum);
  hear360_algr::CopyMonoSIMDWithStride(pOutBuf, pprivate->buffer.centerSample, totalsamples, dstChannels, 1, pprivate->centerChannelNum, 0);
  // hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->centerChannelNum);
  hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->lfeChannelNum);
  hear360_algr::CopyMonoSIMDWithStride(pOutBuf, pprivate->buffer.lsrSample, totalsamples, dstChannels, 1, pprivate->lsrChannelNum, 0);
  // hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->lsrChannelNum);
  hear360_algr::CopyMonoSIMDWithStride(pOutBuf, pprivate->buffer.rsrSample, totalsamples, dstChannels, 1, pprivate->rsrChannelNum, 0);
  // hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, pprivate->rsrChannelNum);

  for(int i = 6; i < dstChannels; i++) {
    hear360_algr::SetMonoWithStride(pOutBuf, 0, totalsamples, dstChannels, i);
  }

  return true;
}

bool ProcessOutOfPlace(void* handle, const float** pInBuf, float** pOutBuf, int srcChannels, int dstChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	return false;

  if (pOutBuf == NULL)
	return false;

  //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	//return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	  return false;

  hear360_algr::CopyMono(pprivate->buffer.leftInput, pInBuf[pprivate->leftChannelNum], totalsamples);
  hear360_algr::CopyMono(pprivate->buffer.rightInput, pInBuf[pprivate->rightChannelNum], totalsamples);

  //PATH 1: C
  // centerSample = (leftReader[i] + rightReader[i]) * negThreeDB;
  hear360_algr::AddMonoSIMD(pprivate->buffer.centerSample, pprivate->buffer.leftInput, pprivate->buffer.rightInput, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.centerSample, pprivate->buffer.centerSample, pprivate->negThreeDB, totalsamples);

  // centerSample = centerHighPass.processSingleSampleRaw(centerSample);
  pprivate->centerHighPass->processInPlace(pprivate->buffer.centerSample, totalsamples);
  // pprivate->centerHighPass->Process(pprivate->buffer.centerSample, totalsamples);

  //PATH 2: L + R
  // subpath A
  // leftSampleA = leftReader[i] * negFourteenDB;
  // rightSampleA = rightReader[i] * negFourteenDB;
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftInput, pprivate->negFourteenDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightInput, pprivate->negFourteenDB, totalsamples);

  //subpath B
  // leftSampleB = leftReader[i] * negSevenDB;
  // rightSampleB = rightReader[i] * negSevenDB;
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleB, pprivate->buffer.leftInput, pprivate->negSevenDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleB, pprivate->buffer.rightInput, pprivate->negSevenDB, totalsamples);

  // leftSampleB = leftLowPass.processSingleSampleRaw(leftSampleB);
  // rightSampleB = rightLowPass.processSingleSampleRaw(rightSampleB);
  pprivate->leftLowPass->processInPlace(pprivate->buffer.leftSampleB, totalsamples);
  pprivate->rightLowPass->processInPlace(pprivate->buffer.rightSampleB, totalsamples);
  // pprivate->leftLowPass->Process(pprivate->buffer.leftSampleB, totalsamples);
  // pprivate->rightLowPass->Process(pprivate->buffer.rightSampleB, totalsamples);

  //sum A + B
  // leftSampleA += leftSampleB;
  // rightSampleA += rightSampleB;
  hear360_algr::AddMonoSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleB, totalsamples);
  hear360_algr::AddMonoSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleB, totalsamples);

  //subpath C
  // leftSampleC = leftReader[i] - rightReader[i];
  // leftSampleC *= negSixDB;
  // rightSampleC = leftSampleC * -1.f;
  hear360_algr::SubMonoSIMD(pprivate->buffer.leftSampleC, pprivate->buffer.leftInput, pprivate->buffer.rightInput, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.leftSampleC, pprivate->buffer.leftSampleC, pprivate->negSixDB, totalsamples);
  hear360_algr::MulMonoByScalarSIMD(pprivate->buffer.rightSampleC, pprivate->buffer.leftSampleC, pprivate->negOne, totalsamples);

  //sum path (A+B) w/ C
  // leftSampleA += leftSampleC;
  // rightSampleA += rightSampleC;
  hear360_algr::AddMonoSIMD(pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleA, pprivate->buffer.leftSampleC, totalsamples);
  hear360_algr::AddMonoSIMD(pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleA, pprivate->buffer.rightSampleC, totalsamples);

  //PATH 3: LSR & RSR - start with new L & R
  // lsrSample = lsrHighPass.processSingleSampleRaw(leftSampleA);
  // rsrSample = rsrHighPass.processSingleSampleRaw(rightSampleA);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.lsrSample, pprivate->buffer.leftSampleA, totalsamples);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.rsrSample, pprivate->buffer.rightSampleA, totalsamples);
  pprivate->lsrHighPass->processInPlace(pprivate->buffer.lsrSample, totalsamples);
  pprivate->rsrHighPass->processInPlace(pprivate->buffer.rsrSample, totalsamples);
  // pprivate->lsrHighPass->Process(pprivate->buffer.lsrSample, totalsamples);
  // pprivate->rsrHighPass->Process(pprivate->buffer.rsrSample, totalsamples);

  ////////////////////////////DDL START//////////////////////////////
  // float lsrOrigSample = lsrSample;
  // float rsrOrigSample = rsrSample;

  // //read delayed sample into current audio buffer
  // lsrSample = lsrDDLReader[lsrDDLReadIndex];
  // rsrSample = rsrDDLReader[rsrDDLReadIndex];
  //
  // //get current audio and store in delay line
  // lsrDDLWriter[lsrDDLWriteIndex] = lsrOrigSample;
  // rsrDDLWriter[rsrDDLWriteIndex] = rsrOrigSample;

  pprivate->lsrDDL->Process(pprivate->buffer.lsrSample, totalsamples);
  pprivate->rsrDDL->Process(pprivate->buffer.rsrSample, totalsamples);

  //routing of processed samples to output
  hear360_algr::CopyMonoSIMD(pOutBuf[pprivate->leftChannelNum], pprivate->buffer.leftSampleA, totalsamples);
  hear360_algr::CopyMonoSIMD(pOutBuf[pprivate->rightChannelNum], pprivate->buffer.rightSampleA, totalsamples);
  hear360_algr::CopyMonoSIMD(pOutBuf[pprivate->centerChannelNum], pprivate->buffer.centerSample, totalsamples);
  memset(pOutBuf[pprivate->lfeChannelNum], 0, totalsamples * sizeof(float));
  hear360_algr::CopyMonoSIMD(pOutBuf[pprivate->lsrChannelNum], pprivate->buffer.lsrSample, totalsamples);
  hear360_algr::CopyMonoSIMD(pOutBuf[pprivate->rsrChannelNum], pprivate->buffer.rsrSample, totalsamples);

  for(int i = 6; i < dstChannels; i++) {
    memset(pOutBuf[i], 0, totalsamples * sizeof(float));
  }

  return true;
}

void ProcessOutOfPlace()
{

}

//######################################################################################################################

} // namespace

//######################################################################################################################
