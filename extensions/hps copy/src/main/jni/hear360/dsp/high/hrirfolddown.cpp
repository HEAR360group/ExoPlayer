
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>

#include <hear360/algr/Base/DSPUtils.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/os/subnormal.h>
#include <hear360/dsp/low/stereoequalizer.h>
#include <hear360/dsp/high/hrirfolddown.h>
//#include <libs/fftw3.h>

#include <hear360/algr/Base/MultiData.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddown
{
#include <hear360/algr/Convolution/HPSConvolutionIRsInt.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi.h>
#include <hear360/algr/Convolution/HPSConvolutionIRsIntSoundFi2.h>

//######################################################################################################################

#define HALF (0.5f)
#define QUARTER (0.25f)

#define SQUAREROOTHALF (0.70710678f)

//######################################################################################################################

GAINPARAMETERS::GAINPARAMETERS ()
:
  master (0.0f),

  frontLeft (0.0f),
  frontRight (0.0f),
  center (0.0f),
  lfe (0.0f),
  backLeft (0.0f),
  backRight (0.0f),
  sideLeft (0.0f),
  sideRight (0.0f)
{}

//######################################################################################################################

PARAMETERS::PARAMETERS ()
: irtailpercentage (100.0f)
, applylegacygains (false)
, enableeq (false)
{}

//######################################################################################################################

CONVOLUTIONPROCESSOR::CONVOLUTIONPROCESSOR(hear360_dsp_os_memory::MANAGER memorymanager)
  : frontleft (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , frontright (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , frontcenter (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , backleft (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , backright (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , sideleft (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , sideright (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
  , lfe (hear360_dsp_os_memory_AUDIOBUFFERSIZE, memorymanager)
{

}

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate)
  : memorymanager (memorymanagerparam)
  , enable (false)
  , enableeq (false)
  , curoffset (0)
  , irtailpercentage (100.0f)
  , mSamplerate(samplerate)
  , convolutionprocessor (memorymanagerparam)
{
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.inputbus, 8 * sizeof(float*));
  for(unsigned int i = 0; i < 8; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.inputbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.inputbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.outputbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.outputbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.outputbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.frontbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rearbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.rearbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.rearbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.centerbus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.centerbus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.centerbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.sidebus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.sidebus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.sidebus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.lfebus, 2 * sizeof(float*));
  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.lfebus[i], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.lfebus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  /*
  buffer.inputbus = new float*[8];
  for(unsigned int i = 0; i < 8; i++)
  {
    buffer.inputbus[i] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    memset(buffer.inputbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  buffer.outputbus = new float*[2];
  for(unsigned int i = 0; i < 2; i++)
  {
    buffer.outputbus[i] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    memset(buffer.outputbus[i], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  }

  buffer.frontbus = new float*[2];
  buffer.frontbus[0] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  buffer.frontbus[1] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  memset(buffer.frontbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.frontbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  buffer.rearbus = new float*[2];
  buffer.rearbus[0] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  buffer.rearbus[1] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  memset(buffer.rearbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.rearbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  buffer.centerbus = new float*[2];
  buffer.centerbus[0] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  buffer.centerbus[1] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  memset(buffer.centerbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.centerbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  buffer.sidebus = new float*[2];
  buffer.sidebus[0] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  buffer.sidebus[1] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  memset(buffer.sidebus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.sidebus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  */

  /*
  convolutionprocessor.frontleft.loadIRs(ir_left_l(samplerate), ir_left_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

  convolutionprocessor.frontright.loadIRs(ir_right_l(samplerate), ir_right_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

  convolutionprocessor.frontcenter.loadIRs(ir_center_l(samplerate), ir_center_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

  convolutionprocessor.backleft.loadIRs(ir_rearleft_l(samplerate), ir_rearleft_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

  convolutionprocessor.backright.loadIRs(ir_rearright_l(samplerate), ir_rearright_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

#if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
  convolutionprocessor.sideleft.loadIRs(ir_sideleft_l(samplerate), ir_sideleft_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));

  convolutionprocessor.sideright.loadIRs(ir_sideright_l(samplerate), ir_sideright_r(samplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
  convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(samplerate));
#endif

  delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
*/

  equalizerprocessor.warm.Init(samplerate);
  equalizerprocessor.lfe.Init(samplerate);
}

//######################################################################################################################
PRIVATE::~PRIVATE ()
{
#if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
  convolutionprocessor.sideright.UnloadIRs();
  convolutionprocessor.sideleft.UnloadIRs();
  /*
  #ifdef HPS_CONVOLUTION_IR_LFE_PROCESS
    convolutionprocessor.lfe.UnloadIRs();
  #endif
  */
#endif

  convolutionprocessor.backright.UnloadIRs();
  convolutionprocessor.backleft.UnloadIRs();
  convolutionprocessor.frontcenter.UnloadIRs();
  convolutionprocessor.frontright.UnloadIRs();
  convolutionprocessor.frontleft.UnloadIRs();

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.outputbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.outputbus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.inputbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.inputbus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.lfebus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.lfebus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.sidebus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.sidebus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.centerbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.centerbus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.rearbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.rearbus);

  for(unsigned int i = 0; i < 2; i++)
  {
    memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontbus[i]);
  }
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontbus);
  /*
  for(unsigned int i = 0; i < 2; i++)
  {
    fftwf_free(buffer.outputbus[i]);
  }
  delete[] buffer.outputbus;

  for(unsigned int i = 0; i < 8; i++)
  {
    fftwf_free(buffer.inputbus[i]);
  }
  delete[] buffer.inputbus;

  fftwf_free(buffer.sidebus[0]);
  fftwf_free(buffer.sidebus[1]);
  delete[] buffer.sidebus;

  fftwf_free(buffer.centerbus[0]);
  fftwf_free(buffer.centerbus[1]);
  delete[] buffer.centerbus;

  fftwf_free(buffer.rearbus[0]);
  fftwf_free(buffer.rearbus[1]);
  delete[] buffer.rearbus;

  fftwf_free(buffer.frontbus[0]);
  fftwf_free(buffer.frontbus[1]);
  delete[] buffer.frontbus;
  */
}

//######################################################################################################################

void PROCESSOR::LoadIRsFromPresets(int presetID)
{
  //default
  if(presetID == 0)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_default::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_default::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_default::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_default::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_default::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    #if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_default::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_default::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    #endif

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
  else if(presetID == 1)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    #if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
    /*
    #ifdef HPS_CONVOLUTION_IR_LFE_PROCESS
      privatedata.convolutionprocessor.lfe.loadIRs(hear360_convolution_ir_soundfi::ir_lfe_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_lfe_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
      privatedata.convolutionprocessor.lfe.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    #endif
*/
    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_soundfi::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_soundfi::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    #endif

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
  else if(presetID == 2)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi2::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi2::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi2::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi2::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi2::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    #if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
    /*
    #ifdef HPS_CONVOLUTION_IR_LFE_PROCESS
      privatedata.convolutionprocessor.lfe.loadIRs(hear360_convolution_ir_soundfi2::ir_lfe_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_lfe_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
      privatedata.convolutionprocessor.lfe.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    #endif
    */
    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_soundfi2::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_soundfi2::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    #endif

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
}

bool PROCESSOR::Update (bool enable, const PARAMETERS *pparameters)
{
  bool updatefailed;

  privatedata.enable = enable;

  if (!enable)
  {
    return (false);
  }

  updatefailed = false;

  privatedata.gainfactor.master         = 0; // not used because I added master to the individual gains below.
  privatedata.gainfactor.frontLeft      = hear360_algr::gainDbToFactor (pparameters->gaindb.frontLeft          + pparameters->gaindb.master);
  privatedata.gainfactor.frontRight     = hear360_algr::gainDbToFactor (pparameters->gaindb.frontRight         + pparameters->gaindb.master);
  privatedata.gainfactor.center         = hear360_algr::gainDbToFactor (pparameters->gaindb.center         + pparameters->gaindb.master);
  privatedata.gainfactor.lfe            = hear360_algr::gainDbToFactor (pparameters->gaindb.lfe            + pparameters->gaindb.master);
  privatedata.gainfactor.backLeft       = hear360_algr::gainDbToFactor (pparameters->gaindb.backLeft       + pparameters->gaindb.master);
  privatedata.gainfactor.backRight       = hear360_algr::gainDbToFactor (pparameters->gaindb.backRight       + pparameters->gaindb.master);
  privatedata.gainfactor.sideLeft        = hear360_algr::gainDbToFactor (pparameters->gaindb.sideLeft           + pparameters->gaindb.master);
  privatedata.gainfactor.sideRight       = hear360_algr::gainDbToFactor (pparameters->gaindb.sideRight           + pparameters->gaindb.master);

  if (pparameters->applylegacygains)
  {
	privatedata.gainfactor.frontLeft *= 0.44668359215096315;          //  -7 dB
  privatedata.gainfactor.frontRight *= 0.44668359215096315;          //  -7 dB
	privatedata.gainfactor.center *= 0.3758374042884442;          //  -8.5 dB
	privatedata.gainfactor.lfe *= 0.22387211385683395;            //  -13 dB
	privatedata.gainfactor.backLeft *= 0.44668359215096315;           //  -7 dB
  privatedata.gainfactor.backRight *= 0.44668359215096315;           //  -7 dB
	privatedata.gainfactor.sideLeft *= 0.44668359215096315;           //  -7 dB
  privatedata.gainfactor.sideRight *= 0.44668359215096315;           //  -7 dB
  }

  privatedata.equalizerprocessor.warm .Update (&pparameters->equalizer.warm);
  privatedata.equalizerprocessor.lfe .Update (&pparameters->equalizer.lfe);

  privatedata.enableeq = pparameters->enableeq;

  if(privatedata.irtailpercentage != pparameters->irtailpercentage)
  {
    privatedata.irtailpercentage = pparameters->irtailpercentage;

    unsigned int irframes = hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate);
    //unsigned int irframes = privatedata.irtailpercentage * hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate) / 100.0f;
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(irframes);
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(irframes);
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(irframes);
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(irframes);
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(irframes);

    if(privatedata.convolutionprocessor.lfe.IsIRLoaded())
    {
      privatedata.convolutionprocessor.lfe.setEffectiveIRFrames(irframes);
    }

#if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(irframes);
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(irframes);
#endif

  }

  //if (privatedata.distanceprocessor.Update (&pparameters->distance)) {updatefailed = true;}

  return (updatefailed);
}

//######################################################################################################################

void PROCESSOR::Reset (void)
{
  privatedata.enable = false;
  privatedata.enableeq = false;
  privatedata.equalizerprocessor.warm .Reset ();
  privatedata.equalizerprocessor.lfe .Reset ();

  privatedata.delayLFE.Reset();
  //privatedata.distanceprocessor.Reset ();

  return;
}

//######################################################################################################################

static void Process6ChannelsToStereoInBlocks
(
  PRIVATE *pprivate,
  float *ppdstaudio[2],            // stereo destination audio
  float *ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],            // 6 channel source audio
  int totalsrcchannels,
  long totalsamples
)
{
  long tocopysamples = std::min(hear360_dsp_os_memory_AUDIOBUFFERSIZE - pprivate->curoffset, totalsamples);
  for(int i = 0; i < totalsrcchannels; i++)
  {
    if(ppsrcaudio[i] != 0)
    {
      hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus[i] + pprivate->curoffset, ppsrcaudio[i], tocopysamples);
    }
    else
    {
      pprivate->buffer.inputbus[i] = 0;
    }
  }
  for(int i = 0; i < 2; i++)
  {
    hear360_algr::CopyMonoSIMD(ppdstaudio[i], pprivate->buffer.outputbus[i] + pprivate->curoffset, tocopysamples);
  }
  pprivate->curoffset += tocopysamples;

  if(pprivate->curoffset == hear360_dsp_os_memory_AUDIOBUFFERSIZE)
  {
    memset(pprivate->buffer.outputbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.outputbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

    memset(pprivate->buffer.frontbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.frontbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTLEFT] != 0)
    {
      pprivate->convolutionprocessor.frontleft.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTLEFT], pprivate->buffer.frontbus[0], pprivate->buffer.frontbus[1]);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.frontbus[0], pprivate->gainfactor.frontLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.frontbus[1], pprivate->gainfactor.frontLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }

    memset(pprivate->buffer.frontbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.frontbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTRIGHT] != 0)
    {
      pprivate->convolutionprocessor.frontright.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTRIGHT], pprivate->buffer.frontbus[0], pprivate->buffer.frontbus[1]);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.frontbus[0], pprivate->gainfactor.frontRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.frontbus[1], pprivate->gainfactor.frontRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }


    memset(pprivate->buffer.rearbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.rearbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::BACKLEFT] != 0)
    {
      pprivate->convolutionprocessor.backleft.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::BACKLEFT], pprivate->buffer.rearbus[0], pprivate->buffer.rearbus[1]);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.rearbus[0], pprivate->gainfactor.backLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.rearbus[1], pprivate->gainfactor.backLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }

    memset(pprivate->buffer.rearbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.rearbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::BACKRIGHT] != 0)
    {
      pprivate->convolutionprocessor.backright.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::BACKRIGHT], pprivate->buffer.rearbus[0], pprivate->buffer.rearbus[1]);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.rearbus[0], pprivate->gainfactor.backRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.rearbus[1], pprivate->gainfactor.backRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }


    memset(pprivate->buffer.centerbus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    memset(pprivate->buffer.centerbus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTCENTER] != 0)
    {
      pprivate->convolutionprocessor.frontcenter.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::FRONTCENTER], pprivate->buffer.centerbus[0], pprivate->buffer.centerbus[1]);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.centerbus[0], pprivate->gainfactor.center, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.centerbus[1], pprivate->gainfactor.center, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    }


    if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE] != 0)
    {
      bool prosssLFE = false;
/*
      #ifdef HPS_CONVOLUTION_IR_LFE_PROCESS
      if(pprivate->convolutionprocessor.lfe.IsIRLoaded())
      {
        prosssLFE = true;

        //Process with IRs
        memset(pprivate->buffer.lfebus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
        memset(pprivate->buffer.lfebus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
        if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE] != 0)
        {
          pprivate->convolutionprocessor.lfe.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], pprivate->buffer.lfebus[0], pprivate->buffer.lfebus[1]);
          hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.lfebus[0], pprivate->gainfactor.lfe, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
          hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.lfebus[1], pprivate->gainfactor.lfe, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
        }
      }
      #endif
*/
      if(!prosssLFE)
      {
        pprivate->delayLFE.Process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], hear360_dsp_os_memory_AUDIOBUFFERSIZE);

    	//if (pprivate->enableeq)
    		pprivate->equalizerprocessor.lfe.Process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], hear360_dsp_os_memory_AUDIOBUFFERSIZE);
        pprivate->equalizerprocessor.lfe.Process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], hear360_dsp_os_memory_AUDIOBUFFERSIZE);

        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], pprivate->gainfactor.lfe, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::LFE], pprivate->gainfactor.lfe, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      }
    }


#if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
    if(totalsrcchannels == 8)
    {
      memset(pprivate->buffer.sidebus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
      memset(pprivate->buffer.sidebus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
      if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::SIDELEFT] != 0)
      {
        pprivate->convolutionprocessor.sideleft.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::SIDELEFT], pprivate->buffer.sidebus[0], pprivate->buffer.sidebus[1]);
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.sidebus[0], pprivate->gainfactor.sideLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.sidebus[1], pprivate->gainfactor.sideLeft, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      }

      memset(pprivate->buffer.sidebus[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
      memset(pprivate->buffer.sidebus[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
      if(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::SIDERIGHT] != 0)
      {
        pprivate->convolutionprocessor.sideright.fft_process(pprivate->buffer.inputbus[hear360_dsp_high_hrirfolddown::SIDERIGHT], pprivate->buffer.sidebus[0], pprivate->buffer.sidebus[1]);
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[0], pprivate->buffer.sidebus[0], pprivate->gainfactor.sideRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.outputbus[1], pprivate->buffer.sidebus[1], pprivate->gainfactor.sideRight, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
      }
    }
#endif

    if (pprivate->enableeq)
  	  pprivate->equalizerprocessor.warm.Process(pprivate->buffer.outputbus, hear360_dsp_os_memory_AUDIOBUFFERSIZE);

    pprivate->curoffset = 0;
  }

  long resttocopysamples = totalsamples - tocopysamples;
  for(int i = 0; i < totalsrcchannels; i++)
  {
    hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus[i], ppsrcaudio[i] + tocopysamples, resttocopysamples);
  }
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
  float *ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],            // 6 channel source audio
  int totalsrcchannels,
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

  float *ppsrcaudioTemp[8];
  for(int i = 0; i < totalsrcchannels; i++)
  {
    if(ppsrcaudio[i] != 0)
    {
      ppsrcaudioTemp[i] = ppsrcaudio[i] + processedFrames;
    }
    else
    {
      ppsrcaudioTemp[i] = 0;
    }
  }

  for(int i = 0; i < pagesMinus1; i++)
  {
    Process6ChannelsToStereoInBlocks(pprivate, ppdstaudioTemp, ppsrcaudioTemp, totalsrcchannels, hear360_dsp_os_memory_AUDIOBUFFERSIZE);
    processedFrames += hear360_dsp_os_memory_AUDIOBUFFERSIZE;
  }

  if(framesInLastPage != 0)
  {
    Process6ChannelsToStereoInBlocks(pprivate, ppdstaudioTemp, ppsrcaudioTemp, totalsrcchannels, framesInLastPage);
  }

  return;
}

//######################################################################################################################

// The formula used is from "Recommendation ITU-R BS.775-3", annex 4, table 2.

static void Process6ChannelsToStereoITUFolddown
(
  PRIVATE *pprivate,
  float *ppdstaudio[2],                         // stereo destination audio
  const float * const ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],            // 6 channel source audio
  int totalsrcchannels,
  long totalsamples
)
{
  hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus[0], ppsrcaudio[FRONTLEFT], totalsamples);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus[1], ppsrcaudio[FRONTRIGHT], totalsamples);
  hear360_algr::CopyMonoSIMD(pprivate->buffer.inputbus[2], ppsrcaudio[FRONTCENTER], totalsamples);

  memset(ppdstaudio[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(ppdstaudio[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  if(totalsrcchannels >= 6)
  {
    if(ppsrcaudio[FRONTLEFT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], pprivate->buffer.inputbus[0], HALF * HALF, totalsamples);
    if(ppsrcaudio[FRONTRIGHT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], pprivate->buffer.inputbus[1], HALF * HALF, totalsamples);
    if(ppsrcaudio[FRONTCENTER] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], pprivate->buffer.inputbus[2], HALF * HALF * HALF, totalsamples);
    if(ppsrcaudio[FRONTCENTER] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], pprivate->buffer.inputbus[2], HALF * HALF * HALF, totalsamples);
    if(ppsrcaudio[BACKLEFT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], ppsrcaudio[BACKLEFT],    HALF * HALF, totalsamples);
    if(ppsrcaudio[BACKRIGHT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], ppsrcaudio[BACKRIGHT],   HALF * HALF, totalsamples);
    if(ppsrcaudio[LFE] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], ppsrcaudio[LFE], HALF * HALF, totalsamples);
    if(ppsrcaudio[LFE] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], ppsrcaudio[LFE], HALF * HALF, totalsamples);
  }

  if(totalsrcchannels >= 8)
  {
    if(ppsrcaudio[FRONTLEFT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], pprivate->buffer.inputbus[0], HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[FRONTRIGHT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], pprivate->buffer.inputbus[1], HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[FRONTCENTER] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], pprivate->buffer.inputbus[2], HALF * HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[FRONTCENTER] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], pprivate->buffer.inputbus[2], HALF * HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[BACKLEFT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], ppsrcaudio[BACKLEFT],    HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[BACKRIGHT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], ppsrcaudio[BACKRIGHT],   HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[LFE] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], ppsrcaudio[LFE], HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[LFE] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], ppsrcaudio[LFE], HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[SIDELEFT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[0], ppsrcaudio[SIDELEFT],    HALF * HALF * SQUAREROOTHALF, totalsamples);
    if(ppsrcaudio[SIDERIGHT] != 0)
      hear360_algr::MixMonoByScalarSIMD   (ppdstaudio[1], ppsrcaudio[SIDERIGHT],   HALF * HALF * SQUAREROOTHALF, totalsamples);
  }

  return;
}

//######################################################################################################################

void PROCESSOR::Process6ChannelsToStereo
(
  float *ppdstaudio[2],                         // stereo destination audio
  float *ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],            // 6 channel source audio
  int totalsrcchannels,
  long totalsamples
)
{
  if (!privatedata.enable)
  {
     Process6ChannelsToStereoITUFolddown (&privatedata, ppdstaudio, ppsrcaudio, totalsrcchannels, totalsamples);

     return;
  }

  Process6ChannelsToStereoLow (&privatedata, ppdstaudio, ppsrcaudio, totalsrcchannels, totalsamples);
  //Process6ChannelsToStereoInBlocks (&privatedata, ppdstaudio, ppsrcaudio, totalsamples);

  return;
}

//######################################################################################################################

PROCESSOR::PROCESSOR (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate)
:
  privatedata (memorymanager, samplerate)
{
  hear360_dsp_os_subnormal::DisableSubnormals ();

  Reset();

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
