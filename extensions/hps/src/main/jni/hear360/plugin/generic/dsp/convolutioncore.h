
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_convolutioncore_H
#define hear360_plugin_generic_dsp_convolutioncore_H

//######################################################################################################################

#include <hear360/dsp/high/convolutioncore.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_convolutioncore
{

//######################################################################################################################

  void* CreateInstance(int samplerate);
  bool DeleteInstance(void* handle);
  void loadIRs(void* handle, unsigned int channelID);
  void loadIRFromInts(void* handle, int* leftIRs, int* rightIRs, int irFrames);
  bool ProcessOutOfPlace(void* handle, hear360_dsp_high_convolutioncore::PARAMETERS *pparameters, const float* pInBuf, float** pOutbuf, long totalsamples);

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
