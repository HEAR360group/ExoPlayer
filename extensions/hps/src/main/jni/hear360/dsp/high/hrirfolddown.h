
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_high_hrirfolddown_H
#define hear360_dsp_high_hrirfolddown_H

//######################################################################################################################

#include <hear360/dsp/private.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/monoequalizer.h>
#include <hear360/dsp/low/stereoequalizer.h>
#include <hear360/algr/Equalizer/HPSEqualizer4Band.h>

//######################################################################################################################

namespace hear360_dsp_high_hrirfolddown
{

//######################################################################################################################

typedef enum
{
  FRONTLEFT,
  FRONTRIGHT,
  FRONTCENTER,
  LFE,
  BACKLEFT,
  BACKRIGHT,
  SIDELEFT,
  SIDERIGHT,
  MAXCHANNELS
}
CHANNEL;

//######################################################################################################################

struct GAINPARAMETERS
{
  float master;

  float frontLeft;
  float frontRight;
  float center;
  float lfe;
  float backLeft;
  float backRight;
  float sideLeft;
  float sideRight;

  GAINPARAMETERS ();
};

//######################################################################################################################

} // namespace

// Delayed include because we need GAINPARAMETERS from above.

#include <hear360/dsp/high/hrirfolddown.ph>

namespace hear360_dsp_high_hrirfolddown
{

//######################################################################################################################

struct EQUALIZERPARAMETERS
{
  hear360_dsp_low_stereoequalizer::PARAMETERS warm;
  hear360_dsp_low_monoequalizer::PARAMETERS lfe;
};

//######################################################################################################################

struct PARAMETERS
{
  GAINPARAMETERS gaindb;

  float irtailpercentage;

  bool applylegacygains;        // Applies gains that are hard coded in some implementations.

  bool enableeq;

  EQUALIZERPARAMETERS equalizer;

  //hear360_dsp_low_distance::PARAMETERS distance;

  PARAMETERS ();
};

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

  bool Update (bool enable, const PARAMETERS *pparameters);

  // Call this as often as you like in between calls to Process*().
  // You need to call this at least once before the first call to Process*().

  // If enable is false, pparameters is ignored.

  // The return bool indicates an error, true = error, false = no error.
  // If the parameters are invalid, it will disable some processing features and return true.
  // You can still call Process*() after a failed Update(), The disabled features won't be enabled until
  //   the next successful call to Update().

  //##############################################################################

  void Process6ChannelsToStereo
  (
    float *ppdstaudio[2],                       // stereo destination audio
    float *ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS],          // 6 channel source audio
    int srcchannels,
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
