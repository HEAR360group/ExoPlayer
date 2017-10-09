
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

#ifndef hear360_dsp_high_hrirfolddownsimple_H
#define hear360_dsp_high_hrirfolddownsimple_H

//######################################################################################################################

#define hear360_dsp_high_hrirfolddownsimple_TOTALMODES (1)

//######################################################################################################################

#include <hear360/dsp/private.h>
#include <hear360/dsp/os/memory.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddownsimple
{

//######################################################################################################################

struct PARAMETERS
{
  bool enable;                          // Everything below is only defined if enable is true.

  int mode;                             // Must satisfy 0 <= mode < TOTALMODES.

  bool enableeq;

  float irtailpercentage;

  //float distance;                       // From 0 to 100
  //float reverbcofactor;                 // From 0 to 100

  //bool enhancedistanceeffect;

  PARAMETERS ();
};

//######################################################################################################################

} // namespace

// Delayed include because we need PARAMETERS from above.

#include <hear360/dsp/high/hrirfolddownsimple.ph>

namespace hear360_dsp_high_hrirfolddownsimple
{

//######################################################################################################################

class PROCESSOR
{

public:

  //##############################################################################

  PROCESSOR (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);

  //##############################################################################

  void Reset (void);

  // Call to reset the state of the object to what it was when it was first instantiated.

  //##############################################################################

  void LoadIRsFromPresets(int presetID);

  //##############################################################################

  bool Update (const PARAMETERS *pparameters);

  // Call this as often as you like in between calls to Process*().
  // You need to call this at least once before the first call to Process*().

  // The return bool indicates an error, true = error, false = no error.
  // If the mode number is invalid, it will disable processing and return true.
  // You can still call Process*() after a failed Update(), processing will just be disabled until
  //   the next successful call to Update().

  //##############################################################################

  void Process6ChannelsToStereo
  (
    float *ppdstaudio[2],                       // stereo destination audio
    float * ppsrcaudio[6],          // 6 channel source audio
    int totalsrcchannels,
    long totalsamples
  );

  // This function reads totalsamples from ppsrcaudio, and writes totalsamples into ppdstaudio.
  // The destination audio buffers may the same as two of the source audio buffers.

  // When processing is disabled, it will just copy the front left and front right channels to
  //   the stereo destination.

  // The six channels are ordered according to the CHANNEL enum defined in the hear360_dsp_high_folddown header.

  //##############################################################################

  hear360_dsp_private_LOCAL (PROCESSOR)

  //##############################################################################

};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
