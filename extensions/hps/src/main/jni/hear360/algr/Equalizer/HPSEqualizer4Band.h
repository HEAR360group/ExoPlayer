#ifndef __HPSEqualizer4Band_h__
#define __HPSEqualizer4Band_h__

#include <hear360/algr/defines.h>

#ifdef __cplusplus

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

class HPSEqualizer4Band
    {
    public:
        HPSEqualizer4Band(float sampleRate = hear360_algr_HPSIIRHRTF_SAMPLERATE);
        void Reset(int whichBand);
        void ResetAll();
        void Set(int whichBand, float freq, float gainDb, float q);

        inline void Process(float *data, long numFrames)
        {
#if hear360_algr_IMPLEMENT_SANITY_CHECKS
            if (!data)
                return;
            if (numFrames < 1)
                return;
#endif
#define BAND_0 0
#define BAND_1 1
#define BAND_2 2
#define BAND_3 3
#if 0 // baseline
            for (long v = 0; v < numFrames; v++)
            {
                // read data
                float fin = data[v];

                // process band 1
                float fout = m_state1[BAND_0] + mSmooCoeffs[BAND_0][0]*fin;
                m_state1[BAND_0] = m_state2[BAND_0] + mSmooCoeffs[BAND_0][1]*fin -
                mSmooCoeffs[BAND_0][3]*fout;
                m_state2[BAND_0] = mSmooCoeffs[BAND_0][2]*fin - mSmooCoeffs[BAND_0][4]*fout;

                // swap
                fin = fout;

                // process band 2
                fout = m_state1[BAND_1] + mSmooCoeffs[BAND_1][0]*fin;
                m_state1[BAND_1] = m_state2[BAND_1] + mSmooCoeffs[BAND_1][1]*fin -
                mSmooCoeffs[BAND_1][3]*fout;
                m_state2[BAND_1] = mSmooCoeffs[BAND_1][2]*fin - mSmooCoeffs[BAND_1][4]*fout;

                // swap
                fin = fout;

                // process band 3
                fout = m_state1[BAND_2] + mSmooCoeffs[BAND_2][0]*fin;
                m_state1[BAND_2] = m_state2[BAND_2] + mSmooCoeffs[BAND_2][1]*fin -
                mSmooCoeffs[BAND_2][3]*fout;
                m_state2[BAND_2] = mSmooCoeffs[BAND_2][2]*fin - mSmooCoeffs[BAND_2][4]*fout;

                // swap
                fin = fout;

                // process band 4
                fout = m_state1[BAND_3] + mSmooCoeffs[BAND_3][0]*fin;
                m_state1[BAND_3] = m_state2[BAND_3] + mSmooCoeffs[BAND_3][1]*fin -
                mSmooCoeffs[BAND_3][3]*fout;
                m_state2[BAND_3] = mSmooCoeffs[BAND_3][2]*fin - mSmooCoeffs[BAND_3][4]*fout;

                // write output data
                data[v] = fout;
            }
#elif 1 // opt 3 - break into two loops; cache state coeffs into 14 registers (+2 regs for fin/fout)
            {
                {
#ifdef EQBAND2
#undef EQBAND2
#endif
#define EQBAND2(v)                                      \
fin = data[v];  /* read data */                         \
fout = rs1b0 + rc0b0*fin;                               \
rs1b0 = rs2b0 + rc1b0*fin - rc3b0*fout;                 \
rs2b0 = rc2b0*fin - rc4b0*fout;                         \
data[v] = fin = rs1b1 + rc0b1*fout; /* write output data */ \
rs1b1 = rs2b1 + rc1b1*fout - rc3b1*fin;                 \
rs2b1 = rc2b1*fout - rc4b1*fin;
                    register float rs1b0 = m_state1[BAND_0];
                    register float rs2b0 = m_state2[BAND_0];
                    register float rs1b1 = m_state1[BAND_1];
                    register float rs2b1 = m_state2[BAND_1];

                    register float rc0b0 = mSmooCoeffs[BAND_0][0];
                    register float rc1b0 = mSmooCoeffs[BAND_0][1];
                    register float rc2b0 = mSmooCoeffs[BAND_0][2];
                    register float rc3b0 = mSmooCoeffs[BAND_0][3];
                    register float rc4b0 = mSmooCoeffs[BAND_0][4];

                    register float rc0b1 = mSmooCoeffs[BAND_1][0];
                    register float rc1b1 = mSmooCoeffs[BAND_1][1];
                    register float rc2b1 = mSmooCoeffs[BAND_1][2];
                    float rc3b1 = mSmooCoeffs[BAND_1][3];
                    float rc4b1 = mSmooCoeffs[BAND_1][4];
                    for (register long v = 0; v < numFrames; v++)
                    {
                        register float fin;
                        register float fout;

                        EQBAND2(v);
                    }
                    m_state1[BAND_0] = rs1b0;
                    m_state2[BAND_0] = rs2b0;
                    m_state1[BAND_1] = rs1b1;
                    m_state2[BAND_1] = rs2b1;

                    rs1b0 = m_state1[BAND_2];
                    rs2b0 = m_state2[BAND_2];
                    rs1b1 = m_state1[BAND_3];
                    rs2b1 = m_state2[BAND_3];

                    rc0b0 = mSmooCoeffs[BAND_2][0];
                    rc1b0 = mSmooCoeffs[BAND_2][1];
                    rc2b0 = mSmooCoeffs[BAND_2][2];
                    rc3b0 = mSmooCoeffs[BAND_2][3];
                    rc4b0 = mSmooCoeffs[BAND_2][4];

                    rc0b1 = mSmooCoeffs[BAND_3][0];
                    rc1b1 = mSmooCoeffs[BAND_3][1];
                    rc2b1 = mSmooCoeffs[BAND_3][2];
                    rc3b1 = mSmooCoeffs[BAND_3][3];
                    rc4b1 = mSmooCoeffs[BAND_3][4];
                    for (register long v = 0; v < numFrames; v++)
                    {
                        register float fin;
                        register float fout;

                        EQBAND2(v);
                    }
                    m_state1[BAND_2] = rs1b0;
                    m_state2[BAND_2] = rs2b0;
                    m_state1[BAND_3] = rs1b1;
                    m_state2[BAND_3] = rs2b1;
                }
            }
#endif
        }

    protected:
        void SetSmoothingCoeffs();

        float m_sr;
        float m_sa0[4];
        float m_sa1[4];
        float m_sa2[4];
        float m_sa3[4];
        float m_sa4[4];
        float m_state1[4], m_state2[4];
        float mSmooCoeffs[4][5];
    };


//######################################################################################################################

} // namespace

//######################################################################################################################

#endif /* __HPSEqualizer4Band_h__ */

#endif
