
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/high/convolutioncore.h>
#include <hear360/plugin/generic/dsp/convolutioncore.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_convolutioncore
{

#define LOG (0)

#if (LOG)
  #define LOGFILE "d:\\generichrirfolddown.txt"
#endif

#if (LOG)
  #include <cstdio>
#endif

//######################################################################################################################

struct PRIVATE
{
  hear360_dsp_high_convolutioncore::PROCESSOR processor;
  hear360_dsp_high_convolutioncore::PARAMETERS parameters;

  hear360_dsp_os_memory::MANAGER memorymanager;

  int mSamplerate;

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate);
  ~PRIVATE();
};

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate)
: processor (memorymanagerparam)
, memorymanager (memorymanagerparam)
{
  mSamplerate = samplerate;
}

//######################################################################################################################

PRIVATE::~PRIVATE ()
{

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

void loadIRs(void *handle, unsigned int channelID)
{
  PRIVATE* pprivate = (PRIVATE*)handle;
  pprivate->processor.LoadIRs (channelID, pprivate->mSamplerate);
}

void loadIRFromInts(void *handle, int* leftIRs, int* rightIRs, int irFrames)
{
  PRIVATE* pprivate = (PRIVATE*)handle;
  pprivate->processor.LoadIRFromInts (leftIRs, rightIRs, irFrames);
}

//######################################################################################################################

void ProcessInBlocks
(
  PRIVATE *pprivate,
  float **ppdstaudio,
  const float *ppsrcaudio,
  long totalsamples
)
{
  float *pshifteddstaudio[2];
  const float *pshiftedsrcaudio;
  long currentposition, samplestoprocess;

  currentposition = 0;

  samplestoprocess = std::min (totalsamples, (long)hear360_dsp_os_memory_AUDIOBUFFERSIZE);
  //samplestoprocess = std::min (totalsamples, (long)hear360_dsp_os_memory_AUDIOBUFFERSIZE);

  while (currentposition < totalsamples)
  {
    samplestoprocess = std::min (samplestoprocess, totalsamples - currentposition);

    for(int i = 0; i < 2; i++)
    {
      pshifteddstaudio[i] = ppdstaudio[i] + currentposition;
    }

    pshiftedsrcaudio = ppsrcaudio + currentposition;

    pprivate->processor.Process ((float**)pshifteddstaudio, (float*)pshiftedsrcaudio, samplestoprocess);

    currentposition += samplestoprocess;
  }
}

//######################################################################################################################

bool ProcessOutOfPlace(void* handle, hear360_dsp_high_convolutioncore::PARAMETERS *pparameters, const float* pInBuf, float** pOutbuf, long totalsamples)
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

  pprivate->processor.Update (pparameters);

  ProcessInBlocks (pprivate, pOutbuf, pInBuf, totalsamples);

  return true;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
