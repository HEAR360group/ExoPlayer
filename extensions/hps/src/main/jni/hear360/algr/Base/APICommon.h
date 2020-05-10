#ifndef __API_COMMON__
#define __API_COMMON__ 1

namespace hear360_algr
{

//######################################################################################################################

/**
 An error returned by calls to AstoundAPI. Error codes are always negative.
 */
typedef enum
{
    /** Success! No error was returned by the API call. */
    kErrNoError                 = 0,
    /** A memory allocation has failed. */
    kErrMemoryFull              = -1,
    /** An object was already initialized. */
    kErrDoubleInitialization    = -2,
    /** returned by accessor methods; all pointers to returned data are nil therefore no data was returned */
    kErrNothingToDo             = -3,
    /** One or more parameters are invalid. */
    kErrInvalidParameter        = -4,
    kErrInvalidCore             = -5,
    kErrParamNotHandled         = -6,
    kErrUnknownErr              = -7,
    kErrInitFailedErr           = -8
} HPSError;

typedef enum AstoundChannelSpecifier 
{
    /**AstoundChannelSpecifier is arranged in standard SMPTE channel order for 5.1 and 7.1 systems. 
     kLeftChannel channel is the first channel in the array. */
    kLeftChannel            = 0,
    /** kRightChannel is the second channel in the array. */
    kRightChannel,
    /** kCenterChannel is the third channel in the array. */
    kCenterChannel,
    /** kLFEChannel is the fourth channel in the array. */
    kLFEChannel,
    /** kLeftRearChannel is the fifth channel in the array. */
    kLeftRearChannel,
    /** kRightRearChannel is the sixth channel in the array. */
    kRightRearChannel,
 #ifdef IMPLEMENT_7_1_REAR_CHANNELS
    /** kLeftSideChannel is the seventh channel in the array. */
    kLeftSideChannel,
    /** kRightSideChannel is the eight channel in the array. */
    kRightSideChannel,
  #endif    //IMPLEMENT_7_1_REAR_CHANNELS

    kNumChannelSpecifiers
} AstoundChannelSpecifier;

} // namespace

//######################################################################################################################

#endif /* __API_COMMON__ */

