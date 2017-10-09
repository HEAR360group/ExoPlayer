#include <hear360/algr/defines.h>
#include <hear360/algr/Equalizer/HPSEqualizerBand.h>
#include <hear360/algr/Base/DSPUtils.h>

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

#define sin_as sin
#define cos_as cos
#define pow_as pow

// -----------------------------------------------------------------------------------------------------------------------------------------------
// convenience method to set a stereo pair of filters with one call
//
void
HPSEqBandSet2(float b0, float b1, float b2, float a0, float a1, float a2, HPSEqualizerBand * eq, HPSEqualizerBand * eq2)
{
    eq->SetSecondOrder(b0, b1, b2, a0, a1, a2);
    eq2->SetSecondOrder(b0, b1, b2, a0, a1, a2);
}

void
HPSEqBandSet2(float freq, float q, float gainDb, HPSEqualizerBand * eq, HPSEqualizerBand * eq2)
{
    eq->SetParametric(freq, gainDb, q);
    eq2->SetParametric(freq, gainDb, q);
}

// ###############################################################################################################################################

HPSEqualizerBand::HPSEqualizerBand(float sampleRate)
{
    Reset();
    m_sr = sampleRate;
    mSmooCoeffs[0]=1.0f;
    mSmooCoeffs[1] = mSmooCoeffs[2] = mSmooCoeffs[3] = mSmooCoeffs[4] = 0.0;
}

void HPSEqualizerBand::Reset(void)
{
    m_state1 = m_state2 = 0;            /* zero initial samples */
}

void HPSEqualizerBand::SetLowpass(float freq, float gainDb, float q)
{
    freq    = limit(freq, 10.0f, m_sr/2.0f-10.0f);
    q       = limit(q, 0.1f, 10.f);
    gainDb  = limit(gainDb, -300.0f, 64.0f);

    float omega = 2.0f * hear360_algr_M_PI * freq / m_sr;
    float sn = sin_as(omega);
    float cs = cos_as(omega);
    float alpha = sn/(2.0f*q);

    float b0 = (1.0f - cs) /2.;
    float b1 = 1.0f - cs;
    float b2 = (1.0f - cs) /2.;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha;

    m_sa0 = b0 / a0;
    m_sa1 = b1 / a0;
    m_sa2 = b2 / a0;
    m_sa3 = a1 / a0;
    m_sa4 = a2 / a0;
    SetSmoothingCoeffs();
}

void HPSEqualizerBand::SetHighpass(float freq, float gainDb, float q)
{
    freq    = limit(freq, 10.0f, m_sr/2.0f-10.0f);
    q       = limit(q, 0.1f, 10.f);
    gainDb  = limit(gainDb, -300.0f, 64.0f);

    float omega = 2.0f * hear360_algr_M_PI * freq / m_sr;
    float sn = sin_as(omega);
    float cs = cos_as(omega);
    float alpha = sn/(2.0f*q);

    float b0 = (1.0f + cs) /2.;
    float b1 = -(1.0f + cs);
    float b2 = (1.0f + cs) /2.;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha;

    m_sa0 = b0 / a0;
    m_sa1 = b1 / a0;
    m_sa2 = b2 / a0;
    m_sa3 = a1 / a0;
    m_sa4 = a2 / a0;

    SetSmoothingCoeffs();
}

void HPSEqualizerBand::SetBandpass(float freq, float gainDb, float q)
{

    freq    = limit(freq, 10.0f, m_sr/2.0f-10.0f);
    q       = limit(q, 0.1f, 10.f);
    gainDb  = limit(gainDb, -300.0f, 64.0f);

    float omega = 2.0f * hear360_algr_M_PI * freq / m_sr;
    float sn = sin_as(omega);
    float cs = cos_as(omega);
    float alpha = sn/(2.0f*q);

    float b0 = alpha;
    float b1 = 0;
    float b2 = -alpha;
    float a0 = 1 + alpha;
    float a1 = -2 * cs;
    float a2 = 1 - alpha;

    m_sa0 = b0 / a0;
    m_sa1 = b1 / a0;
    m_sa2 = b2 / a0;
    m_sa3 = a1 / a0;
    m_sa4 = a2 / a0;
    SetSmoothingCoeffs();
}

void HPSEqualizerBand::SetSecondOrder(float b0, float b1, float b2, float a0, float a1, float a2)
{
    m_sa0 = b0 / a0;
    m_sa1 = b1 / a0;
    m_sa2 = b2 / a0;
    m_sa3 = a1 / a0;
    m_sa4 = a2 / a0;
    SetSmoothingCoeffs();
}

void HPSEqualizerBand::SetParametric(float freq, float gainDb, float q)
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

    m_sa0 = b0 / a0;
    m_sa1 = b1 / a0;
    m_sa2 = b2 / a0;
    m_sa3 = a1 / a0;
    m_sa4 = a2 / a0;
    SetSmoothingCoeffs();
}

void HPSEqualizerBand::SetSmoothingCoeffs(void)
{
    mSmooCoeffs[0]=m_sa0;
    mSmooCoeffs[1]=m_sa1;
    mSmooCoeffs[2]=m_sa2;
    mSmooCoeffs[3]=m_sa3;
    mSmooCoeffs[4]=m_sa4;
    return;
}


//######################################################################################################################

} // namespace

//######################################################################################################################
