
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <hear360/plugin/generic/dll/hps-headtracking.h>
#include <hear360/plugin/generic/dsp/vsheadtracking.h>

//######################################################################################################################

#ifdef __cplusplus
extern "C"
{
#endif
  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_getVolumeMatrix(HPS_Headtracking_Instance_Handle handle, float* outVolumeMatrix)
  {
    hear360_plugin_generic_dsp_vsheadtracking::getVolumeMatrix(handle, outVolumeMatrix);
    return 0;
  }

  F_DECLSPEC F_DLLEXPORT HPS_Headtracking_Instance_Handle HPS_Headtracking_CreateInstance(int samplerate, bool isHeight)
  {
    return hear360_plugin_generic_dsp_vsheadtracking::CreateInstance(samplerate, isHeight);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_DeleteInstance(HPS_Headtracking_Instance_Handle handle)
  {
	  return (hear360_plugin_generic_dsp_vsheadtracking::DeleteInstance(handle) ? 1 : 0);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_ProcessOutOfPlace(HPS_Headtracking_Instance_Handle handle, float azimuth, float** ppInputBuf, float** ppOutputBuf, int inputChCount, int bufLength)
  {
	   return (hear360_plugin_generic_dsp_vsheadtracking::ProcessOutOfPlace(handle, azimuth, (const float**)ppInputBuf, ppOutputBuf, inputChCount, bufLength) ? 1 : 0);
  }

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_ProcessOutOfPlaceInterleaved(HPS_Headtracking_Instance_Handle handle, float azimuth, float* pInputBuf, float* pOutputBuf, int inputChCount, int bufLength)
  {
    return (hear360_plugin_generic_dsp_vsheadtracking::ProcessOutOfPlaceInterleaved(handle, azimuth, (const float*)pInputBuf, pOutputBuf, inputChCount, bufLength) ? 1 : 0);
  }
#ifdef __cplusplus
}
#endif
//######################################################################################################################
