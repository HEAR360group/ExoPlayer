#ifndef __HPSStaticDelay_h__
#define __HPSStaticDelay_h__

#ifdef __cplusplus

class HPSStaticDelay
	{

	public:

		HPSStaticDelay(float sampleRate = 44100, float maxDelaySeconds = 0.001);
		~HPSStaticDelay();
		void Reset(void);
		void Update(void);
		long SetDelaySamples(long samples);
		long GetDelaySamples();
		float *GetDelayBuffer(long *bufferSize);

		// -----------------------------------------------------------------------------------------------------------------------------------------------

		inline void ProcessDoubleBuffered(float *data, long n)
		{
			for (long v = 0; v<n; v++) {

				float fac = (float)v/(float)n;

				float tempPrevious = mDelayPrevious[mDelayRoverPrevious];
				mDelayPrevious[mDelayRoverPrevious] = data[v];

				float tempCurrent = mDelayCurrent[mDelayRoverCurrent];
				mDelayCurrent[mDelayRoverCurrent] = data[v];

				data[v] = (1.-fac)*tempPrevious + fac*tempCurrent;

				mDelayRoverPrevious++;
				if (mDelayRoverPrevious >= mDelaySamplesPrevious)
					mDelayRoverPrevious = 0;

				mDelayRoverCurrent++;
				if (mDelayRoverCurrent >= mDelaySamplesCurrent)
					mDelayRoverCurrent = 0;
			}

			if (mDelaySamplesNew)
				Update();
		}

		// -----------------------------------------------------------------------------------------------------------------------------------------------

		inline void Process(float *data, long n, bool outputSound = true)
		{

			for (long v = 0; v<n; v++) {

				float tempCurrent = mDelayCurrent[mDelayRoverCurrent];
				mDelayCurrent[mDelayRoverCurrent] = data[v];

				if (outputSound) data[v] = tempCurrent;

				mDelayRoverCurrent++;
				if (mDelayRoverCurrent >= mDelaySamplesCurrent)
					mDelayRoverCurrent = 0;
			}

			if (mDelaySamplesNew) {
				mDelaySamplesCurrent		= mDelaySamplesNew;
				mDelaySamplesNew = 0;
			}
		}

		// -----------------------------------------------------------------------------------------------------------------------------------------------


	private:

		float *mDelayPrevious;
		long mDelayRoverPrevious;
		long mDelaySamplesPrevious;

		float *mDelayCurrent;
		long mDelayRoverCurrent;
		long mDelaySamplesCurrent;

		long mDelaySamplesNew;

		long mMaxDelaySamples;

	};

#endif /* __HPSStaticDelay_h__ */

#endif //#ifdef __cplusplus
