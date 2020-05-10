
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
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi2.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi3.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntHybridC.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSky.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFiSky.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi2Sky.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi3Sky.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntHybridCSky.h>

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
  , sampleRateRelatedGain (1.0f)
{
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.inputbus, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.inputbus, 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.processbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.processbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.processbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

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

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.processbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.processbus);

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

void PROCESSOR::LoadIRFromFloats(float* leftIRs, float* rightIRs, unsigned int irFrames)
{
  privatedata.convolutionprocessor.crfilter.loadIRFromFloats(leftIRs, rightIRs, irFrames);
  privatedata.convolutionprocessor.crfilter.setEffectiveIRFrames(irFrames);
}

void PROCESSOR::LoadIRFromInts(int* leftIRs, int* rightIRs, unsigned int irFrames)
{
  privatedata.convolutionprocessor.crfilter.loadIRs(leftIRs, rightIRs, irFrames);
  privatedata.convolutionprocessor.crfilter.setEffectiveIRFrames(irFrames);
}

//######################################################################################################################

void PROCESSOR::LoadIRs(unsigned int channelID, int samplerate, int presetID)
{
  //float sampleRateGain = (float)samplerate / 48000.0f;
#ifdef HPS_CONVOLUTION_IR_DEFAULT_SET
  if(presetID == 0)
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
      case 5:
        LoadIRFromInts(hear360_convolution_ir_default::ir_sideleft_l(samplerate), hear360_convolution_ir_default::ir_sideleft_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 6:
        LoadIRFromInts(hear360_convolution_ir_default::ir_sideright_l(samplerate), hear360_convolution_ir_default::ir_sideright_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SET
  if(presetID == 1)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_left_l(samplerate), hear360_convolution_ir_soundfi::ir_left_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_right_l(samplerate), hear360_convolution_ir_soundfi::ir_right_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_center_l(samplerate), hear360_convolution_ir_soundfi::ir_center_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 5:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_sideleft_l(samplerate), hear360_convolution_ir_soundfi::ir_sideleft_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 6:
        LoadIRFromInts(hear360_convolution_ir_soundfi::ir_sideright_l(samplerate), hear360_convolution_ir_soundfi::ir_sideright_r(samplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SET
  if(presetID == 2)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_left_l(samplerate), hear360_convolution_ir_soundfi2::ir_left_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_right_l(samplerate), hear360_convolution_ir_soundfi2::ir_right_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_center_l(samplerate), hear360_convolution_ir_soundfi2::ir_center_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi2::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi2::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 5:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_sideleft_l(samplerate), hear360_convolution_ir_soundfi2::ir_sideleft_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 6:
        LoadIRFromInts(hear360_convolution_ir_soundfi2::ir_sideright_l(samplerate), hear360_convolution_ir_soundfi2::ir_sideright_r(samplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SET
  if(presetID == 3)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_left_l(samplerate), hear360_convolution_ir_soundfi3::ir_left_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_right_l(samplerate), hear360_convolution_ir_soundfi3::ir_right_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_center_l(samplerate), hear360_convolution_ir_soundfi3::ir_center_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi3::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi3::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 5:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_sideleft_l(samplerate), hear360_convolution_ir_soundfi3::ir_sideleft_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 6:
        LoadIRFromInts(hear360_convolution_ir_soundfi3::ir_sideright_l(samplerate), hear360_convolution_ir_soundfi3::ir_sideright_r(samplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SET
  if(presetID == 4)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_left_l(samplerate), hear360_convolution_ir_hybridc::ir_left_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_right_l(samplerate), hear360_convolution_ir_hybridc::ir_right_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_center_l(samplerate), hear360_convolution_ir_hybridc::ir_center_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_rearleft_l(samplerate), hear360_convolution_ir_hybridc::ir_rearleft_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_rearright_l(samplerate), hear360_convolution_ir_hybridc::ir_rearright_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 5:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_sideleft_l(samplerate), hear360_convolution_ir_hybridc::ir_sideleft_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 6:
        LoadIRFromInts(hear360_convolution_ir_hybridc::ir_sideright_l(samplerate), hear360_convolution_ir_hybridc::ir_sideright_r(samplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_DEFAULT_SKY_SET
  if(presetID == 100)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_default_sky::ir_left_l(samplerate), hear360_convolution_ir_default_sky::ir_left_r(samplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_default_sky::ir_right_l(samplerate), hear360_convolution_ir_default_sky::ir_right_r(samplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_default_sky::ir_center_l(samplerate), hear360_convolution_ir_default_sky::ir_center_r(samplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_default_sky::ir_rearleft_l(samplerate), hear360_convolution_ir_default_sky::ir_rearleft_r(samplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_default_sky::ir_rearright_l(samplerate), hear360_convolution_ir_default_sky::ir_rearright_r(samplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SKY_SET
  if(presetID == 101)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi_sky::ir_left_l(samplerate), hear360_convolution_ir_soundfi_sky::ir_left_r(samplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi_sky::ir_right_l(samplerate), hear360_convolution_ir_soundfi_sky::ir_right_r(samplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi_sky::ir_center_l(samplerate), hear360_convolution_ir_soundfi_sky::ir_center_r(samplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi_sky::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi_sky::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi_sky::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi_sky::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SKY_SET
  if(presetID == 102)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi2_sky::ir_left_l(samplerate), hear360_convolution_ir_soundfi2_sky::ir_left_r(samplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi2_sky::ir_right_l(samplerate), hear360_convolution_ir_soundfi2_sky::ir_right_r(samplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi2_sky::ir_center_l(samplerate), hear360_convolution_ir_soundfi2_sky::ir_center_r(samplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi2_sky::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi2_sky::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi2_sky::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi2_sky::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SKY_SET
  if(presetID == 103)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_soundfi3_sky::ir_left_l(samplerate), hear360_convolution_ir_soundfi3_sky::ir_left_r(samplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_soundfi3_sky::ir_right_l(samplerate), hear360_convolution_ir_soundfi3_sky::ir_right_r(samplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_soundfi3_sky::ir_center_l(samplerate), hear360_convolution_ir_soundfi3_sky::ir_center_r(samplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_soundfi3_sky::ir_rearleft_l(samplerate), hear360_convolution_ir_soundfi3_sky::ir_rearleft_r(samplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_soundfi3_sky::ir_rearright_l(samplerate), hear360_convolution_ir_soundfi3_sky::ir_rearright_r(samplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SKY_SET
  if(presetID == 104)
  {
    switch(channelID)
    {
      case 0:
        LoadIRFromInts(hear360_convolution_ir_hybridc_sky::ir_left_l(samplerate), hear360_convolution_ir_hybridc_sky::ir_left_r(samplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 1:
        LoadIRFromInts(hear360_convolution_ir_hybridc_sky::ir_right_l(samplerate), hear360_convolution_ir_hybridc_sky::ir_right_r(samplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 2:
        LoadIRFromInts(hear360_convolution_ir_hybridc_sky::ir_center_l(samplerate), hear360_convolution_ir_hybridc_sky::ir_center_r(samplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 3:
        LoadIRFromInts(hear360_convolution_ir_hybridc_sky::ir_rearleft_l(samplerate), hear360_convolution_ir_hybridc_sky::ir_rearleft_r(samplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
      case 4:
        LoadIRFromInts(hear360_convolution_ir_hybridc_sky::ir_rearright_l(samplerate), hear360_convolution_ir_hybridc_sky::ir_rearright_r(samplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(samplerate));
        break;
    }
  }
#endif
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

    memset(pprivate->buffer.processbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.processbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

    pprivate->convolutionprocessor.crfilter.fft_process(pprivate->buffer.inputbus, pprivate->buffer.processbus[0], pprivate->buffer.processbus[1]);
    for(int i = 0; i < 2; i++)
    {
      hear360_algr::CopyMonoSIMD(pprivate->buffer.outputbus[i], pprivate->buffer.processbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }

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
