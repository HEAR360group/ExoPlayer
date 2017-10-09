#include <hear360/algr/defines.h>
#include <hear360/algr/Equalizer/HPSEqualizer4Band.h>
#include <hear360/algr/Base/DSPUtils.h>

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

#define sin_as sin
#define cos_as cos
#define pow_as pow

HPSEqualizer4Band::HPSEqualizer4Band(float sampleRate)
{
    m_sr = sampleRate;

    for (int i=0; i<4; i++)
    {
        m_state1[i]=m_state2[i]=0.0f;
        mSmooCoeffs[i][0] = 1.0f;
        mSmooCoeffs[i][1] = mSmooCoeffs[i][2] = mSmooCoeffs[i][3] = mSmooCoeffs[i][4] = 0.0f;
    }
}

void
HPSEqualizer4Band::Reset(int whichBand)
{
    /* zero initial samples */
    m_state1[whichBand] = m_state2[whichBand] = 0;
}       void
HPSEqualizer4Band::ResetAll()
{
    for (int band=0; band<4; band++)
        this->Reset(band);
}



void
HPSEqualizer4Band::Set(int whichBand, float freq, float gainDb, float q)
{
    freq    = limit(freq, 10.0f, m_sr/2.0f-10.0f);
    q       = limit(q, 0.1f, 10.f);
    gainDb  = limit(gainDb, -300.0f, 64.0f);

    float A = pow_as(10.0f, gainDb / 40.0f);
    float omega = 2.0f * hear360_algr_M_PI * freq / m_sr;
    float sn = sin_as(omega);
    float cs = cos_as(omega);
    float alpha = sn/(2.0f*q);

    float b0 = 1.0f + (alpha * A);
    float b1 = -2.0f * cs;
    float b2 = 1.0f - (alpha * A);
    float a0 = 1.0f + (alpha /A);
    float a1 = -2.0f * cs;
    float a2 = 1.0f - (alpha /A);

    m_sa0[whichBand] = b0 / a0;
    m_sa1[whichBand] = b1 / a0;
    m_sa2[whichBand] = b2 / a0;
    m_sa3[whichBand] = a1 / a0;
    m_sa4[whichBand] = a2 / a0;

    SetSmoothingCoeffs();
}

void
HPSEqualizer4Band::SetSmoothingCoeffs()
{
    for (register int whichBand=0; whichBand<4; whichBand++)
    {
        mSmooCoeffs[whichBand][0]=m_sa0[whichBand];
        mSmooCoeffs[whichBand][1]=m_sa1[whichBand];
        mSmooCoeffs[whichBand][2]=m_sa2[whichBand];
        mSmooCoeffs[whichBand][3]=m_sa3[whichBand];
        mSmooCoeffs[whichBand][4]=m_sa4[whichBand];
    }
    return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################
