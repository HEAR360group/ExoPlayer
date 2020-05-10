#include <hear360/algr/Delay/HPSStaticDelay.h>
#include <hear360/algr/Base/DSPUtils.h>



HPSStaticDelay::HPSStaticDelay(float sampleRate, float maxDelaySeconds)
{
	maxDelaySeconds = hear360_algr::limit(maxDelaySeconds, 0.01, 60.);

	mDelaySamplesPrevious		= 0;
	mDelaySamplesCurrent		= 0;
	mDelaySamplesNew			= 0;

	mDelayRoverPrevious			= 0;
	mDelayRoverCurrent			= 0;

	mMaxDelaySamples	= maxDelaySeconds*sampleRate;

	mDelayPrevious = new float[mMaxDelaySamples];
	mDelayCurrent = new float[mMaxDelaySamples];
	memset(mDelayPrevious, 0, mMaxDelaySamples*sizeof(float));
	memset(mDelayCurrent, 0, mMaxDelaySamples*sizeof(float));
}

// -----------------------------------------------------------------------------------------------------------------------------------------------

HPSStaticDelay::~HPSStaticDelay()
{
	delete[] mDelayPrevious;
	delete[] mDelayCurrent;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------

void HPSStaticDelay::Reset(void)
{
	mDelayRoverPrevious			= 0;
	mDelayRoverCurrent			= 0;
	memset(mDelayPrevious, 0, mMaxDelaySamples*sizeof(float));
	memset(mDelayCurrent, 0, mMaxDelaySamples*sizeof(float));
}

// -----------------------------------------------------------------------------------------------------------------------------------------------

void HPSStaticDelay::Update(void)
{
	mDelaySamplesPrevious		= mDelaySamplesCurrent;
	mDelaySamplesCurrent		= mDelaySamplesNew;
	mDelayRoverPrevious			= mDelayRoverCurrent;
	memmove(mDelayPrevious, mDelayCurrent, mMaxDelaySamples*sizeof(float));
	mDelaySamplesNew = 0;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------

long HPSStaticDelay::SetDelaySamples(long samples)
{
	mDelaySamplesNew = samples;

	if (mDelaySamplesNew > mMaxDelaySamples)
		mDelaySamplesNew = mMaxDelaySamples;

	if (samples < 0)
		mDelaySamplesNew = 0;

	return mDelaySamplesNew;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------

long HPSStaticDelay::GetDelaySamples()
{
	return mDelaySamplesCurrent;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
float * HPSStaticDelay::GetDelayBuffer(long *bufferSize)
{
	*bufferSize = mDelaySamplesCurrent;
	return mDelayCurrent;
}

// ###############################################################################################################################################
