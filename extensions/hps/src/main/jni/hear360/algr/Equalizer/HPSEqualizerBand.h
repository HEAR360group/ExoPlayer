#ifdef __cplusplus

#ifndef __HPSEqualizerBand_h__
#define __HPSEqualizerBand_h__

#include <hear360/algr/defines.h>

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

class HPSEqualizerBand;  //forward

/// convenience method to set a stereo pair of filters with one call
void HPSEqBandSet2(float freq, float q, float gainDb, HPSEqualizerBand * eq, HPSEqualizerBand * eq2);

/// convenience method to set a stereo pair of filters with one call
void HPSEqBandSet2(float b0, float b1, float b2, float a0, float a1, float a2, HPSEqualizerBand * eq, HPSEqualizerBand * eq2);

/**
 *The equalizer class implements a direct form II biquad filter. The filter must be initialized with one of the Set methods
 * prior to being used
 */
class HPSEqualizerBand
    {
    public:
        HPSEqualizerBand(float sampleRate = hear360_algr_HPSIIRHRTF_SAMPLERATE);
        void Reset(void);

        // ----------------------------------------------------------------------------------------------------------------
        // The equalizer class is based on a ;
        // ----------------------------------------------------------------------------------------------------------------

        /// initialize filter coefficients for a lowpass filter
        void SetLowpass(float freq, float gainDb, float q);

        // initialize filter coefficients for a highpass filter
        void SetHighpass(float freq, float gainDb, float q);

        // initialize filter coefficients for a bandpass filter
        void SetBandpass(float freq, float gainDb, float q);

        // initialize filter coefficients for a parametric EQ
        void SetParametric(float freq, float gainDb, float q);

        // initialize filter coefficients for a second order filter
        void SetSecondOrder(float b0, float b1, float b2, float a0, float a1, float a2);

        /**
         * process the filter (in-place)
         **/
        inline void Process(register float *data, register long numFrames)
        {
#if hear360_algr_IMPLEMENT_SANITY_CHECKS
            if (!data)
                return;
            if (numFrames < 1)
                return;
#endif
            register float rs1 = m_state1;
            register float rs2 = m_state2;
            register float rsc0b0 = mSmooCoeffs[0];
            register float rsc1b0 = mSmooCoeffs[1];
            register float rsc2b0 = mSmooCoeffs[2];
            register float rsc3b0 = mSmooCoeffs[3];
            register float rsc4b0 = mSmooCoeffs[4];
            register float fout;
            for (register long v = 0; v < numFrames; v++)
            {
                register float fin = data[v];

                // apply band 1
                data[v] = fout = rs1 + rsc0b0*fin;
                rs1 = rs2 + rsc1b0*fin -rsc3b0*fout;
                rs2 = rsc2b0*fin - rsc4b0*fout;
            }
            m_state1 = rs1;
            m_state2 = rs2;
        }

        /**
         * sends the filtered data to output, and the inverse of the filter to output2
         * for example, if we are set to be a lowpass filter, output1 will contain the lowpass result and output2 will contain the remaining frequencies
         **/
        inline void Split(register float *input, register float * output, register float * output2, register long numFrames)
        {
#if hear360_algr_IMPLEMENT_SANITY_CHECKS
            if (!input)
                return;
            if (!output)
                return;
            if (!output2)
                return;
            if (numFrames < 1)
                return;
#endif
            register float rs1 = m_state1;
            register float rs2 = m_state2;
            register float rsc0b0 = mSmooCoeffs[0];
            register float rsc1b0 = mSmooCoeffs[1];
            register float rsc2b0 = mSmooCoeffs[2];
            register float rsc3b0 = mSmooCoeffs[3];
            register float rsc4b0 = mSmooCoeffs[4];
            register float fout;
            for (register long v = 0; v < numFrames; v++)
            {
                // apply band 1
                register float fin = input[v];
                output[v] = fout = rs1 + rsc0b0*fin;
                rs1 = rs2 + rsc1b0*fin -rsc3b0*fout;
                rs2 = rsc2b0*fin - rsc4b0*fout;
                output2[v] = fin - fout;

/*
                fin = input[v+1];
                output[v+1] = fout = rs1 + rsc0b0*fin;
                rs1 = rs2 + rsc1b0*fin -rsc3b0*fout;
                rs2 = rsc2b0*fin - rsc4b0*fout;
                output2[v+1] = fin - fout;
*/
            }
            m_state1 = rs1;
            m_state2 = rs2;
        }

        /**
         * sends the filtered data to output, and the inverse of the filter to output2; the input is multiplied by a gain factor prior to filtering
         * for example, if we are set to be a lowpass filter, output1 will contain the lowpass result and output2 will contain the remaining frequencies
         **/
        inline void SplitWithAttenuation(register float *input, register float * output, register float * output2, register float gainFactor, register long numFrames)
        {
#if hear360_algr_IMPLEMENT_SANITY_CHECKS
            if (!input)
                return;
            if (!output)
                return;
            if (!output2)
                return;
            if (numFrames < 1)
                return;
#endif
            register float rs1 = m_state1;
            register float rs2 = m_state2;
            register float rsc0b0 = mSmooCoeffs[0];
            register float rsc1b0 = mSmooCoeffs[1];
            register float rsc2b0 = mSmooCoeffs[2];
            register float rsc3b0 = mSmooCoeffs[3];
            register float rsc4b0 = mSmooCoeffs[4];
            register float fout;
            for (register long v = 0; v < numFrames; v++)
            {
                // apply band 1
                register float fin = input[v] * gainFactor;
                output[v] = fout = rs1 + rsc0b0*fin;
                rs1 = rs2 + rsc1b0*fin -rsc3b0*fout;
                rs2 = rsc2b0*fin - rsc4b0*fout;
                output2[v] = fin - fout;
/*
                fin = input[v+1] * gainFactor;
                output[v+1] = fout = rs1 + rsc0b0*fin;
                rs1 = rs2 + rsc1b0*fin -rsc3b0*fout;
                rs2 = rsc2b0*fin - rsc4b0*fout;
                output2[v+1] = fin - fout;
*/
            }
            m_state1 = rs1;
            m_state2 = rs2;
        }

    protected:

        void SetSmoothingCoeffs(void);

        float m_sr;
        float m_sa0, m_sa1, m_sa2, m_sa3, m_sa4;
        float mSmooCoeffs[5];
        float m_state1, m_state2;
    private:
    };

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif /* __HPSEqualizerBand_h__ */
#endif // __cplusplus
