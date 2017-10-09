
//######################################################################################################################

// � 2016 Hear360

//######################################################################################################################

#include <hear360/algr/Equalizer/HPSEqualizerBand.h>
#include <hear360/dsp/os/memory.h>
#include <hear360/dsp/low/equalizerband.h>
#include <hear360/dsp/low/monoequalizer.h>

//######################################################################################################################

namespace hear360_dsp_low_monoequalizer
{

	//######################################################################################################################

	PARAMETERS::PARAMETERS()
		: enable(false)
	{}

	//######################################################################################################################

	void PROCESSOR::Update(const PARAMETERS *pparameters)
	{
		privatedata.enable = pparameters->enable;

		if (!pparameters->enable) return;

		privatedata.pequalizer->SetLowpass(pparameters->pband.frequency,
			pparameters->pband.gaindb, pparameters->pband.qualityfactor);

		return;
	}

	//######################################################################################################################

	void PROCESSOR::Reset(void)
	{
		privatedata.enable = false;

		return;
	}

	//######################################################################################################################

	void PROCESSOR::Process
		(
		float *ppaudio,            // stereo audio
		long totalsamples
		)
	{
		if (!privatedata.enable) return;

		privatedata.pequalizer->Process(ppaudio, totalsamples);

		return;
	}

	void PROCESSOR::Init(int samplerate)
	{
		//UnInit();
		privatedata.pequalizer = new hear360_algr::HPSEqualizerBand(samplerate);
	}

  void PROCESSOR::UnInit()
	{
		if(privatedata.pequalizer != 0)
		{
	  	delete privatedata.pequalizer;
			privatedata.pequalizer = 0;
		}
	}

	//######################################################################################################################

	PROCESSOR::PROCESSOR()
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
