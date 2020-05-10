
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <hear360/algr/Equalizer/HPSEqualizer4Band.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/equalizerband.h>
#include <hear360/dsp/low/stereoequalizer.h>

//######################################################################################################################

namespace hear360_dsp_low_stereoequalizer
{

//######################################################################################################################

PARAMETERS::PARAMETERS ()
:
  enable (false)
{}

//######################################################################################################################

void PROCESSOR::Update (const PARAMETERS *pparameters)
{
  privatedata.enable = pparameters->enable;

  if (!pparameters->enable) return;

  for (int cx = 0 ; cx < 2 ; cx++)
  {
    for (int bx = 0 ; bx < 4 ; bx++)
    {
      privatedata.ppequalizer[cx]->Set (bx, pparameters->pband[bx].frequency,
                pparameters->pband[bx].gaindb, pparameters->pband[bx].qualityfactor);
    }
  }

  return;
}

//######################################################################################################################

void PROCESSOR::Reset (void)
{
  privatedata.enable = false;

  return;
}

//######################################################################################################################

void PROCESSOR::Process
(
  float *ppaudio[2],            // stereo audio
  long totalsamples
)
{
  if (!privatedata.enable) return;

  for (int cx = 0 ; cx < 2 ; cx++)
  {
    privatedata.ppequalizer[cx]->Process (ppaudio[cx], totalsamples);
  }

  return;
}

//######################################################################################################################

void PROCESSOR::Init(int samplerate)
{
  for (int cx = 0 ; cx < 2 ; cx++)
  {
    privatedata.ppequalizer[cx] = new hear360_algr::HPSEqualizer4Band(samplerate);
  }
}

void PROCESSOR::UnInit()
{
  for (int cx = 0 ; cx < 2 ; cx++)
  {
    if(privatedata.ppequalizer[cx] != 0)
    {
      delete privatedata.ppequalizer[cx];
      privatedata.ppequalizer[cx] = 0;
    }
  }
}

//######################################################################################################################

PROCESSOR::PROCESSOR ()
{
  Reset();

  return;
}

PROCESSOR::~PROCESSOR()
{
  UnInit();

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
