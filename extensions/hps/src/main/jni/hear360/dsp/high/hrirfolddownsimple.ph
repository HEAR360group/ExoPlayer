
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################
/*

Simple Folddown processor

While hear360_dsp_high_folddown lets you customize lots of settings,
  this simple folddown module already has the settings preset into individual modes.
Just call Update() to enable/disable the processor and choose the mode.

*/
//######################################################################################################################

#ifndef hear360_dsp_high_hrirfolddownsimple_PH
#define hear360_dsp_high_hrirfolddownsimple_PH

//######################################################################################################################

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/high/hrirfolddown.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddownsimple
{

//######################################################################################################################

struct PRIVATE
{
  PARAMETERS simpleparameters;

  hear360_dsp_high_hrirfolddown::PROCESSOR folddownprocessor;

  hear360_dsp_high_hrirfolddown::PARAMETERS pparameters [hear360_dsp_high_hrirfolddownsimple_TOTALMODES];

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
