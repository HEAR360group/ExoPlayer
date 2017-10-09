
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_interleave_H
#define hear360_plugin_generic_dsp_interleave_H

//######################################################################################################################

namespace hear360_plugin_generic_dsp_interleave
{

//######################################################################################################################

void InterleaveStereoToMulti
(
  float *pdstaudiointerleaved,
  const float * const ppsrcaudio[2],
  int dstchannels,
  long totalsamples
);
// If the output has more than 2 channels, all extra channels will be muted.
// If the output has only 1 channel, the average of the 2 channels will be saved.

void DeinterleaveMultiToMono
(
  float *pdstaudio,
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
);
// If the input is mono, it just copies it.
// If the input has 2 or more channels, it takes the average of the first 2 channels.

void DeinterleaveMultiToStereo
(
  float *ppdstaudio[2],
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
);
// If the input has less than 2 channels, it will mute it.
// If the input has 2 or more channels, it takes the first 2.

void DeinterleaveMultiTo6Channels
(
  float *ppdstaudio[6],
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
);
// If the input has less than 2 channels, it will mute it.
// If the input has 2 or more channels, it takes the first 6 available.

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
