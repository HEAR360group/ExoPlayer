
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <hear360/plugin/generic/dll/hps-upmix51.h>
#include <hear360/plugin/generic/dsp/stereoupmix51.h>

//######################################################################################################################

#ifdef __cplusplus
extern "C"
{
#endif
  F_DECLSPEC F_DLLEXPORT HPS_Upmix51_Instance_Handle HPS_Upmix51_CreateInstance(int samplerate)
  {
    return hear360_plugin_generic_dsp_stereoupmix51::CreateInstance(samplerate);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Upmix51_DeleteInstance(HPS_Upmix51_Instance_Handle handle)
  {
	  return (hear360_plugin_generic_dsp_stereoupmix51::DeleteInstance(handle) ? 1 : 0);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Upmix51_ProcessOutOfPlace(HPS_Upmix51_Instance_Handle handle, float** ppInputBuf, float** ppOutputBuf, int srcChCount, int dstChCount, int bufLength)
  {
	   return (hear360_plugin_generic_dsp_stereoupmix51::ProcessOutOfPlace(handle, (const float**)ppInputBuf, ppOutputBuf, srcChCount, dstChCount, bufLength) ? 1 : 0);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Upmix51_ProcessOutOfPlaceInterleaved(HPS_Upmix51_Instance_Handle handle, float* pInputBuf, float* pOutputBuf, int srcChCount, int dstChCount, int bufLength)
  {
    return (hear360_plugin_generic_dsp_stereoupmix51::ProcessOutOfPlaceInterleaved(handle, (const float*)pInputBuf, pOutputBuf, srcChCount, dstChCount, bufLength) ? 1 : 0);
  }
#ifdef __cplusplus
}
#endif
//######################################################################################################################
