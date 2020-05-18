
//######################################################################################################################

// © 2019 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_stereoeq_H
#define hear360_plugin_generic_dsp_stereoeq_H

//######################################################################################################################

//######################################################################################################################

namespace hear360_plugin_generic_dsp_stereoeq
{
  void* CreateInstance(int samplerate);
  bool DeleteInstance(void* handle);
  bool Update (void* handle, const float* eqF, const float* eqG, const float* eqQ);
  bool ProcessInPlace(void* handle, float** pBuf, long totalsamples);
  bool ProcessInPlaceInterleaved(void* handle, float* pBuf, long totalsamples);
} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
