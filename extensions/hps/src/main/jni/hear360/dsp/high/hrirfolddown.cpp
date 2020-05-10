
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

GAINPARAMETERS::GAINPARAMETERS ()
:
  master (0.0f),

  frontLeft (0.0f),
  frontRight (0.0f),
  center (0.0f),
  lfe (-18.0f),
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
  if(convolutionprocessor.sideright.IsIRLoaded())
      convolutionprocessor.sideright.UnloadIRs();
  if(convolutionprocessor.sideleft.IsIRLoaded())
      convolutionprocessor.sideleft.UnloadIRs();
  /*
  #ifdef HPS_CONVOLUTION_IR_LFE_PROCESS
    convolutionprocessor.lfe.UnloadIRs();
  #endif
  */

  if(convolutionprocessor.backright.IsIRLoaded())
    convolutionprocessor.backright.UnloadIRs();
  if(convolutionprocessor.backright.IsIRLoaded())
    convolutionprocessor.backright.UnloadIRs();
  if(convolutionprocessor.frontcenter.IsIRLoaded())
    convolutionprocessor.frontcenter.UnloadIRs();
  if(convolutionprocessor.frontright.IsIRLoaded())
    convolutionprocessor.frontright.UnloadIRs();
  if(convolutionprocessor.frontleft.IsIRLoaded())
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

void PROCESSOR::LoadIRsFromFloats(float** leftIRs, float** rightIRs, unsigned int irFrames)
{
  if(leftIRs[0] != 0 && rightIRs[0] != 0) {
    privatedata.convolutionprocessor.frontleft.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::FRONTLEFT], rightIRs[hear360_dsp_high_hrirfolddown::FRONTLEFT], irFrames);
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[1] != 0 && rightIRs[1] != 0) {
    privatedata.convolutionprocessor.frontright.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::FRONTRIGHT], rightIRs[hear360_dsp_high_hrirfolddown::FRONTRIGHT], irFrames);
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[2] != 0 && rightIRs[2] != 0) {
    privatedata.convolutionprocessor.frontcenter.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::FRONTCENTER], rightIRs[hear360_dsp_high_hrirfolddown::FRONTCENTER], irFrames);
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[4] != 0 && rightIRs[4] != 0) {
    privatedata.convolutionprocessor.backleft.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::BACKLEFT], rightIRs[hear360_dsp_high_hrirfolddown::BACKLEFT], irFrames);
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[5] != 0 && rightIRs[5] != 0) {
    privatedata.convolutionprocessor.backright.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::BACKRIGHT], rightIRs[hear360_dsp_high_hrirfolddown::BACKRIGHT], irFrames);
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[6] != 0 && rightIRs[6] != 0) {
    privatedata.convolutionprocessor.sideleft.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::SIDELEFT], rightIRs[hear360_dsp_high_hrirfolddown::SIDELEFT], irFrames);
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[7] != 0 && rightIRs[7] != 0) {
    privatedata.convolutionprocessor.sideright.loadIRFromFloats(leftIRs[hear360_dsp_high_hrirfolddown::SIDERIGHT], rightIRs[hear360_dsp_high_hrirfolddown::SIDERIGHT], irFrames);
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(irFrames);
  }

  privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
}

void PROCESSOR::LoadIRsFromInts(int** leftIRs, int** rightIRs, unsigned int irFrames)
{
  if(leftIRs[0] != 0 && rightIRs[0] != 0) {
    privatedata.convolutionprocessor.frontleft.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::FRONTLEFT], rightIRs[hear360_dsp_high_hrirfolddown::FRONTLEFT], irFrames);
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[1] != 0 && rightIRs[1] != 0) {
    privatedata.convolutionprocessor.frontright.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::FRONTRIGHT], rightIRs[hear360_dsp_high_hrirfolddown::FRONTRIGHT], irFrames);
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[2] != 0 && rightIRs[2] != 0) {
    privatedata.convolutionprocessor.frontcenter.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::FRONTCENTER], rightIRs[hear360_dsp_high_hrirfolddown::FRONTCENTER], irFrames);
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[4] != 0 && rightIRs[4] != 0) {
    privatedata.convolutionprocessor.backleft.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::BACKLEFT], rightIRs[hear360_dsp_high_hrirfolddown::BACKLEFT], irFrames);
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[5] != 0 && rightIRs[5] != 0) {
    privatedata.convolutionprocessor.backright.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::BACKRIGHT], rightIRs[hear360_dsp_high_hrirfolddown::BACKRIGHT], irFrames);
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[6] != 0 && rightIRs[6] != 0) {
    privatedata.convolutionprocessor.sideleft.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::SIDELEFT], rightIRs[hear360_dsp_high_hrirfolddown::SIDELEFT], irFrames);
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(irFrames);
  }

  if(leftIRs[7] != 0 && rightIRs[7] != 0) {
    privatedata.convolutionprocessor.sideright.loadIRs(leftIRs[hear360_dsp_high_hrirfolddown::SIDERIGHT], rightIRs[hear360_dsp_high_hrirfolddown::SIDERIGHT], irFrames);
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(irFrames);
  }

  privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
}

//######################################################################################################################

void PROCESSOR::LoadIRsFromPresets(int presetID)
{
#ifdef HPS_CONVOLUTION_IR_DEFAULT_SET
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

    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_default::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_default::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_default::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_default::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SET
  if(presetID == 1)
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

    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_soundfi::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_soundfi::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_soundfi::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SET
  if(presetID == 2)
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

    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_soundfi2::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_soundfi2::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SET
  if(presetID == 3)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi3::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi3::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi3::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi3::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi3::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_soundfi3::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_soundfi3::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_soundfi3::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SET
  if(presetID == 4)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_hybridc::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_hybridc::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_hybridc::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_hybridc::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_hybridc::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideleft.loadIRs(hear360_convolution_ir_hybridc::ir_sideleft_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_sideleft_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideleft.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.sideright.loadIRs(hear360_convolution_ir_hybridc::ir_sideright_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc::ir_sideright_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.sideright.setEffectiveIRFrames(hear360_convolution_ir_hybridc::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif

#ifdef HPS_CONVOLUTION_IR_DEFAULT_SKY_SET
  //default
  if(presetID == 100)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_default_sky::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_default_sky::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_default_sky::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_default_sky::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_default_sky::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_default_sky::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_default_sky::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_default_sky::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_default_sky::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_default_sky::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_default_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SKY_SET
  if(presetID == 101)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi_sky::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi_sky::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi_sky::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi_sky::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi_sky::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SKY_SET
  if(presetID == 102)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi2_sky::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi2_sky::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi2_sky::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi2_sky::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi2_sky::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi2_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SKY_SET
  if(presetID == 103)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_soundfi3_sky::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_soundfi3_sky::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_soundfi3_sky::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_soundfi3_sky::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_soundfi3_sky::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_soundfi3_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SKY_SET
  if(presetID == 104)
  {
    privatedata.convolutionprocessor.frontleft.loadIRs(hear360_convolution_ir_hybridc_sky::ir_left_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::ir_left_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontleft.setEffectiveIRFrames(hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontright.loadIRs(hear360_convolution_ir_hybridc_sky::ir_right_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::ir_right_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontright.setEffectiveIRFrames(hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.frontcenter.loadIRs(hear360_convolution_ir_hybridc_sky::ir_center_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::ir_center_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.frontcenter.setEffectiveIRFrames(hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backleft.loadIRs(hear360_convolution_ir_hybridc_sky::ir_rearleft_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::ir_rearleft_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backleft.setEffectiveIRFrames(hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.convolutionprocessor.backright.loadIRs(hear360_convolution_ir_hybridc_sky::ir_rearright_l(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::ir_rearright_r(privatedata.mSamplerate), hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));
    privatedata.convolutionprocessor.backright.setEffectiveIRFrames(hear360_convolution_ir_hybridc_sky::HPS_CONVOLUTION_IR_LEN(privatedata.mSamplerate));

    privatedata.delayLFE.SetDelaySamples(hear360_dsp_os_memory_AUDIOBUFFERSIZE / 2);
  }
#endif
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

  //Offset by sampling rate (linear based on 48000)
  float sampleRateGain = (float)privatedata.mSamplerate / 48000.0f;
  privatedata.gainfactor.frontLeft *= sampleRateGain;
  privatedata.gainfactor.frontRight *= sampleRateGain;
  privatedata.gainfactor.center *= sampleRateGain;
  //No FLE processing
  privatedata.gainfactor.backLeft *= sampleRateGain;
  privatedata.gainfactor.backRight *= sampleRateGain;
  privatedata.gainfactor.sideLeft *= sampleRateGain;
  privatedata.gainfactor.sideRight *= sampleRateGain;

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
/*
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
  */

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
