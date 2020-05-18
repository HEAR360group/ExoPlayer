
//######################################################################################################################

// © 2019 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/dsp/os/memory.h>
#include <hear360/plugin/generic/dsp/vsheadtracking.h>
#include <hear360/algr/Base/MultiData.h>
#include <hear360/algr/Equalizer/IIRFilterFirstOrder.h>
//#include <hear360/algr/Equalizer/HPSEqualizerBand.h>
#include <hear360/algr/Delay/HPSStaticDelay.h>
#include <hear360/dsp/low/equalizerband.h>
#include <hear360/dsp/low/stereoequalizer.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_stereoeq
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
  float** destination;
};

struct EQUALIZERPROCESSOR
{
  hear360_dsp_low_stereoequalizer::PROCESSOR bandgroup0;
  hear360_dsp_low_stereoequalizer::PROCESSOR bandgroup1;
  hear360_dsp_low_stereoequalizer::PROCESSOR bandgroup2;
};
//######################################################################################################################

struct PRIVATE
{
  EQUALIZERPROCESSOR equalizerprocessor;
  BUFFER buffer;

  hear360_dsp_os_memory::MANAGER memorymanager;

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
  ~PRIVATE();
};

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate)
: memorymanager (memorymanagerparam)
{
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontleft, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.frontright, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination, 2 * sizeof(float*));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination[0], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memorymanager.pGrab(memorymanager.pmanagerdata, (void**)&buffer.destination[1], hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  memset(buffer.destination[0], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
  memset(buffer.destination[1], 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));

  //initialize filters
  equalizerprocessor.bandgroup0.Init(samplerate);
  equalizerprocessor.bandgroup1.Init(samplerate);
  equalizerprocessor.bandgroup2.Init(samplerate);
}

//######################################################################################################################

PRIVATE::~PRIVATE ()
{
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination[0]);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination[1]);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.destination);

  memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontleft);
  memorymanager.pFree(memorymanager.pmanagerdata, buffer.frontright);
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

bool Update (void* handle, const float* eqF, const float* eqG, const float* eqQ)
{
  if (handle == NULL)
    return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
    return false;

  hear360_dsp_low_stereoequalizer::PARAMETERS params0;
  hear360_dsp_low_stereoequalizer::PARAMETERS params1;
  hear360_dsp_low_stereoequalizer::PARAMETERS params2;

  params0.enable = true;
  params1.enable = true;
  params2.enable = true;

  for(int i = 0; i < 4; i++) {
    params0.pband[i].frequency = eqF[i];
    params0.pband[i].qualityfactor = eqQ[i];
    params0.pband[i].gaindb = eqG[i];
    params1.pband[i].frequency = eqF[i + 4];
    params1.pband[i].qualityfactor = eqQ[i + 4];
    params1.pband[i].gaindb = eqG[i + 4];
    params2.pband[i].frequency = eqF[i + 8];
    params2.pband[i].qualityfactor = eqQ[i + 8];
    params2.pband[i].gaindb = eqG[i + 8];
  }

  pprivate->equalizerprocessor.bandgroup0.Update(&params0);
  pprivate->equalizerprocessor.bandgroup1.Update(&params1);
  pprivate->equalizerprocessor.bandgroup2.Update(&params2);

  return true;
}

//######################################################################################################################

bool ProcessInPlace(void* handle, float** pBuf, long totalsamples)
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

  pprivate->equalizerprocessor.bandgroup0.Process(pBuf, totalsamples);
  pprivate->equalizerprocessor.bandgroup1.Process(pBuf, totalsamples);
  pprivate->equalizerprocessor.bandgroup2.Process(pBuf, totalsamples);

  return true;
}

bool ProcessInPlaceInterleaved(void* handle, float* pBuf, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pBuf == NULL)
	  return false;

    PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
  	return false;

  float *ppsrcaudio[2];

  ppsrcaudio [0]   = pprivate->buffer.frontleft;
  ppsrcaudio [1]  = pprivate->buffer.frontright;

  hear360_algr::CopyMonoSIMDWithStride(ppsrcaudio [0], pBuf, totalsamples, 1, 2, 0, 0);
  hear360_algr::CopyMonoSIMDWithStride(ppsrcaudio [1], pBuf, totalsamples, 1, 2, 0, 1);

  pprivate->equalizerprocessor.bandgroup0.Process(ppsrcaudio, totalsamples);
  pprivate->equalizerprocessor.bandgroup1.Process(ppsrcaudio, totalsamples);
  pprivate->equalizerprocessor.bandgroup2.Process(ppsrcaudio, totalsamples);

  hear360_algr::CopyMonoSIMDWithStride(pBuf, ppsrcaudio [0], totalsamples, 2, 1, 0, 0);
  hear360_algr::CopyMonoSIMDWithStride(pBuf, ppsrcaudio [1], totalsamples, 2, 1, 1, 0);

  return true;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
