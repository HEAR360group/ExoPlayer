
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_hrirfolddown_H
#define hear360_plugin_generic_dsp_hrirfolddown_H

//######################################################################################################################

#include <hear360/dsp/high/hrirfolddownsimple.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_hrirfolddown
{

//######################################################################################################################

  void* CreateInstance(int samplerate);
  bool DeleteInstance(void* handle);
  bool LoadIRsFromPresets(void* handle, int presetID);
  bool ProcessInPlace(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, float** pBuf, int srcChannels, long totalsamples);
  bool ProcessInPlaceInterleaved(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, float* pBuf, int srcChannels, long totalsamples);
  bool ProcessOutOfPlace(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, const float** pInBuf, float** pOutbuf, int dstChannels, int srcChannels, long totalsamples);
  bool ProcessOutOfPlaceInterleaved(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, const float* pInBuf, float* pOutbuf, int srcChannels, long totalsamples);

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
