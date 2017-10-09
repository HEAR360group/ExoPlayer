
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_high_convolutioncore_H
#define hear360_dsp_high_convolutioncore_H

//######################################################################################################################

#include <hear360/dsp/private.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/monoequalizer.h>
#include <hear360/dsp/low/stereoequalizer.h>
#include <hear360/algr/Equalizer/HPSEqualizer4Band.h>

//######################################################################################################################

namespace hear360_dsp_high_convolutioncore
{

//######################################################################################################################

} // namespace

// Delayed include because we need GAINPARAMETERS from above.

#include <hear360/dsp/high/convolutioncore.ph>

namespace hear360_dsp_high_convolutioncore
{

//######################################################################################################################

struct PARAMETERS
{
  float irtailpercentage;

  PARAMETERS ();
};

//######################################################################################################################

class PROCESSOR
{

public:

  //##############################################################################

  PROCESSOR (hear360_dsp_os_memory::MANAGER memorymanager);

  //##############################################################################

  void Reset (void);

  // Call to reset the state of the object to what it was when it was first instantiated.

  //##############################################################################

  bool Update (const PARAMETERS *pparameters);

  // Call this as often as you like in between calls to Process*().
  // You need to call this at least once before the first call to Process*().

  // If enable is false, pparameters is ignored.

  // The return bool indicates an error, true = error, false = no error.
  // If the parameters are invalid, it will disable some processing features and return true.
  // You can still call Process*() after a failed Update(), The disabled features won't be enabled until
  //   the next successful call to Update().

  //##############################################################################

  void LoadIRFromInts(int* leftIRs, int* rightIRs, unsigned int irFrames);

  //##############################################################################

  void LoadIRs(unsigned int channelsFlag, int samplerate);

  //##############################################################################

  void Process
  (
    float *ppdstaudio[2],                       // stereo destination audio
    float *ppsrcaudio,          // mono channel source audio
    long totalsamples
  );

  // This function reads totalsamples from ppsrcaudio, and writes totalsamples into ppdstaudio.
  // The destination audio buffers may the same as two of the source audio buffers.

  // When processing is disabled, it will just copy the front left and front right channels to
  //   the stereo destination.

  // The six channels are ordered according to the CHANNEL enum defined at the top of this header.

  //##############################################################################

  hear360_dsp_private_LOCAL (PROCESSOR)

  //##############################################################################

};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
