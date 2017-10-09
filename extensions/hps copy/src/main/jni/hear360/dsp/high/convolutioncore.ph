
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_high_convolutioncore_PH
#define hear360_dsp_high_convolutioncore_PH

//######################################################################################################################

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/monoequalizer.h>
#include <hear360/dsp/low/stereoequalizer.h>
#include <hear360/algr/Convolution/CRFilter.h>

//######################################################################################################################

namespace hear360_dsp_high_convolutioncore
{

//######################################################################################################################

#define hear360_dsp_high_convolutioncore_AUDIOBUFFERSIZE (1024)

//######################################################################################################################

struct CONVOLUTIONPROCESSOR
{
  hear360_algr::CRFilter crfilter;

  CONVOLUTIONPROCESSOR(hear360_dsp_os_memory::MANAGER memorymanager);
};

//######################################################################################################################

struct BUFFER
{
  float* inputbus;
  float** outputbus;
};

//######################################################################################################################

struct PRIVATE
{
  hear360_dsp_os_memory::MANAGER memorymanager;

  long curoffset;
  float irtailpercentage;

  BUFFER buffer;

  CONVOLUTIONPROCESSOR convolutionprocessor;

  //PRIVATE ();
  ~PRIVATE();

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager);
};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
