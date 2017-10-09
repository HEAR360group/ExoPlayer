
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>

#include <hear360/algr/Base/DSPUtils.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/os/subnormal.h>
#include <hear360/dsp/high/convolutioncore.h>
//#include <libs/fftw3.h>

#include <hear360/algr/Base/MultiData.h>

//######################################################################################################################

namespace hear360_dsp_high_convolutioncore
{
#include <hear360/algr/Convolution/HPSConvolutionIRsInt.h>
//######################################################################################################################

#define HALF (0.5f)
#define QUARTER (0.25f)

#define SQUAREROOTHALF (0.70710678f)

//######################################################################################################################

PARAMETERS::PARAMETERS ()
: irtailpercentage (100.0f)
{}

//######################################################################################################################

CONVOLUTIONPROCESSOR::CONVOLUTIONPROCESSOR(hear360_dsp_os_memory::MANAGER memorymanager)
  : crfilter (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
{

}

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam)
  : memorymanager (memorymanagerparam)
  , curoffset (0)
  , irtailpercentage (100.0f)
  , convolutionprocessor (memorymanagerparam)
{
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.inputbus, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.inputbus, 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.outputbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.outputbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.outputbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }
}

//######################################################################################################################
PRIVATE::~PRIVATE ()
{
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.outputbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.outputbus);

  memorymanager.pFree(memorymanager.pmanagerdata, buffer.inputbus);
}

//######################################################################################################################

bool PROCESSOR::Update (const PARAMETERS *pparameters)
{
  bool updatefailed;

  updatefailed = false;

  if(privatedata.irtailpercentage != pparameters->irtailpercentage)
  {
    privatedata.irtailpercentage = pparameters->irtailpercentage;

    unsigned int irframes = privatedata.convolutionprocessor.crfilter.getTotalIRFrames();
    //unsigned int irframes = hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate);
    //unsigned int irframes = privatedata.irtailpercentage * hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate) / 100.0f;
    privatedata.convolutionprocessor.crfilter.setEffectiveIRFrames(irframes);
  }

  //if (privatedata.distanceprocessor.Update (&pparameters->distance)) {updatefailed = true;}

  return (updatefailed);
}

//######################################################################################################################

void PROCESSOR::Reset (void)
{
  return;
}

//######################################################################################################################

void PROCESSOR::LoadIRFromInts(int* leftIRs, int* rightIRs, unsigned int irFrames)
{
  privatedata.convolutionprocessor.crfilter.loadIRs(leftIRs, rightIRs, irFrames);
  privatedata.convolutionprocessor.crfilter.setEffectiveIRFrames(irFrames);
}

//######################################################################################################################

void PROCESSOR::LoadIRs(unsigned int channelID, int samplerate)
{
  switch(channelID)
  {
    case 0:
      LoadIRFromInts(hear360_convolution_ir_default::ir_left_l(samplerate), hear360_convolution_ir_default::ir_left_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
    case 1:
      LoadIRFromInts(hear360_convolution_ir_default::ir_right_l(samplerate), hear360_convolution_ir_default::ir_right_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
    case 2:
      LoadIRFromInts(hear360_convolution_ir_default::ir_center_l(samplerate), hear360_convolution_ir_default::ir_center_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
    case 3:
      LoadIRFromInts(hear360_convolution_ir_default::ir_rearleft_l(samplerate), hear360_convolution_ir_default::ir_rearleft_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
    case 4:
      LoadIRFromInts(hear360_convolution_ir_default::ir_rearright_l(samplerate), hear360_convolution_ir_default::ir_rearright_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
#ifndef HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS
    case 5:
      LoadIRFromInts(hear360_convolution_ir_default::ir_sideleft_l(samplerate), hear360_convolution_ir_default::ir_sideleft_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
    case 6:
      LoadIRFromInts(hear360_convolution_ir_default::ir_sideright_l(samplerate), hear360_convolution_ir_default::ir_sideright_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
      break;
#endif
  }

}

//######################################################################################################################

static void Process6ChannelsToStereoInBlocks
(
  PRIVATE *pprivate,
  float *ppdstaudio[2],            // stereo destination audio
  float *ppsrcaudio,               // mono channel source audio
  long totalsamples
)
{
  long tocopysamples = std::min(hear360_dsp_os_memory_AUDIOBUFFERSIZE - pprivate->curoffset, totalsamples);

  hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus + pprivate->curoffset, ppsrcaudio, tocopysamples);

  for(int i = 0; i < 2; i++)
  {
    hear360_algr::CopyMonoSIMD(ppdstaudio[i], pprivate->buffer.outputbus[i] + pprivate->curoffset, tocopysamples);
  }
  pprivate->curoffset += tocopysamples;

  if(pprivate->curoffset == hear360_dsp_os_memory_AUDIOBUFFERSIZE)
  {
    memset(pprivate->buffer.outputbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.outputbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

    pprivate->convolutionprocessor.crfilter.fft_process(pprivate->buffer.inputbus, pprivate->buffer.outputbus[0], pprivate->buffer.outputbus[1]);

    pprivate->curoffset = 0;
  }

  long resttocopysamples = totalsamples - tocopysamples;

  hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus, ppsrcaudio + tocopysamples, resttocopysamples);

  for(int i = 0; i < 2; i++)
  {
    hear360_algr::CopyMonoSIMD(ppdstaudio[i] + tocopysamples, pprivate->buffer.outputbus[i], resttocopysamples);
  }
  pprivate->curoffset += resttocopysamples;

  return;
}

//######################################################################################################################

static void Process6ChannelsToStereoLow
(
  PRIVATE *pprivate,
  float *ppdstaudio[2],                         // stereo destination audio
  float *ppsrcaudio,                            // mono channel source audio
  long totalsamples
)
{
  //Paging
  long processedFrames = 0;
  long pagesMinus1 = totalsamples / hear360_dsp_os_memory_AUDIOBUFFERSIZE;
  long framesInLastPage = totalsamples % hear360_dsp_os_memory_AUDIOBUFFERSIZE;

  float *ppdstaudioTemp[2];
  for(int i = 0; i < 2; i++)
  {
    ppdstaudioTemp[i] = ppdstaudio[i] + processedFrames;
  }

  float *ppsrcaudioTemp;

  if(ppsrcaudio != 0)
  {
    ppsrcaudioTemp = ppsrcaudio + processedFrames;
  }
  else
  {
    ppsrcaudioTemp = 0;
  }

  for(int i = 0; i < pagesMinus1; i++)
  {
    Process6ChannelsToStereoInBlocks(pprivate, ppdstaudioTemp, ppsrcaudioTemp, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    processedFrames += hear360_dsp_os_memory_AUDIOBUFFERSIZE;
  }

  if(framesInLastPage != 0)
  {
    Process6ChannelsToStereoInBlocks(pprivate, ppdstaudioTemp, ppsrcaudioTemp, framesInLastPage);
  }

  return;
}

//######################################################################################################################

void PROCESSOR::Process
(
  float *ppdstaudio[2],                         // stereo destination audio
  float *ppsrcaudio,                            // mono source audio
  long totalsamples
)
{
  Process6ChannelsToStereoLow (&privatedata, ppdstaudio, ppsrcaudio, totalsamples);

  return;
}

//######################################################################################################################

PROCESSOR::PROCESSOR (hear360_dsp_os_memory::MANAGER memorymanager)
:
  privatedata (memorymanager)
{
  hear360_dsp_os_subnormal::DisableSubnormals ();

  Reset();

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
