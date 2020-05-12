
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/plugin/generic/dll/hps.h>
#include <hear360/plugin/generic/dll/hps-upmix51.h>
#include <hear360/plugin/generic/dll/hps-headtracking.h>
#include <hear360/plugin/generic/dll/hps-hrirfolddown.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/algr/Base/MultiData.h>

//######################################################################################################################

namespace hear360_plugin_generic_dll_hps
{
  struct BUFFER
  {
    float** upmix51Buf;
    float* upmix51BufInterleaved;
    float** htBuf;
    float* htBufInterleaved;
    float** inBuf;
    float** outBuf;
    float* outBufInterleaved;
  };

  struct PRIVATE
  {
    HPS_Upmix51_Instance_Handle upmix51_instance_handle;
    HPS_Headtracking_Instance_Handle headtracking_instance_handle;
    HPS_HRIRFolddown_Instance_Handle folddown_instance_handle;

    BUFFER buffer;
    float masterScale;
    bool isHeight;

    hear360_dsp_os_memory::MANAGER memorymanager;

    PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate, int presetID);
    ~PRIVATE();
  };

  PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate, int presetID)
  : masterScale (0.45f)
  , isHeight (presetID >= 100)
  , memorymanager (memorymanagerparam)
  {
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf, 8 * sizeof(float*));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[0], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[1], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[2], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[3], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[4], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[5], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[6], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBuf[7], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));

    // buffer.htBuf = new float*[8];
    // for(int i = 0; i < 8; i++)
    // {
    //   buffer.htBuf[i] = new float[hear360_dsp_os_memory_AUDIOBUFFERSIZE];
    // }

    memset(buffer.htBuf[0], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[1], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[2], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[3], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[4], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[5], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[6], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.htBuf[7], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));

    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.htBufInterleaved, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float) * 8);

    memset(buffer.htBufInterleaved, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float) * 8);


    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf, 8 * sizeof(float*));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[0], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[1], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[2], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[3], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[4], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[5], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[6], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51Buf[7], hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));

    memset(buffer.upmix51Buf[0], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[1], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[2], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[3], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[4], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[5], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[6], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));
    memset(buffer.upmix51Buf[7], 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float));

    memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.upmix51BufInterleaved, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float) * 8);

    memset(buffer.upmix51BufInterleaved, 0, hear360_dsp_os_memory_IO_AUDIOBUFFERSIZE * sizeof(float) * 8);

    upmix51_instance_handle = HPS_Upmix51_CreateInstance(samplerate);
    headtracking_instance_handle = HPS_Headtracking_CreateInstance(samplerate, isHeight);
    folddown_instance_handle = HPS_HRIRFolddown_CreateInstance(samplerate);
    HPS_HRIRFolddown_LoadIRs(folddown_instance_handle, presetID);
  }

  PRIVATE::~PRIVATE ()
  {
    if(upmix51_instance_handle != NULL) {
      HPS_Upmix51_DeleteInstance(upmix51_instance_handle);
    }

    if(headtracking_instance_handle != NULL) {
      HPS_Headtracking_DeleteInstance(headtracking_instance_handle);
    }

    if(folddown_instance_handle != NULL) {
      HPS_HRIRFolddown_DeleteInstance(folddown_instance_handle);
    }


    if(buffer.htBuf != NULL) {
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[0]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[1]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[2]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[3]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[4]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[5]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[6]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf[7]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBuf);
    }

    if(buffer.htBufInterleaved != NULL) {
      //delete[] buffer.htBufInterleaved;
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.htBufInterleaved);
    }


    if(buffer.upmix51Buf != NULL) {
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[0]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[1]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[2]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[3]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[4]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[5]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[6]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf[7]);
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51Buf);
    }

    if(buffer.upmix51BufInterleaved != NULL) {
      //delete[] buffer.upmix51BufInterleaved;
      memorymanager.pFree(memorymanager.pmanagerdata, buffer.upmix51BufInterleaved);
    }
  }

  void* CreateInstance(int samplerate, int presetID)
  {
    PRIVATE *pprivate;

    pprivate = new PRIVATE(hear360_dsp_os_memory::MANAGER(), samplerate, presetID);

    return (void*)pprivate;
  }

  bool DeleteInstance(void* handle)
  {
    if (handle == NULL)
      return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
      return false;

    delete pprivate;
    handle = NULL;

    return true;
  }

  void ProcessOutOfPlace(void* handle, float azimuth, float** inBuf, float** outBuf, int inputChannelCount, bool stereoUpMix51, int bufFrames)
  {
    if (handle == NULL)
      return;

    if (inBuf == NULL)
  	  return;

    if (outBuf == NULL)
  	  return;

    //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
  	//return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
  	  return;

    if(stereoUpMix51 && inputChannelCount == 2)
    {
      HPS_Upmix51_ProcessOutOfPlace(pprivate->upmix51_instance_handle, inBuf, pprivate->buffer.upmix51Buf, inputChannelCount, 8, bufFrames);
      HPS_Headtracking_ProcessOutOfPlace(pprivate->headtracking_instance_handle, azimuth, pprivate->buffer.upmix51Buf, pprivate->buffer.htBuf, 8, bufFrames);
    }
    else
    {
      HPS_Headtracking_ProcessOutOfPlace(pprivate->headtracking_instance_handle, azimuth, inBuf, pprivate->buffer.htBuf, inputChannelCount, bufFrames);
    }

    HPS_HRIRFolddown_ProcessOutOfPlace(pprivate->folddown_instance_handle, pprivate->buffer.htBuf, outBuf, pprivate->isHeight ? 6 : 8, bufFrames, true, true);

    hear360_algr::MulMonoByScalarSIMD(outBuf[0], outBuf[0], pprivate->masterScale, bufFrames);
    hear360_algr::MulMonoByScalarSIMD(outBuf[1], outBuf[1], pprivate->masterScale, bufFrames);
    hear360_algr::LimitMonoSIMD(outBuf[0], outBuf[0], -1.0f, 1.0f, bufFrames);
    hear360_algr::LimitMonoSIMD(outBuf[1], outBuf[1], -1.0f, 1.0f, bufFrames);
  }

  void ProcessOutOfPlaceInterleaved(void* handle, float azimuth, float* inBuf, float* outBuf, int inputChannelCount, int outputChannelCount, bool stereoUpMix51, int bufFrames)
  {
    if (handle == NULL)
      return;

    if (inBuf == NULL)
      return;

    if (outBuf == NULL)
      return;

    //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
    //return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
      return;
/*
    for(int i = 0; i < bufFrames; i++)
    {
      for(int j = 0; j < inputChannelCount; j++)
      {
        pprivate->buffer.inBuf[j][i] = inBuf[i * inputChannelCount + j];
      }
    }

    ProcessOutOfPlace(handle, azimuth, pprivate->buffer.inBuf, pprivate->buffer.outBuf, inputChannelCount, bufFrames);

    for(int i = 0; i < bufFrames; i++)
    {
      for(int j = 0; j < inputChannelCount; j++)
      {
        outBuf[i * inputChannelCount + j] = pprivate->buffer.outBuf[j][i];
      }
    }
*/

    if(stereoUpMix51 && inputChannelCount == 2)
    {
      HPS_Upmix51_ProcessOutOfPlaceInterleaved(pprivate->upmix51_instance_handle, inBuf, pprivate->buffer.upmix51BufInterleaved, inputChannelCount, 8, bufFrames);
      HPS_Headtracking_ProcessOutOfPlaceInterleaved(pprivate->headtracking_instance_handle, azimuth, pprivate->buffer.upmix51BufInterleaved, pprivate->buffer.htBufInterleaved, 8, bufFrames);
    }
    else
    {
      //HPS_Headtracking_ProcessOutOfPlaceInterleaved(pprivate->headtracking_instance_handle, azimuth, inBuf, pprivate->buffer.htBufInterleaved, inputChannelCount, bufFrames);
    }

    //HPS_Headtracking_ProcessOutOfPlaceInterleaved(pprivate->headtracking_instance_handle, azimuth, inBuf, outBuf, inputChannelCount, bufFrames);
    HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(pprivate->folddown_instance_handle, inBuf, outBuf, inputChannelCount, outputChannelCount, bufFrames, true, true);

    // HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(pprivate->folddown_instance_handle, pprivate->buffer.htBufInterleaved, outBuf, pprivate->isHeight ? 6 : 8, outputChannelCount, bufFrames, true, true);
    //HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(pprivate->folddown_instance_handle, inBuf, outBuf, inputChannelCount, outputChannelCount, bufFrames, true, true);
    //HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(pprivate->folddown_instance_handle, inBuf, outBuf, 8, bufFrames, true, true);

    hear360_algr::MulMonoByScalarSIMDWithStride(outBuf, outBuf, pprivate->masterScale, bufFrames, outputChannelCount, outputChannelCount, 0, 0);
    hear360_algr::MulMonoByScalarSIMDWithStride(outBuf, outBuf, pprivate->masterScale, bufFrames, outputChannelCount, outputChannelCount, 1, 1);
    hear360_algr::LimitMonoSIMD(outBuf, outBuf, -1.0f, 1.0f, bufFrames * outputChannelCount);
  }

  void ProcessInPlace(void* handle, float azimuth, float** buf, int channelCount, int stereoUpMix51, int bufFrames)
  {
    if (handle == NULL)
      return;

    if (buf == NULL)
      return;

    //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
    //return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
      return;

    if(stereoUpMix51 && channelCount == 2)
    {
      HPS_Upmix51_ProcessOutOfPlace(pprivate->upmix51_instance_handle, buf, pprivate->buffer.upmix51Buf, channelCount, 8, bufFrames);
      HPS_Headtracking_ProcessOutOfPlace(pprivate->headtracking_instance_handle, azimuth, pprivate->buffer.upmix51Buf, pprivate->buffer.htBuf, 8, bufFrames);
    }
    else
    {
      HPS_Headtracking_ProcessOutOfPlace(pprivate->headtracking_instance_handle, azimuth, buf, pprivate->buffer.htBuf, channelCount, bufFrames);
    }

    HPS_HRIRFolddown_ProcessOutOfPlace(pprivate->folddown_instance_handle, pprivate->buffer.htBuf, buf, 8, bufFrames, true, true);

    hear360_algr::MulMonoByScalarSIMD(buf[0], buf[0], pprivate->masterScale, bufFrames);
    hear360_algr::MulMonoByScalarSIMD(buf[1], buf[1], pprivate->masterScale, bufFrames);
    hear360_algr::LimitMonoSIMD(buf[0], buf[0], -1.0f, 1.0f, bufFrames);
    hear360_algr::LimitMonoSIMD(buf[1], buf[1], -1.0f, 1.0f, bufFrames);
  }

  void ProcessInPlaceInterleaved(void* handle, float azimuth, float* buf, int channelCount, bool stereoUpMix51, int bufFrames)
  {
    if (handle == NULL)
      return;

    if (buf == NULL)
      return;

    //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
    //return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
      return;
/*
    for(int i = 0; i < bufFrames; i++)
    {
      for(int j = 0; j < channelCount; j++)
      {
        pprivate->buffer.outBuf[j][i] = buf[i * channelCount + j];
      }
    }

    ProcessInPlace(handle, azimuth, pprivate->buffer.outBuf, channelCount, bufFrames);

    for(int i = 0; i < bufFrames; i++)
    {
      for(int j = 0; j < channelCount; j++)
      {
        buf[i * channelCount + j] = pprivate->buffer.outBuf[j][i];
      }
    }
*/
    if(stereoUpMix51 && channelCount == 2)
    {
      HPS_Upmix51_ProcessOutOfPlaceInterleaved(pprivate->upmix51_instance_handle, buf, pprivate->buffer.upmix51BufInterleaved, channelCount, 8, bufFrames);
      HPS_Headtracking_ProcessOutOfPlaceInterleaved(pprivate->headtracking_instance_handle, azimuth, pprivate->buffer.upmix51BufInterleaved, pprivate->buffer.htBufInterleaved, 8, bufFrames);
    }
    else
    {
      HPS_Headtracking_ProcessOutOfPlaceInterleaved(pprivate->headtracking_instance_handle, azimuth, buf, pprivate->buffer.htBufInterleaved, channelCount, bufFrames);
    }

    HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(pprivate->folddown_instance_handle, pprivate->buffer.htBufInterleaved, buf, 8, channelCount, bufFrames, true, true);

    hear360_algr::MulMonoByScalarSIMDWithStride(buf, buf, pprivate->masterScale, bufFrames, channelCount, channelCount, 0, 0);
    hear360_algr::MulMonoByScalarSIMDWithStride(buf, buf, pprivate->masterScale, bufFrames, channelCount, channelCount, 1, 1);
    hear360_algr::LimitMonoSIMD(buf, buf, -1.0f, 1.0f, bufFrames * channelCount);
  }

  int GetVolumeMatrix(void* handle, float* outVolumeMatrix)
  {
    if (handle == NULL)
      return 0;

    //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
    //return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

    if (pprivate == NULL)
      return 0;

    return HPS_Headtracking_getVolumeMatrix(pprivate->headtracking_instance_handle, outVolumeMatrix);
  }
}

#ifdef __cplusplus
extern "C"
{
#endif
  F_DECLSPEC F_DLLEXPORT int Hear360_HPS_GetVolumeMatrix(Hear360_HPS_Instance_Handle handle, float* outVolumeMatrix)
  {
    return hear360_plugin_generic_dll_hps::GetVolumeMatrix(handle, outVolumeMatrix);
  }

  F_DECLSPEC F_DLLEXPORT Hear360_HPS_Instance_Handle Hear360_HPS_CreateInstance(int samplerate, int presetID)
  {
    return hear360_plugin_generic_dll_hps::CreateInstance(samplerate, presetID);
  }

  F_DECLSPEC F_DLLEXPORT int Hear360_HPS_DeleteInstance(Hear360_HPS_Instance_Handle handle)
  {
	  return (hear360_plugin_generic_dll_hps::DeleteInstance(handle) ? 1 : 0);
  }

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessOutOfPlace(Hear360_HPS_Instance_Handle handle, float azimuth, float** inBuf, float** outBuf, int inputChannelCount, bool stereoUpMix51, int bufFrames)
  {
	   return hear360_plugin_generic_dll_hps::ProcessOutOfPlace(handle, azimuth, inBuf, outBuf, inputChannelCount, stereoUpMix51, bufFrames);
  }

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessOutOfPlaceInterleaved(Hear360_HPS_Instance_Handle handle, float azimuth, float* inBuf, float* outBuf, int inputChannelCount, int outputChannelCount, bool stereoUpMix51, int bufFrames)
  {
    return hear360_plugin_generic_dll_hps::ProcessOutOfPlaceInterleaved(handle, azimuth, inBuf, outBuf, inputChannelCount, outputChannelCount, stereoUpMix51, bufFrames);
  }

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessInPlace(Hear360_HPS_Instance_Handle handle, float azimuth, float** buf, int channelCount, bool stereoUpMix51, int bufFrames)
  {
    return hear360_plugin_generic_dll_hps::ProcessInPlace(handle, azimuth, buf, channelCount, stereoUpMix51, bufFrames);
  }

  F_DECLSPEC F_DLLEXPORT void Hear360_HPS_ProcessInPlaceInterleaved(Hear360_HPS_Instance_Handle handle, float azimuth, float* buf, int channelCount, bool stereoUpMix51, int bufFrames)
  {
    return hear360_plugin_generic_dll_hps::ProcessInPlaceInterleaved(handle, azimuth, buf, channelCount, stereoUpMix51, bufFrames);
  }
#ifdef __cplusplus
}
#endif
//######################################################################################################################
