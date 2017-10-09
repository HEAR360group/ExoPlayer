
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/high/hrirfolddownsimple.h>
#include <hear360/plugin/generic/dsp/interleave.h>
#include <hear360/plugin/generic/dsp/hrirfolddown.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_hrirfolddown
{

#define LOG (0)

#if (LOG)
  #define LOGFILE "d:\\generichrirfolddown.txt"
#endif

#if (LOG)
  #include <cstdio>
#endif

//######################################################################################################################

struct BUFFER
{
  float* frontleft;
  float* frontright;
  float* center;
  float* lfe;
  float* backleft;
  float* backright;
  float* sideleft;
  float* sideright;

  float** destination;
  /*
  float** frontbus;
  float** centerbus;
  float** rearbus;
  float** sidebus;
  */

  /*
  hear360_dsp_os_memory::MONO frontleft;
  hear360_dsp_os_memory::MONO frontright;
  hear360_dsp_os_memory::MONO frontcenter;
  hear360_dsp_os_memory::MONO lfe;
  hear360_dsp_os_memory::MONO backleft;
  hear360_dsp_os_memory::MONO backright;

  hear360_dsp_os_memory::STEREO destination;
  */
};

//######################################################################################################################

struct PRIVATE
{
  hear360_dsp_high_hrirfolddownsimple::PROCESSOR processor;
  hear360_dsp_high_hrirfolddownsimple::PARAMETERS parameters;

  BUFFER buffer;
  hear360_dsp_os_memory::MANAGER memorymanager;

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
  ~PRIVATE();
};

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate)
: processor (memorymanagerparam, samplerate)
, memorymanager (memorymanagerparam)
{
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontleft, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontright, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.center, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.lfe, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.backleft, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.backright, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.sideleft, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.sideright, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  //buffer.frontleft = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.frontright = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.center = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.lfe = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.backleft = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.backright = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.sideleft = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.sideright = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination, 2 * sizeof(float*));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination[0], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination[1], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  //buffer.destination = new float*[2];
  //buffer.destination[0] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //buffer.destination[1] = fftwf_alloc_real(hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  memset(buffer.destination[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.destination[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
}

//######################################################################################################################

PRIVATE::~PRIVATE ()
{
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination[0]);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination[1]);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination);
  /*
  fftwf_free(buffer.destination[0]);
  buffer.destination[0] = NULL;
  fftwf_free(buffer.destination[1]);
  buffer.destination[1] = NULL;
  delete[] buffer.destination;
  buffer.destination = NULL;
  */
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontleft);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontright);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.center);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.lfe);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.backleft);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.backright);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.sideleft);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.sideright);

  //fftwf_free(buffer.frontleft);
  //fftwf_free(buffer.frontright);
  //fftwf_free(buffer.center);
  //fftwf_free(buffer.lfe);
  //fftwf_free(buffer.backleft);
  //fftwf_free(buffer.backright);
  //fftwf_free(buffer.sideleft);
  //fftwf_free(buffer.sideright);
}

//######################################################################################################################

struct GLOBALDATA
{
  //unsigned int nextID;
  //std::map<unsigned int, PRIVATE*> hrirfolddownInstances;

  #if (LOG)
  FILE *pfile;
  #endif

  GLOBALDATA ();
  ~GLOBALDATA();
};

static GLOBALDATA gdata;

GLOBALDATA::GLOBALDATA()
{
  #if (LOG)

  gdata.pfile = std::fopen (LOGFILE, "a");

  std::fprintf (gdata.pfile, "Opening log.\n\n");

  std::fflush (gdata.pfile);

  #endif
}

GLOBALDATA::~GLOBALDATA()
{
  #if (LOG)

  std::fprintf (gdata.pfile, "Closing log.\n\n");

  std::fclose (gdata.pfile);

  #endif
}

//######################################################################################################################

static void FillUnexposedParameters (hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters)
{
  pparameters->mode = 0;

  return;
}

//######################################################################################################################

void* CreateInstance(int samplerate)
{
  PRIVATE *pprivate;

  pprivate = new PRIVATE(hear360_dsp_os_memory::MANAGER(), samplerate);

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

//######################################################################################################################

void ProcessLow
(
  PRIVATE *pprivate,
  float *pdstaudiointerleaved,
  const float *psrcaudiointerleaved,
  int totaldstchannels,
  int totalsrcchannels,
  long totalsamples
)
{
  float *ppsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS];

  ppsrcaudio [hear360_dsp_high_hrirfolddown::FRONTLEFT]   = pprivate->buffer.frontleft;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::FRONTRIGHT]  = pprivate->buffer.frontright;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::FRONTCENTER] = pprivate->buffer.center;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::LFE]         = pprivate->buffer.lfe;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::BACKLEFT]    = pprivate->buffer.backleft;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::BACKRIGHT]   = pprivate->buffer.backright;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::SIDELEFT]    = pprivate->buffer.sideleft;
  ppsrcaudio [hear360_dsp_high_hrirfolddown::SIDERIGHT]   = pprivate->buffer.sideright;

  hear360_plugin_generic_dsp_interleave::DeinterleaveMultiTo6Channels (ppsrcaudio, psrcaudiointerleaved, totalsrcchannels, totalsamples, true);
  /*
  if(totalsrcchannels != 6 && totalsrcchannels != 8)
  {
    long totalinterleavedsamples = totalsamples * totalsrcchannels;

    for(long i = 0; i < totalinterleavedsamples; i++)
    {
      pdstaudiointerleaved[i] = psrcaudiointerleaved[i];
    }

    return;
  }
  */
  //memset(pprivate->buffer.destination[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  //memset(pprivate->buffer.destination[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  pprivate->processor.Process6ChannelsToStereo (pprivate->buffer.destination, ppsrcaudio, totalsrcchannels, totalsamples);

  hear360_plugin_generic_dsp_interleave::InterleaveStereoToMulti (pdstaudiointerleaved, pprivate->buffer.destination, totaldstchannels, totalsamples);

  return;
}

//######################################################################################################################

void ProcessInBlocks
(
  PRIVATE *pprivate,
  float **ppdstaudio,
  const float **ppsrcaudio,
  int totalsrcchannels,
  long totalsamples
)
{
  float *pshifteddstaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS];;
  const float *pshiftedsrcaudio[hear360_dsp_high_hrirfolddown::MAXCHANNELS];;
  long currentposition, samplestoprocess;

  currentposition = 0;

  samplestoprocess = std::min (totalsamples, (long)hear360_dsp_os_memory_AUDIOBUFFERSIZE);

  while (currentposition < totalsamples)
  {
    samplestoprocess = std::min (samplestoprocess, totalsamples - currentposition);

    for(int i = 0; i < 2; i++)
    {
      pshifteddstaudio[i] = ppdstaudio[i] + currentposition;
    }

    for(int i = 0; i < totalsrcchannels; i++)
    {
      if(ppsrcaudio[i] != 0)
      {
        pshiftedsrcaudio[i] = ppsrcaudio[i] + currentposition;
      }
      else
      {
        pshiftedsrcaudio[i] = 0;
      }
    }

    pprivate->processor.Process6ChannelsToStereo ((float**)pshifteddstaudio, (float**)pshiftedsrcaudio, totalsrcchannels, samplestoprocess);

    currentposition += samplestoprocess;
  }
}

void ProcessInBlocks
(
  PRIVATE *pprivate,
  float *pdstaudiointerleaved,
  const float *psrcaudiointerleaved,
  int totaldstchannels,
  int totalsrcchannels,
  long totalsamples
)
{
  float *pshifteddstaudiointerleaved;
  const float *pshiftedsrcaudiointerleaved;
  long currentposition, samplestoprocess;

  currentposition = 0;

  samplestoprocess = std::min (totalsamples, (long)hear360_dsp_os_memory_AUDIOBUFFERSIZE);

  while (currentposition < totalsamples)
  {
    samplestoprocess = std::min (samplestoprocess, totalsamples - currentposition);

    pshifteddstaudiointerleaved = pdstaudiointerleaved + (currentposition * totaldstchannels);

    pshiftedsrcaudiointerleaved = psrcaudiointerleaved + (currentposition * totalsrcchannels);

    ProcessLow (pprivate, pshifteddstaudiointerleaved, pshiftedsrcaudiointerleaved, totaldstchannels, totalsrcchannels, samplestoprocess);

    currentposition += samplestoprocess;
  }

  return;
}
//######################################################################################################################

bool LoadIRsFromPresets(void* handle, int presetID)
{
  if (handle == NULL)
    return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
    return false;

  pprivate->processor.LoadIRsFromPresets(presetID);

  return true;
}

//######################################################################################################################

bool ProcessInPlace(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, float** pBuf, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pBuf == NULL)
  	return false;

  if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	  return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	return false;

  FillUnexposedParameters (pparameters);

  pprivate->processor.Update (pparameters);

  ProcessInBlocks (pprivate, pBuf, (const float**)pBuf, srcChannels, totalsamples);

  //mute the rest channels
  for(int i = 2; i < srcChannels - 2; i++)
  {
    if(pBuf[i] != 0)
    {
      memset(pBuf[i], 0, sizeof(float) * totalsamples);
    }
  }

  return true;
}

bool ProcessInPlaceInterleaved(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, float* pBuf, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pBuf == NULL)
	 return false;

  //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	//return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
    return false;

  FillUnexposedParameters (pparameters);

  pprivate->processor.Update (pparameters);

  ProcessInBlocks (pprivate, pBuf, pBuf, srcChannels, srcChannels, totalsamples);

  return true;
}

bool ProcessOutOfPlace(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, const float** pInBuf, float** pOutbuf, int dstChannels, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	return false;

  if (pOutbuf == NULL)
	return false;

  //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	//return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	return false;

  FillUnexposedParameters (pparameters);

  pprivate->processor.Update (pparameters);

  ProcessInBlocks (pprivate, pOutbuf, pInBuf, srcChannels, totalsamples);

  return true;
}

bool ProcessOutOfPlaceInterleaved(void* handle, hear360_dsp_high_hrirfolddownsimple::PARAMETERS *pparameters, const float* pInBuf, float* pOutbuf, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	  return false;

  if (pOutbuf == NULL)
	  return false;

  if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	  return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	  return false;

  FillUnexposedParameters (pparameters);

  pprivate->processor.Update (pparameters);

  ProcessInBlocks (pprivate, pOutbuf, pInBuf, 2, srcChannels, totalsamples);

  return true;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
