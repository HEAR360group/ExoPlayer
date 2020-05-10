
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_stereoupmix51_H
#define hear360_plugin_generic_dsp_stereoupmix51_H

//######################################################################################################################

//######################################################################################################################

namespace hear360_plugin_generic_dsp_stereoupmix51
{

//######################################################################################################################

//######################################################################################################################

  void* CreateInstance(int samplerate);
  bool DeleteInstance(void* handle);
  //bool ProcessInPlace(void* handle, float azimuth, float** pBuf, int srcChannels, long totalsamples);
  //bool ProcessInPlaceInterleaved(void* handle, float azimuth, float* pBuf, int srcChannels, long totalsamples);
  bool ProcessOutOfPlace(void* handle, const float** pInBuf, float** pOutBuf, int srcChannels, int dstChannels, long totalsamples);
  bool ProcessOutOfPlaceInterleaved(void* handle, const float* pInBuf, float* pOutBuf, int srcChannels, int dstChannels, long totalsamples);

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
