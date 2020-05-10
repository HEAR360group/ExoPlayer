
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_plugin_generic_dsp_vsheadtracking_H
#define hear360_plugin_generic_dsp_vsheadtracking_H

//######################################################################################################################
#include <math.h>
//######################################################################################################################

namespace hear360_plugin_generic_dsp_vsheadtracking
{

//######################################################################################################################

class Vector3d
{
  public:
  float x;
  float y;
  float z;

  Vector3d()
  {
    x = 0;
    y = 0;
    z = 0;
  }

  Vector3d(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  static void rotate(float theta, Vector3d& in, Vector3d& out) {
    float cosTheta = cosf(theta);
    float sinTheta = sinf(theta);
    //Vector3d out = new Vector3d();
    out.x = in.x * cosTheta + in.z * sinTheta;
    out.y = in.y;
    out.z = -in.x * sinTheta + in.z * cosTheta;
    out.normalize();
  }

  float dot(Vector3d v1) {
    return v1.x * x + v1.y * y + v1.z * z;
  }

  void cross(Vector3d v1, Vector3d v2)
  {
    float x, y;

    x = v1.y*v2.z - v1.z*v2.y;
    y = v2.x*v1.z - v2.z*v1.x;
    this->z = v1.x*v2.y - v1.y*v2.x;
    this->x = x;
    this->y = y;
  }

  void normalize()
  {
    float norm;

    norm = 1.0/sqrt(x * x + y * y + z * z);
    x *= norm;
    y *= norm;
    z *= norm;
  }

  float length()
  {
    return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
  }

  float angle(Vector3d v1)
   {
      float vDot = this->dot(v1) / ( this->length()*v1.length() );
      if( vDot < -1.0) vDot = -1.0;
      if( vDot >  1.0) vDot =  1.0;
      return((float) (acos( vDot )));
   }
};

class DegreeDiff {
    public:
    float dotValue;
    float crossValue;
    float angleValue;
    int speakerIndex;

    DegreeDiff& operator=(const DegreeDiff& other) // copy assignment
    {
      if (this != &other) { // self-assignment check expected
        this->dotValue = other.dotValue;
        this->crossValue = other.crossValue;
        this->angleValue = other.angleValue;
        this->speakerIndex = other.speakerIndex;
      }
      return *this;
    }
};

//######################################################################################################################

#define HEAR360_PLUGIN_DSP_VECDOTPRODUCT(v1, v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)

#define HEAR360_PLUGIN_DSP_VECMAGNITUDE(v) (sqrtf(v.x * v.x + v.y * v.y + v.z * v.z))

#define HEAR360_PLUGIN_DSP_VECNORMALIZE(v) \
{ \
	float magnitude = HEAR360_PLUGIN_DSP_VECMAGNITUDE(v); \
	v.x /= magnitude; \
	v.y /= magnitude; \
	v.z /= magnitude; \
}

#define HEAR360_PLUGIN_DSP_VECCROSSPRODUCT(v1, v2, v3) \
	v3.x = v1.y * v2.z - v1.z * v2.y; \
	v3.y = v1.z * v2.x - v1.x * v2.z; \
	v3.z = v1.x * v2.y - v1.y * v2.x

#define HEAR360_PLUGIN_DSP_ANGLE(v1, angle) \
{ \
  double vDot = this.dot(v1) / ( this.length()*v1.length() ); \
  if( vDot < -1.0) vDot = -1.0; \
  if( vDot >  1.0) vDot =  1.0; \
  angle = Math.acos( vDot ); \
}

#define HEAR360_PLUGIN_DSP_ROTATEABOUTAXIS(v1, angle, vaxis, v2) \
{ \
	float s = sinf(angle); \
	float c = cosf(angle); \
	float k = 1.0f - c; \
	v2.x = v1.x * (c + k * vaxis.x * vaxis.x) + v1.y * (k * vaxis.x * vaxis.y - s * vaxis.z) + v1.z * (k * vaxis.x * vaxis.z + s * vaxis.y); \
    v2.y = v1.x * (k * vaxis.x * vaxis.y + s * vaxis.z) + v1.y * (c + k * vaxis.y * vaxis.y) + v1.z * (k * vaxis.y * vaxis.z - s * vaxis.x);  \
    v2.z = v1.x * (k * vaxis.x * vaxis.z - s * vaxis.y) + v1.y * (k * vaxis.y * vaxis.z + s * vaxis.x) + v1.z * (c + k * vaxis.z * vaxis.z); \
}

#define HEAR360_PLUGIN_DSP_CLAMP(value, min, max) ((value < min) ? min : ((value > max) ? max : value))

#define HEAR360_PLUGIN_DSP_FLOAT2BOOL(x) ((x == 0.0f) ? false : true)

#define HEAR360_PLUGIN_DSP_ILLEGAL_PI (3.14159266f)
#define HEAR360_PLUGIN_DSP_PI (3.14159265f)
#define HEAR360_PLUGIN_DSP_HALF_PI (1.570796325f)

#define MAX_CHANNEL_COUNT (8)
#define DEFAULT_CHANNEL_COUNT (8)
#define FRACTION_STEP (0.35f)

#define HAS_LFE (true)
#define LFE_CHANNEL_ID (3)

//#define FRONT_VEC (Vector3d(0, 0, 1))
//#define Y_AXIS (Vector3d(0, 1, 0))

//######################################################################################################################

//Vector3d FRONT_VEC(0, 0, 1);
//Vector3d Y_AXIS(0, 1, 0);

//######################################################################################################################

  void getVolumeMatrix(void* handle, float* outVolumeMatrix);

  void* CreateInstance(int samplerate, bool isHeight);
  bool DeleteInstance(void* handle);
  //bool ProcessInPlace(void* handle, float azimuth, float** pBuf, int srcChannels, long totalsamples);
  //bool ProcessInPlaceInterleaved(void* handle, float azimuth, float* pBuf, int srcChannels, long totalsamples);
  bool ProcessOutOfPlace(void* handle, float azimuth, const float** pInBuf, float** pOutBuf, int srcChannels, long totalsamples);
  bool ProcessOutOfPlaceInterleaved(void* handle, float azimuth, const float* pInBuf, float* pOutBuf, int srcChannels, long totalsamples);

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // include guard

//######################################################################################################################
