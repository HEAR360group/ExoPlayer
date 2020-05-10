//saul
#ifdef __cplusplus

#ifndef __HPSEqualizerBand_h__
#define __HPSEqualizerBand_h__

//######################################################################################################################

namespace hear360_algr
{

//######################################################################################################################

class IIRFilterFirstOrder;

class IIRCoefficientsSinglePole
{
public:
    //==============================================================================
    IIRCoefficientsSinglePole();
    ~IIRCoefficientsSinglePole();


    IIRCoefficientsSinglePole (double c0, double c1, double c2, double c3);

    //==============================================================================
    static IIRCoefficientsSinglePole makeLowPass (double sampleRate,
                                        double frequency);

    static IIRCoefficientsSinglePole makeLowPass (double sampleRate,
                                        double frequency,
                                        double Q);

    //==============================================================================
    static IIRCoefficientsSinglePole makeHighPass (double sampleRate,
                                         double frequency);

    static IIRCoefficientsSinglePole makeHighPass (double sampleRate,
                                         double frequency,
                                         double Q);

    //==============================================================================
    float coefficients[3];
};


class IIRFilterFirstOrder
{
public:

    IIRFilterFirstOrder();
    ~IIRFilterFirstOrder();

    //==============================================================================
    void setCoefficients (const IIRCoefficientsSinglePole& newCoefficients);

    IIRCoefficientsSinglePole getCoefficients() const    { return coefficients; }
    //==============================================================================
    void reset();

    //NOT thread-safe - no locking, etc..
    float processSingleSampleRaw (float sample);

    void processInPlace (float* buffer, int length);

    void processOutPlace (float* inBuf, float* outBuf, int length);

protected:
    //==============================================================================
    IIRCoefficientsSinglePole coefficients;
    float v1 = 0.f;
};

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif /* __IIRFilterFirstOrder_h__ */
#endif // __cplusplus
