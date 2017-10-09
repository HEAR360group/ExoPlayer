
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <hear360/plugin/generic/dsp/interleave.h>

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
)
{
  if (dstchannels >= 2)
  {
    for (long sx = 0 ; sx < totalsamples ; sx++)
    {
      pdstaudiointerleaved [sx * dstchannels]     = ppsrcaudio[0][sx] * 2;  //gain by 6db
      pdstaudiointerleaved [sx * dstchannels + 1] = ppsrcaudio[1][sx] * 2;  //gain by 6db
    }

    for (long sx = 0 ; sx < totalsamples ; sx++)
    {
      for (long ex = 2 ; ex < dstchannels ; ex++)
      {
        pdstaudiointerleaved [sx * dstchannels + ex] = 0;
      }
    }
  }
  else if (dstchannels == 1)
  {
    for (long sx = 0 ; sx < totalsamples ; sx++)
    {
      pdstaudiointerleaved [sx] = (ppsrcaudio[0][sx] + ppsrcaudio[1][sx]);
    }
  }

  return;
}

//######################################################################################################################

void DeinterleaveMultiToMono
(
  float *pdstaudio,
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
)
{
  if (validsamples)
  {
    if (totalsrcchannels == 1)
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        pdstaudio[sx] = psrcaudiointerleaved [sx];
      }
    }
    else
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        pdstaudio[sx] = (psrcaudiointerleaved [sx * totalsrcchannels] + psrcaudiointerleaved [sx * totalsrcchannels + 1]) / 2;
      }
    }
  }
  else
  {
    for (long sx = 0 ; sx < totalsamples ; sx++)
    {
      pdstaudio[sx] = 0;
    }
  }

  return;
}

//######################################################################################################################

void DeinterleaveMultiToStereo
(
  float *ppdstaudio[2],
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
)
{
  if (validsamples && (totalsrcchannels >= 2))
  {
    for (long sx = 0 ; sx < totalsamples ; sx++)
    {
      ppdstaudio[0][sx] = psrcaudiointerleaved [sx * totalsrcchannels ];
      ppdstaudio[1][sx] = psrcaudiointerleaved [sx * totalsrcchannels + 1];
    }
  }
  else
  {
    for (int cx = 0 ; cx < 2 ; cx++)
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        ppdstaudio[cx][sx] = 0;
      }
    }
  }

  return;
}

//######################################################################################################################

void DeinterleaveMultiTo6Channels
(
  float **ppdstaudio,
  const float *psrcaudiointerleaved,
  int totalsrcchannels,
  long totalsamples,
  bool validsamples
)
{
  // This function needs to be updated to use the correct constants on both sides of the equals sign.

  if (validsamples && (totalsrcchannels >= 2))
  {
    for (int cx = 0 ; (cx < 8) && (cx < totalsrcchannels) ; cx++)
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        ppdstaudio[cx][sx] = psrcaudiointerleaved [sx * totalsrcchannels + cx];
      }
    }

    for (int cx = totalsrcchannels ; cx < 8 ; cx++)
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        ppdstaudio[cx][sx] = 0;
      }
    }
  }
  else
  {
    for (int cx = 0 ; cx < totalsrcchannels ; cx++)
    {
      for (long sx = 0 ; sx < totalsamples ; sx++)
      {
        ppdstaudio[cx][sx] = 0;
      }
    }
  }

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
