
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_high_hrirfolddown_PH
#define hear360_dsp_high_hrirfolddown_PH

//######################################################################################################################

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/monoequalizer.h>
#include <hear360/dsp/low/stereoequalizer.h>
#include <hear360/algr/Convolution/CRFilter.h>
#include <hear360/algr/Delay/HPSStaticDelay.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddown
{

//######################################################################################################################

#define hear360_dsp_high_folddown_AUDIOBUFFERSIZE (1024)

//######################################################################################################################
/*
struct DIRECTIONPARAMETERS
{
  hear360_dsp_low_direction::PARAMETERS frontleft;
  hear360_dsp_low_direction::PARAMETERS frontright;
  hear360_dsp_low_direction::PARAMETERS frontcenter;
  hear360_dsp_low_direction::PARAMETERS backleft;
  hear360_dsp_low_direction::PARAMETERS backright;

  DIRECTIONPARAMETERS ();
};
*/
//######################################################################################################################

struct CONVOLUTIONPROCESSOR
{
  hear360_algr::CRFilter frontleft;
  hear360_algr::CRFilter frontright;
  hear360_algr::CRFilter frontcenter;
  hear360_algr::CRFilter backleft;
  hear360_algr::CRFilter backright;
  hear360_algr::CRFilter sideleft;
  hear360_algr::CRFilter sideright;
  hear360_algr::CRFilter lfe;

  CONVOLUTIONPROCESSOR(hear360_dsp_os_memory::MANAGER memorymanager);
};

//######################################################################################################################

struct EQUALIZERPROCESSOR
{
  hear360_dsp_low_stereoequalizer::PROCESSOR warm;
  hear360_dsp_low_monoequalizer::PROCESSOR lfe;
};

//######################################################################################################################

struct BUFFER
{
  float** inputbus;
  float** outputbus;

  float** frontbus;
  float** centerbus;
  float** rearbus;
  float** sidebus;

  float** lfebus;
};

//######################################################################################################################

struct PRIVATE
{
  hear360_dsp_os_memory::MANAGER memorymanager;

  bool enable;
  bool enableeq;
  long curoffset;
  float irtailpercentage;
  int mSamplerate;

  GAINPARAMETERS gainfactor;

  EQUALIZERPROCESSOR equalizerprocessor;

  HPSStaticDelay delayLFE;
  BUFFER buffer;

  CONVOLUTIONPROCESSOR convolutionprocessor;

  //PRIVATE ();
  ~PRIVATE();

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
