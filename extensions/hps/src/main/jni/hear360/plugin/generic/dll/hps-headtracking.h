
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_geneic_dll_headtracking_H
#define hear360_plugin_geneic_dll_headtracking_H

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
  typedef void * HPS_Headtracking_Instance_Handle;

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_getVolumeMatrix(HPS_Headtracking_Instance_Handle handle, float* outVolumeMatrix);

  F_DECLSPEC F_DLLEXPORT HPS_Headtracking_Instance_Handle HPS_Headtracking_CreateInstance(int samplerate, bool isHeight);

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_DeleteInstance(HPS_Headtracking_Instance_Handle handle);

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_ProcessOutOfPlace(HPS_Headtracking_Instance_Handle handle, float azimuth, float** ppInputBuf, float** ppOutputBuf, int inputChCount, int bufLength);

  F_DECLSPEC F_DLLEXPORT int HPS_Headtracking_ProcessOutOfPlaceInterleaved(HPS_Headtracking_Instance_Handle handle, float azimuth, float* pInputBuf, float* pOutputBuf, int inputChCount, int bufLength);

  //F_DECLSPEC F_DLLEXPORT void HPS_Headtracking_ProcessInPlace(HPS_Headtracking_Instance_Handle handle, float azimuth, float** ppBuf, int inputChCount, int bufLength);

  //F_DECLSPEC F_DLLEXPORT void HPS_Headtracking_ProcessInPlaceInterleaved(HPS_Headtracking_Instance_Handle handle, float azimuth, float* ppBuf, int inputChCount, int bufLength);
#ifdef __cplusplus
}
#endif
//######################################################################################################################

#endif // include guard
