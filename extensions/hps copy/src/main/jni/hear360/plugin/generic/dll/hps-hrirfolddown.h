
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_geneic_dll_hrirfolddown_H
#define hear360_plugin_geneic_dll_hrirfolddown_H

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
  typedef void * HPS_HRIRFolddown_Instance_Handle;

  typedef void * HPS_HRIRConvolutionCore_Instance_Handle;

  F_DECLSPEC F_DLLEXPORT HPS_HRIRFolddown_Instance_Handle HPS_HRIRFolddown_CreateInstance(int samplerate);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRFolddown_DeleteInstance(HPS_HRIRFolddown_Instance_Handle handle);

  F_DECLSPEC F_DLLEXPORT void HPS_HRIRFolddown_LoadIRs(HPS_HRIRFolddown_Instance_Handle handle, int presetID);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRFolddown_ProcessInPlace(HPS_HRIRFolddown_Instance_Handle handle, float** pBuf, int srcChannels, long totalsamples, bool hpsEnabled, bool warmEQEnabled);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRFolddown_ProcessInPlaceInterleaved(HPS_HRIRFolddown_Instance_Handle handle, float* pBuf, int srcChannels, long totalsamples, bool hpsEnabled, bool warmEQEnabled);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRFolddown_ProcessOutOfPlace(HPS_HRIRFolddown_Instance_Handle handle, float** pInBuf, float** pOutbuf, int dstChannels, int srcChannels, long totalsamples, bool hpsEnabled, bool warmEQEnabled);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(HPS_HRIRFolddown_Instance_Handle handle, float* pInBuf, float* pOutbuf, int srcChannels, long totalsamples, bool hpsEnabled, bool warmEQEnabled);

  F_DECLSPEC F_DLLEXPORT HPS_HRIRConvolutionCore_Instance_Handle HPS_HRIRConvolutionCore_CreateInstance(int samplerate);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRConvolutionCore_DeleteInstance(HPS_HRIRConvolutionCore_Instance_Handle handle);

  F_DECLSPEC F_DLLEXPORT int HPS_HRIRConvolutionCore_ProcessOutOfPlace(HPS_HRIRConvolutionCore_Instance_Handle handle, float* pInBuf, float** pOutbuf, long totalsamples);

  F_DECLSPEC F_DLLEXPORT void HPS_HRIRConvolutionCore_LoadIR(HPS_HRIRConvolutionCore_Instance_Handle handle, unsigned int channelID);

  F_DECLSPEC F_DLLEXPORT void HPS_HRIRConvolutionCore_LoadIRFromInts(HPS_HRIRConvolutionCore_Instance_Handle handle, int* leftIRs, int* rightIRs, int irFrames);
#ifdef __cplusplus
}
#endif
//######################################################################################################################

#endif // include guard
