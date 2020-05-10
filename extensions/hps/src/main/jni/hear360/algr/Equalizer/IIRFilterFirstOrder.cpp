//by saul
//NOTE: I have an unused parameter for Q, may be implemented later but no need

#include <math.h>
#include "IIRFilterFirstOrder.h"

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

//deal with any denormals
 #define SNAP_TO_ZERO(n)    if (! (n < -1.0e-8f || n > 1.0e-8f)) n = 0;

IIRCoefficientsSinglePole::IIRCoefficientsSinglePole()
{
    //I don't know why memset won't work
    coefficients[0] = 0.f;
    coefficients[1] = 0.f;
    coefficients[2] = 0.f;
}

IIRCoefficientsSinglePole::~IIRCoefficientsSinglePole() {}

IIRCoefficientsSinglePole::IIRCoefficientsSinglePole (double c0, double c1, double c2, double c3)
{
    auto a = 1.0 / c2;

    coefficients[0] = (float) (c0 * a);
    coefficients[1] = (float) (c1 * a);
    coefficients[2] = (float) (c3 * a);
}

IIRCoefficientsSinglePole IIRCoefficientsSinglePole::makeLowPass (double sampleRate,
                                              double frequency)
{
    return makeLowPass (sampleRate, frequency, 1.0 / sqrt(2.0));

}

IIRCoefficientsSinglePole IIRCoefficientsSinglePole::makeLowPass (double sampleRate,
                                              double frequency,
                                              double Q)
{
    //ensure valid sample rate and freq and we're below nyquist
    if (sampleRate <= 0.0 || frequency <= 0.0 || frequency > static_cast<float>(sampleRate * 0.5))
    {
        frequency = 140.0;
        sampleRate = 48000;
    }


    auto n = tan(M_PI * frequency / sampleRate);


    return IIRCoefficientsSinglePole(n, n, n + 1, n - 1);
}

IIRCoefficientsSinglePole IIRCoefficientsSinglePole::makeHighPass (double sampleRate,
                                               double frequency)
{
    return makeHighPass (sampleRate, frequency, 1.0 / sqrt(2.0));
}

IIRCoefficientsSinglePole IIRCoefficientsSinglePole::makeHighPass (double sampleRate,
                                               double frequency,
                                               double Q)
{
    if (sampleRate <= 0.0 || frequency <= 0.0 || frequency > static_cast<float>(sampleRate * 0.5))
    {
        frequency = 140.0;
        sampleRate = 48000;
    }

    auto n = tan(M_PI * frequency / sampleRate);


    return IIRCoefficientsSinglePole(1, -1, n + 1, n - 1);
}

//==============================================================================
IIRFilterFirstOrder::IIRFilterFirstOrder() {}

IIRFilterFirstOrder::~IIRFilterFirstOrder() {}

void IIRFilterFirstOrder::setCoefficients (const IIRCoefficientsSinglePole& newCoefficients)
{
    coefficients = newCoefficients;
}

//==============================================================================
void IIRFilterFirstOrder::reset()
{
    v1 = 0.f;
}

float IIRFilterFirstOrder::processSingleSampleRaw (float in)
{
    auto out = coefficients.coefficients[0] * in + v1;

    SNAP_TO_ZERO(out);

    v1 = coefficients.coefficients[1] * in - coefficients.coefficients[2] * out;

    return out;
}

void IIRFilterFirstOrder::processInPlace (float* buffer, int length)
{
    for(int i = 0; i < length; i++)
    {
        buffer[i] = processSingleSampleRaw(buffer[i]);
    }
}

void IIRFilterFirstOrder::processOutPlace (float* inBuf, float* outBuf, int length)
{
    for(int i = 0; i < length; i++)
    {
        outBuf[i] = processSingleSampleRaw(inBuf[i]);
    }
}

//######################################################################################################################

} // namespace

//######################################################################################################################
