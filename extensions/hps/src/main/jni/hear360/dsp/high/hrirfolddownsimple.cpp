
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <cstddef>

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/equalizerband.h>
#include <hear360/dsp/high/hrirfolddown.h>
#include <hear360/dsp/high/hrirfolddownsimple.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddownsimple
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

PARAMETERS::PARAMETERS ()
:
  enable (false),
  //mode (0),

  enableeq (false),
  irtailpercentage (100.0f)

  //distance (0),
  //reverbcofactor (0),

  //enhancedistanceeffect (false)
{}

//######################################################################################################################
/*
Original parameters given to me on 2013 January for the wwise plugin.
*/
//######################################################################################################################
/*
Parameters given to me on 2013 May for the wwise plugin.
*/

static void SetParameter (hear360_dsp_high_hrirfolddown::PARAMETERS *pparameters, bool enableEQ, float masterGain, float* gains)
{
  using namespace hear360_dsp_low_equalizerband;

  pparameters->gaindb.master = masterGain;

  if(gains != 0)
  {
    pparameters->gaindb.frontLeft = gains[hear360_dsp_high_hrirfolddown::FRONTLEFT];
    pparameters->gaindb.frontRight = gains[hear360_dsp_high_hrirfolddown::FRONTRIGHT];
    pparameters->gaindb.center = gains[hear360_dsp_high_hrirfolddown::FRONTCENTER];
    pparameters->gaindb.sideLeft = gains[hear360_dsp_high_hrirfolddown::SIDELEFT];
    pparameters->gaindb.sideRight = gains[hear360_dsp_high_hrirfolddown::SIDERIGHT];
    //pparameters->gaindb.lfe = hear360_convolution_ir_default::GAIN_LFE;
    pparameters->gaindb.backLeft = gains[hear360_dsp_high_hrirfolddown::BACKLEFT];
    pparameters->gaindb.backRight = gains[hear360_dsp_high_hrirfolddown::BACKRIGHT];
  }

  pparameters->equalizer.warm.enable = enableEQ;
  pparameters->equalizer.lfe.enable = false;

  pparameters->applylegacygains = false;

  pparameters->equalizer.warm.pband[0] = BAND (10.0f, 1.0f, 0.0f);
  pparameters->equalizer.warm.pband[1] = BAND(94.6f, 2.0f, 1.0f);
  pparameters->equalizer.warm.pband[2] = BAND(2000.0f, 0.25f, -3.0f);
  pparameters->equalizer.warm.pband[3] = BAND(20000.0f, 2.0f, -1.0f);

  pparameters->equalizer.lfe.pband = BAND(140.0f, 1.41f, 0.0f);

  return;
}

static void ParameterPreset201305 (hear360_dsp_high_hrirfolddown::PARAMETERS *pparameters, int presetID)
{
  using namespace hear360_dsp_low_equalizerband;

#ifdef HPS_CONVOLUTION_IR_DEFAULT_SET
  if(presetID == 0)
  {
    pparameters->gaindb.master = hear360_convolution_ir_default::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_default::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_default::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_default::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_default::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_default::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_default::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_default::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_default::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = true;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SET
  if(presetID == 1)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SET
  if(presetID == 2)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi2::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi2::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi2::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi2::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi2::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi2::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi2::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi2::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi2::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SET
  if(presetID == 3)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi3::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi3::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi3::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi3::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi3::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi3::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi3::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi3::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi3::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SET
  if(presetID == 4)
  {
    pparameters->gaindb.master = hear360_convolution_ir_hybridc::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_hybridc::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_hybridc::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_hybridc::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_hybridc::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_hybridc::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_hybridc::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_hybridc::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_hybridc::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_DEFAULT_SKY_SET
  if(presetID == 100)
  {
    pparameters->gaindb.master = hear360_convolution_ir_default_sky::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_default_sky::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_default_sky::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_default_sky::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_default_sky::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_default_sky::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_default_sky::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_default_sky::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_default_sky::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = true;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI_SKY_SET
  if(presetID == 101)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi_sky::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi_sky::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi_sky::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi_sky::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi_sky::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi_sky::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi_sky::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi_sky::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi_sky::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI2_SKY_SET
  if(presetID == 102)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi2_sky::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi2_sky::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi2_sky::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi2_sky::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi2_sky::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi2_sky::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi2_sky::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi2_sky::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi2_sky::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_SOUNDFI3_SKY_SET
  if(presetID == 103)
  {
    pparameters->gaindb.master = hear360_convolution_ir_soundfi3_sky::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_soundfi3_sky::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_soundfi3_sky::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_soundfi3_sky::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_soundfi3_sky::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_soundfi3_sky::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_soundfi3_sky::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_soundfi3_sky::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_soundfi3_sky::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
#ifdef HPS_CONVOLUTION_IR_HYBRIDC_SKY_SET
  if(presetID == 104)
  {
    pparameters->gaindb.master = hear360_convolution_ir_hybridc_sky::GAIN_MASTER;
    pparameters->gaindb.frontLeft = hear360_convolution_ir_hybridc_sky::GAIN_FRONT_LEFT;
    pparameters->gaindb.frontRight = hear360_convolution_ir_hybridc_sky::GAIN_FRONT_RIGHT;
    pparameters->gaindb.center = hear360_convolution_ir_hybridc_sky::GAIN_CENTER;
    pparameters->gaindb.sideLeft = hear360_convolution_ir_hybridc_sky::GAIN_SIDE_LEFT;
    pparameters->gaindb.sideRight = hear360_convolution_ir_hybridc_sky::GAIN_SIDE_RIGHT;
    pparameters->gaindb.lfe = hear360_convolution_ir_hybridc_sky::GAIN_LFE;
    pparameters->gaindb.backLeft = hear360_convolution_ir_hybridc_sky::GAIN_BACK_LEFT;
    pparameters->gaindb.backRight = hear360_convolution_ir_hybridc_sky::GAIN_BACK_RIGHT;

    pparameters->equalizer.warm.enable = false;
    pparameters->equalizer.lfe.enable = false;
  }
#endif
/*
  //HRIR
  pparameters->gaindb.master = 10.5f;
#if (!defined(HPS_CONVOLUTION_IR_ADDITION_5_CHANNELS))
  pparameters->gaindb.frontLeft = 7.0f;
  pparameters->gaindb.frontRight = 7.5f;
  pparameters->gaindb.center = 3.5f;
  pparameters->gaindb.sideLeft = 0.0f;
  pparameters->gaindb.sideRight = 0.0f;
  pparameters->gaindb.lfe = -18.0f;
  pparameters->gaindb.backLeft = 3.5f;
  pparameters->gaindb.backRight = 6.0f;
#else
  pparameters->gaindb.frontLeft = -0.0f;
  pparameters->gaindb.frontRight = -0.0f;
  pparameters->gaindb.center = -1.5f;
  pparameters->gaindb.sideLeft = -0.0f;
  pparameters->gaindb.sideRight = -0.0f;
  pparameters->gaindb.lfe = -18.0f;
  pparameters->gaindb.backLeft = -0.0f;
  pparameters->gaindb.backRight = -0.0f;
#endif

  pparameters->equalizer.warm.enable = true;
  pparameters->equalizer.lfe.enable = false;
*/
  pparameters->applylegacygains = false;

  pparameters->equalizer.warm.pband[0] = BAND (10.0f, 1.0f, 0.0f);
  pparameters->equalizer.warm.pband[1] = BAND(94.6f, 2.0f, 1.0f);
  pparameters->equalizer.warm.pband[2] = BAND(2000.0f, 0.25f, -3.0f);
  pparameters->equalizer.warm.pband[3] = BAND(20000.0f, 2.0f, -1.0f);

  pparameters->equalizer.lfe.pband = BAND(140.0f, 1.41f, 0.0f);

  return;
}

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate)
:
  folddownprocessor (memorymanager, samplerate)
{
  //ParameterPreset201305 (&pparameters[0]);

  return;
}

//######################################################################################################################

void CheckIfEqual (bool *pequal, const PARAMETERS *pparameters1, const PARAMETERS *pparameters2)
{
  if (pparameters1->enable                != pparameters2->enable) goto NOTEQUAL;
  if (pparameters1->mode                  != pparameters2->mode) goto NOTEQUAL;
  if (pparameters1->enableeq              != pparameters2->enableeq) goto NOTEQUAL;
  if (pparameters1->irtailpercentage      != pparameters2->irtailpercentage) goto NOTEQUAL;
  //if (pparameters1->reverbcofactor        != pparameters2->reverbcofactor) goto NOTEQUAL;
  //if (pparameters1->enhancedistanceeffect != pparameters2->enhancedistanceeffect) goto NOTEQUAL;

  *pequal = true;

  return;

NOTEQUAL:

  *pequal = false;

  return;
}

//######################################################################################################################

/*
void UpdateDistanceParameters (hear360_dsp_high_hrirfolddown::PARAMETERS *pfullparameters, const PARAMETERS *pparameters)
{
  pfullparameters->distance.attenuation = false;

  pfullparameters->distance.attenuationtype = hear360_dsp_low_distance::ATTENUATIONTYPE_LINEARSQUARED;

  pfullparameters->distance.reverb = pparameters->enabledistance;

  pfullparameters->distance.model = hear360_dsp_low_distance::MODEL_SIMPLE;

  pfullparameters->distance.reverbtype =
    (
      pparameters->enhancedistanceeffect ?
      hear360_dsp_low_distance::REVERBTYPE_STANDARD :
      hear360_dsp_low_distance::REVERBTYPE_FAST
    );

  pfullparameters->distance.distance         = pparameters->distance;
  pfullparameters->distance.reverbcofactor   = pparameters->reverbcofactor;

  return;
}
*/
//######################################################################################################################

void PROCESSOR::LoadIRsFromFloats(float** leftIRs, float** rightIRs, bool enableEQ, float masterGain, float* gains, unsigned int irFrames)
{
  privatedata.folddownprocessor.LoadIRsFromFloats(leftIRs, rightIRs, irFrames);

  SetParameter(&privatedata.pparameters[0], enableEQ, masterGain, gains);
}

void PROCESSOR::LoadIRsFromInts(int** leftIRs, int** rightIRs, bool enableEQ, float masterGain, float* gains, unsigned int irFrames)
{
  privatedata.folddownprocessor.LoadIRsFromInts(leftIRs, rightIRs, irFrames);

  SetParameter(&privatedata.pparameters[0], enableEQ, masterGain, gains);
}

void PROCESSOR::LoadIRsFromPresets(int presetID)
{
  privatedata.folddownprocessor.LoadIRsFromPresets (presetID);

  ParameterPreset201305 (&privatedata.pparameters[0], presetID);
}

bool PROCESSOR::Update (const PARAMETERS *pparameters)
{
  bool equal;
  hear360_dsp_high_hrirfolddown::PARAMETERS fullparameters;

  if ((!pparameters->enable) && (!privatedata.simpleparameters.enable)) return (false);

  CheckIfEqual (&equal, pparameters, &privatedata.simpleparameters);

  if (equal) return (false);

  privatedata.simpleparameters = *pparameters;

  if (!pparameters->enable)
  {
    privatedata.folddownprocessor.Update (false, NULL);

    return (false);
  }

  if ((pparameters->mode < 0) || (pparameters->mode >= hear360_dsp_high_hrirfolddownsimple_TOTALMODES))
  {
    privatedata.folddownprocessor.Update (false, NULL);

    privatedata.simpleparameters.enable = false;

    return (true);
  }

  fullparameters = privatedata.pparameters[pparameters->mode];

  fullparameters.enableeq = pparameters->enableeq;

  fullparameters.irtailpercentage = pparameters->irtailpercentage;

  //UpdateDistanceParameters (&fullparameters, pparameters);

  privatedata.folddownprocessor.Update (true, &fullparameters);

  return (false);
}

//######################################################################################################################

void PROCESSOR::Reset (void)
{
  privatedata.folddownprocessor.Reset();

  privatedata.folddownprocessor.Update (false, NULL);

  privatedata.simpleparameters.enable = false;

  return;
}

//######################################################################################################################

void PROCESSOR::Process6ChannelsToStereo
(
  float *ppdstaudio[2],                         // stereo destination audio
  float * ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],            // 6 channel source audio
  int totalsrcchannels,
  long totalsamples
)
{
  privatedata.folddownprocessor.Process6ChannelsToStereo (ppdstaudio, ppsrcaudio, totalsrcchannels, totalsamples);

  return;
}

//######################################################################################################################

PROCESSOR::PROCESSOR (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate)
:
  privatedata (memorymanager, samplerate)
{
  Reset();

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
