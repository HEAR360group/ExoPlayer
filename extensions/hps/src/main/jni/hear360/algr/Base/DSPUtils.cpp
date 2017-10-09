#include <hear360/algr/defines.h>
#include <hear360/algr/Base/DSPUtils.h>
#include <hear360/algr/Base/APICommon.h>

//######################################################################################################################

namespace hear360_algr
{

	float ** CreateBus(long numChannels, long numFrames)
	{
		float **bus = new float*[numChannels];
		if (bus)
		{
			memset(bus, 0, numChannels*sizeof(float*));

			for (long v = 0; v < numChannels; v++)
			{
				bus[v] = new float[numFrames];
				if (bus[v])
				{
					memset(bus[v], 0, numFrames*sizeof(float));
				}
				else
				{
					DestroyBus(bus, numChannels);
					return NULL;
				}

			}
		}
		return bus;
	}

	// --------------------------------------------------------------------------------------------------------------------------------------

	float ** DestroyBus(float **data, long numChannels)
	{
		if (!data)
			return NULL;
		for (long c = 0; c < numChannels; c++)
		{
			if (data[c])
			{
				delete[] data[c];
				data[c] = NULL;
			}
		}
		if (data)
		{
			delete[] data;
			data = NULL;
		}
		return NULL;
	}

//######################################################################################################################

void MergeBus(float **data1, float fac1, float **data2, float fac2, long numChannels, long numFrames)
{
    for (long c = 0; c < numChannels; c++)
    {
        for (long s = 0; s < numFrames; s++)
        {
            data2[c][s] = data1[c][s] = (fac1*data1[c][s] + fac2*data2[c][s]);
        }
    }
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void MergeBus2(float **data1, float **data2, long numChannels, long numFrames)
{
    for (long c = 0; c < numChannels; c++)
    {
        for (long s = 0; s < numFrames; s++)
        {
            data2[c][s] = data1[c][s] = .5*(data1[c][s] + data2[c][s]);
        }
    }
}

void AddSrcBusToDestBusWithGain(float **destBus, float **srcBus, float gainFac, long numChannels, long numFrames)
{
    for (long c = 0; c < numChannels; c++)
    {
        for (long s = 0; s < numFrames; s++)
        {
            destBus[c][s] += gainFac * srcBus[c][s];
        }
    }
}


// --------------------------------------------------------------------------------------------------------------------------------------

void CloneChannelData(float **data, long channel, long numChannels, long numFrames)
{
    if (channel > numChannels) return;

    if (!data[channel]) return;

    for (long c = 0; c < numChannels; c++)
    {
        if (c != channel)
            memmove(data[c], data[channel], numFrames*sizeof(float));
    }
}

// ------------------------------------------------------------------------------------------------------------------------------

void CopyBusDataSubtractingBFromA(float **destination, float **dataA, float **dataB, long numChannels, long numFrames)
{
    for (long c = 0; c < numChannels; c++)
    {
        if (dataA[c] && dataB[c] && destination[c]) {
            for (long s = 0; s < numFrames; s++) {
                destination[c][s] = dataA[c][s] - dataB[c][s];
            }
        }
    }
}

//----------------------------------------------------------------------------------------

void
ZeroBusData(register float **destination, register long numChannels, register long numFrames)
{
    register long v;
    for (v = 0; v < numChannels; v++)
    {
        memset(destination[v], 0, numFrames*sizeof(float));
    }
}

//----------------------------------------------------------------------------------------

void
CopyBusDataWithGain(float **destination, float **data, register long numChannels, register long numFrames, register float gain)
{
    register long v;
    for (v = 0; v < numChannels; v++)
    {
        register long w;
        register float * dest = destination[v];
        register float * src = data[v];
        for(w = 0; w < numFrames; w++)
        {
            dest[w] = src[w]*gain;
        }
    }
}

//----------------------------------------------------------------------------------------

void
CopyBusData(register float **destination, register float **data, register long numChannels, register long numFrames)

{
    for (register long v = 0; v < numChannels; v++)
    {
        memmove(destination[v], data[v], numFrames*sizeof(float));
    }
}

//----------------------------------------------------------------------------------------

void MergeStereoWithOutputsReversed(register float **dest, register float **source, register long numFrames)
{
    register float * rdata1L = dest[0]; // left
    register float * rdata1R = dest[1]; // right
    register float * rdata2L = source[0]; // left
    register float * rdata2R = source[1]; // right
    register float rhalf = 0.5f;
    register long s;
    for (s = numFrames-1; s >=0 ; s--)
    {
        register float lsamp = rhalf*(rdata1L[s] + rdata2R[s]);
        register float rsamp = rhalf*(rdata1R[s] + rdata2L[s]);

        rdata1L[s] = lsamp;
        rdata1R[s] = rsamp;
    }
}

void MergeStereo(register float **dest, register float **source, register long numFrames)
{
    register float * rdata1L = dest[0]; // left
    register float * rdata1R = dest[1]; // right
    register float * rdata2L = source[0]; // left
    register float * rdata2R = source[1]; // right
    register float rhalf = 0.5f;
    register long s;
    for (s = numFrames-1; s >=0 ; s--)
    {
        register float lsamp = rhalf*(rdata1L[s] + rdata2L[s]);
        register float rsamp = rhalf*(rdata1R[s] + rdata2R[s]);

        rdata1L[s] = lsamp;
        rdata1R[s] = rsamp;
    }
}

float gainDbToFactor(float gain)
{
    return pow(10.0f, gain/20.0f);
}

float gainFactorToDb(float fac)
{
    return 20.*log(fac);
}


float limit(float value, float min, float max)
{
    if (value<min)
        value = min;
    else if (value > max)
        value = max;
    return value;
}

//----------------------------------------------------------------------------------------

int GetSampleRateIndex(int iSampleRate)
{
    const int index =

    ((iSampleRate > 0) && (iSampleRate <= eMaxSampleRate441k)) ?  eSampleRate441k :
    ((iSampleRate <= eMaxSampleRate48k)) ?  eSampleRate48k :
    ((iSampleRate <= eMaxSampleRate882k)) ?  eSampleRate882k :
    ((iSampleRate <= eMaxSampleRate96k)) ?  eSampleRate96k :
    ((iSampleRate <= eMaxSampleRate176k)) ?  eSampleRate176k :
    ((iSampleRate <= eMaxSampleRate192k)) ?  eSampleRate192k :
    eSampleRateUnsupported;

    return index;
}

//----------------------------------------------------------------------------------------

void
mixStereoLocalizationResults(float ** destBus, float ** locBusL, float ** locBusR, unsigned long nFrames)
{
    register float * dest_l = destBus[kLeftChannel];
    register float * dest_r = destBus[kRightChannel];
    register float * locleft_l = locBusL[kLeftChannel];
    register float * locleft_r = locBusL[kRightChannel];
    register float * locright_l = locBusR[kLeftChannel];
    register float * locright_r = locBusR[kRightChannel];
    for(register unsigned long w = 0; w < nFrames; w++)
    {
        *dest_l++ = 0.5f * (*locleft_l++ + *locright_l++);
        *dest_r++ = 0.5f * (*locleft_r++ + *locright_r++);
    }
}

//----------------------------------------------------------------------------------------

void
mixStereoPair(float ** destBus, float ** srcBus, register float gainFac1, register float gainFac2, unsigned long nFrames)
{
    register float * dest_l = destBus[kLeftChannel];
    register float * dest_r = destBus[kRightChannel];
    register float * src_l = srcBus[kLeftChannel];
    register float * src_r = srcBus[kRightChannel];
    for(register unsigned long i = 0; i < nFrames; i++)
    {
        register float lsamp = *dest_l;
        register float rsamp = *dest_r;

        *dest_l++ = (lsamp * gainFac1) + (*src_l++ * gainFac2);
        *dest_r++ = (rsamp * gainFac1) + (*src_r++ * gainFac2);
    }
}

//----------------------------------------------------------------------------------------

void
gainStereoPair(float ** destBus, register float gainFac, unsigned long nFrames)
{
    register float * samp_l = destBus[kLeftChannel];
    register float * samp_r = destBus[kRightChannel];
    for(register unsigned long i = 0; i < nFrames; i++)
    {
        register float lsamp = *samp_l;
        register float rsamp = *samp_r;

        *samp_l++ = (lsamp * gainFac);
        *samp_r++ = (rsamp * gainFac);
    }
}

//######################################################################################################################

} // namespace

//######################################################################################################################
