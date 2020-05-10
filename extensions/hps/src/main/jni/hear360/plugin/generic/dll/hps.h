
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_geneic_dll_hps_H
#define hear360_plugin_geneic_dll_hps_H

/*
    Compiler specific settings.
*/

#if defined(__CYGWIN32__)
    #define F_CDECL __cdecl
    #define F_STDCALL __stdcall
    #define F_DECLSPEC __declspec
    #define F_DLLEXPORT ( dllexport )
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
    #define F_CDECL _cdecl
    #define F_STDCALL _stdcall
    #define F_DECLSPEC __declspec
    #define F_DLLEXPORT ( dllexport )
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
    #define F_CDECL
    #define F_STDCALL
    #define F_DECLSPEC
    #define F_DLLEXPORT __attribute__ ((visibility("default")))
#else
    #define F_CDECL
    #define F_STDCALL
    #define F_DECLSPEC
    #define F_DLLEXPORT
#endif

//######################################################################################################################

#ifdef __cplusplus
extern "C"
{
#endif
  typedef void * Hear360_HPS_Instance_Handle;

  F_DECLSPEC F_DLLEXPORT int Hear360_HPS_GetVolumeMatrix(Hear360_HPS_Instance_Handle handle, float* outVolumeMatrix);

  F_DECLSPEC F_DLLEXPORT Hear360_HPS_Instance_Handle Hear360_HPS_CreateInstance(int samplerate, int presetID);

  F_DECLSPEC F_DLLEXPORT int Hear360_HPS_DeleteInstance(Hear360_HPS_Instance_Handle handle);

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessOutOfPlace(Hear360_HPS_Instance_Handle handle, float azimuth, float** inBuf, float** outBuf, int inputChannelCount, bool stereoUpMix51, int bufFrames);

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessOutOfPlaceInterleaved(Hear360_HPS_Instance_Handle handle, float azimuth, float* inBuf, float* outBuf, int inputChannelCount, int outputChannelCount, bool stereoUpMix51, int bufFrames);

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessInPlace(Hear360_HPS_Instance_Handle handle, float azimuth, float** buf, int channelCount, bool stereoUpMix51, int bufFrames);

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessInPlaceInterleaved(Hear360_HPS_Instance_Handle handle, float azimuth, float* buf, int channelCount, bool stereoUpMix51, int bufFrames);
#ifdef __cplusplus
}
#endif
//######################################################################################################################

#endif // include guard
