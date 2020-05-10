
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#include <algorithm>
#include <cstring>
#include <map>

#include <hear360/dsp/os/memory.h>
#include <hear360/plugin/generic/dsp/vsheadtracking.h>
#include <hear360/algr/Base/MultiData.h>

//######################################################################################################################

namespace hear360_plugin_generic_dsp_vsheadtracking
{

#define LOG (0)

#if (LOG)
  #define LOGFILE "d:\\generichrirfolddown.txt"
#endif

#if (LOG)
  #include <cstdio>
#endif

//######################################################################################################################

struct BUFFER
{
  //float** input;
  //float* temp;
  //float** destination;
};

//######################################################################################################################

struct PRIVATE
{
  //BUFFER buffer;
  float speakerPos[MAX_CHANNEL_COUNT];
  Vector3d speakerVec[MAX_CHANNEL_COUNT];
  Vector3d rotatedSpeakerVec[MAX_CHANNEL_COUNT];
  float volumeMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];
  float interpolatedMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];
  //float flatVolumeMatrix[MAX_CHANNEL_COUNT * MAX_CHANNEL_COUNT];
  float lastAzimuth;
  //float interpolatedAzimuth;

  hear360_dsp_os_memory::MANAGER memorymanager;

  Vector3d FRONT_VEC;
  Vector3d Y_AXIS;

  PRIVATE (hear360_dsp_os_memory::MANAGER memorymanager, int samplerate, bool isHeight);
  ~PRIVATE();
};

//######################################################################################################################

PRIVATE::PRIVATE (hear360_dsp_os_memory::MANAGER memorymanagerparam, int samplerate, bool isHeight)
: memorymanager (memorymanagerparam)
//, lastAzimuth (0.0f)
//, interpolatedAzimuth(0.0f)
{
  FRONT_VEC = Vector3d(0, 0, 1);
  Y_AXIS = Vector3d(0, 1, 0);

  for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
  {
    for(int j = 0; j < MAX_CHANNEL_COUNT; j++)
    {
      volumeMatrix[i][j] = 0;
      interpolatedMatrix[i][j] = 0;
    }
  }

  if(isHeight) {
    speakerPos[0] = -45.0f / 180 * M_PI;
    speakerPos[1] = 45.0f / 180 * M_PI;
    speakerPos[2] = -135.0f / 180 * M_PI;
    speakerPos[3] = 0.0f / 180 * M_PI;
    speakerPos[4] = 135.0f / 180 * M_PI;
    speakerPos[5] = 0.0f / 180 * M_PI;
    speakerPos[6] = 0.0f / 180 * M_PI;
    speakerPos[7] = 0.0f / 180 * M_PI;
  }
  else {
    speakerPos[0] = -45.0f / 180 * M_PI;
    speakerPos[1] = 45.0f / 180 * M_PI;
    speakerPos[2] = 0.0f / 180 * M_PI;
    speakerPos[3] = 0.0f / 180 * M_PI;
    speakerPos[4] = -135.0f / 180 * M_PI;
    speakerPos[5] = 135.0f / 180 * M_PI;
    speakerPos[6] = -90.0f / 180 * M_PI;
    speakerPos[7] = 90.0f / 180 * M_PI;
  }

  for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
    Vector3d::rotate(speakerPos[i], FRONT_VEC, speakerVec[i]);
    //HEAR360_PLUGIN_DSP_ROTATEABOUTAXIS(FRONT_VEC, speakerPos[i], Y_AXIS, speakerVec[i]);
  }
}

//######################################################################################################################

PRIVATE::~PRIVATE ()
{

}

//######################################################################################################################

struct GLOBALDATA
{
  //unsigned int nextID;
  //std::map<unsigned int, PRIVATE*> hrirfolddownInstances;

  #if (LOG)
  FILE *pfile;
  #endif

  GLOBALDATA ();
  ~GLOBALDATA();
};

static GLOBALDATA gdata;

GLOBALDATA::GLOBALDATA()
{
  #if (LOG)

  gdata.pfile = std::fopen (LOGFILE, "a");

  std::fprintf (gdata.pfile, "Opening log.\n\n");

  std::fflush (gdata.pfile);

  #endif
}

GLOBALDATA::~GLOBALDATA()
{
  #if (LOG)

  std::fprintf (gdata.pfile, "Closing log.\n\n");

  std::fclose (gdata.pfile);

  #endif
}

//######################################################################################################################

void* CreateInstance(int samplerate, bool isHeight)
{
  PRIVATE *pprivate;

  pprivate = new PRIVATE(hear360_dsp_os_memory::MANAGER(), samplerate, isHeight);

  return (void*)pprivate;
}

bool DeleteInstance(void* handle)
{
  if (handle == NULL)
    return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
    return false;

  delete pprivate;
  handle = NULL;

  return true;
}

//######################################################################################################################

void getVolumeMatrix(void* handle, float* outVolumeMatrix) {
  PRIVATE* pprivate = (PRIVATE*)handle;

  for(int outIndex = 0; outIndex < MAX_CHANNEL_COUNT; outIndex++) {
    for (int inIndex = 0; inIndex < MAX_CHANNEL_COUNT; inIndex++) {
      outVolumeMatrix[inIndex * MAX_CHANNEL_COUNT + outIndex] = pprivate->volumeMatrix[inIndex][outIndex];
    }
  }
}

//######################################################################################################################

int orderInsert(DegreeDiff* arr, int first, int last, DegreeDiff& target) {
  int i = last;
  while((i > first) && (target.angleValue < arr[i - 1].angleValue)) {
    //while((i > first) && (target.dotValue > arr[i - 1].dotValue)) {
    arr[i] = arr[i - 1];
    i = i - 1;
  }
  arr[i] = target;
  return i;
}

//######################################################################################################################

// void smoothMatrix(float midMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT], float toMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT]) {
//   for(int i = 0; i < MAX_CHANNEL_COUNT; i++) {
//     for (int j = 0; j < MAX_CHANNEL_COUNT; j++) {
//       midMatrix[i][j] += FRACTION_STEP * (toMatrix[i][j] - midMatrix[i][j]);
//     }
//   }
// }
//
// void smoothAzimuth(float& midAzimuth, float& toAzimuth) {
//   midAzimuth += FRACTION_STEP * (toAzimuth - midAzimuth);
// }

//######################################################################################################################

void CalculateVolumeMatrix(void* handle, float azimuth, int srcChannels)
{
  if(azimuth > HEAR360_PLUGIN_DSP_ILLEGAL_PI || azimuth < -HEAR360_PLUGIN_DSP_ILLEGAL_PI) {
    //azimuth = 0;
    //abort();
  }

  PRIVATE* pprivate = (PRIVATE*)handle;

  // for(int outIndex = 0; outIndex < DEFAULT_CHANNEL_COUNT; outIndex++) {
  //   for(int inIndex = 0; inIndex < DEFAULT_CHANNEL_COUNT; inIndex++) {
  //     pprivate->volumeMatrix[inIndex][outIndex] = 0;
  //   }
  // }
  //
  // //pprivate->volumeMatrix[0][0] = 0;
  // //pprivate->volumeMatrix[0][1] = 0;
  // pprivate->volumeMatrix[0][2] = 1.0f;
  // //pprivate->volumeMatrix[5][1] = 0.5;
  //
  // return;

  Vector3d rotatedFrontVec;
  Vector3d::rotate(azimuth, pprivate->FRONT_VEC, rotatedFrontVec);

  for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
    if(HAS_LFE && i == LFE_CHANNEL_ID)
      continue;

    Vector3d::rotate(pprivate->speakerPos[i], rotatedFrontVec, pprivate->rotatedSpeakerVec[i]);
    //HEAR360_PLUGIN_DSP_ROTATEABOUTAXIS(rotatedFrontVec, pprivate->speakerPos[i], Y_AXIS, pprivate->rotatedSpeakerVec[i]);
  }

  //For each input channel
  for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
    if(HAS_LFE && i == LFE_CHANNEL_ID)
      continue;

    DegreeDiff degreeDiffArrayL[MAX_CHANNEL_COUNT];
    DegreeDiff degreeDiffArrayR[MAX_CHANNEL_COUNT];
    int degreeDiffArrayCountL = 0;
    int degreeDiffArrayCountR = 0;

    int pointToSpeakerPosID = 0;

    for (int speakerPosID = 0; speakerPosID < DEFAULT_CHANNEL_COUNT; speakerPosID++) {
      if(HAS_LFE && speakerPosID == LFE_CHANNEL_ID)
        continue;

      //double dotValue = rotatedSpeakerVec[i].dot(speakerVec[speakerPosID]);
      Vector3d crossVec;
      crossVec.cross(pprivate->rotatedSpeakerVec[i], pprivate->speakerVec[speakerPosID]);
      float angleValue = pprivate->rotatedSpeakerVec[i].angle(pprivate->speakerVec[speakerPosID]);
      //if(dotValue <= 0)
      //continue;

      DegreeDiff degreeDiff;
      //degreeDiff.dotValue = dotValue;
      degreeDiff.crossValue = crossVec.y;
      degreeDiff.angleValue = angleValue;
      degreeDiff.speakerIndex = speakerPosID;

      if(degreeDiff.angleValue == 0 /*|| Math.abs(degreeDiff.dotValue) < 0.001*/) {
        degreeDiffArrayCountL = 0;
        degreeDiffArrayCountR = 0;
        pointToSpeakerPosID = speakerPosID;
        break;
      }
      //Select the nearest speaker from left
      else if(degreeDiff.crossValue < 0) {
        orderInsert(degreeDiffArrayL, 0, degreeDiffArrayCountL, degreeDiff);
        degreeDiffArrayCountL++;
      }
      //Select the nearest speaker from right
      else {
        orderInsert(degreeDiffArrayR, 0, degreeDiffArrayCountR, degreeDiff);
        degreeDiffArrayCountR++;
      }
    }

    if(degreeDiffArrayCountL != 0 && degreeDiffArrayCountR != 0) {
      //Calculate volume distribution
      int speakerIndex0 = degreeDiffArrayL[0].speakerIndex;
      float angle0 = degreeDiffArrayL[0].angleValue;
      //double dot0 = degreeDiffArrayL[0].dotValue;
      int speakerIndex1 = degreeDiffArrayR[0].speakerIndex;
      float angle1 = degreeDiffArrayR[0].angleValue;
      //double dot1 = degreeDiffArrayR[0].dotValue;

      //double actAngle0 = Math.sin(angle0);
      //double actAngle1 = Math.cos(angle1);
      float speaker0Vol = angle1 / (angle0 + angle1);
      float speaker1Vol = angle0 / (angle0 + angle1);
      //double speaker0ActVol = Math.sin(speaker0Vol * Math.PI / 2);
      //double speaker1ActVol = Math.cos(speaker0Vol * Math.PI / 2);

      // if((angle0 + angle1) == 0) {
      //   abort();
      // }

      // if(angle0 < 0 || angle1 < 0) {
      //   abort();
      // }

      // if((angle0 + angle1) > (M_PI / 2)) {
      //   abort();
      // }

      //double speaker0Vol = dot0 / (dot0 + dot1);
      //double speaker1Vol = dot1 / (dot0 + dot1);

      pprivate->volumeMatrix[i][speakerIndex0] = speaker0Vol;
      pprivate->volumeMatrix[i][speakerIndex1] = speaker1Vol;

      for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
        if(j == speakerIndex0 || j == speakerIndex1)
          continue;

        if(HAS_LFE && j == LFE_CHANNEL_ID)
          continue;

        pprivate->volumeMatrix[i][j] = 0;
      }
    }
    else {
      pprivate->volumeMatrix[i][pointToSpeakerPosID] = 1;

      for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
        if(j == pointToSpeakerPosID)
          continue;

        if(HAS_LFE && j == LFE_CHANNEL_ID)
          continue;

        pprivate->volumeMatrix[i][j] = 0;
      }
    }
  }

  if(HAS_LFE)
    pprivate->volumeMatrix[LFE_CHANNEL_ID][LFE_CHANNEL_ID] = 1;

    //For stereo sound track, split the center SPL to L and R and disable the center
  if(srcChannels == 2) {
    for(int i = 0; i < MAX_CHANNEL_COUNT; i++) {
      double centerVolume = pprivate->volumeMatrix[i][2];
      pprivate->volumeMatrix[i][2] = 0;
      pprivate->volumeMatrix[i][0] += (centerVolume / 2.0);
      pprivate->volumeMatrix[i][1] += (centerVolume / 2.0);
    }
  }

  // for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
  //   for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
  //     pprivate->volumeMatrix[i][j] = 0;
  //   }
  // }
  //
  // for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
  //   pprivate->volumeMatrix[j][j] = 1;
  // }
}

//######################################################################################################################

bool ProcessOutOfPlaceInterleaved(void* handle, float azimuth, const float* pInBuf, float* pOutBuf, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	return false;

  if (pOutBuf == NULL)
	return false;

  //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	//return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	return false;

  CalculateVolumeMatrix(handle, azimuth, srcChannels);

  float curMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];

  for(int i = 0; i < totalsamples; i++) {
    for(int k = 0; k < MAX_CHANNEL_COUNT; k++) {
      for (int j = 0; j < MAX_CHANNEL_COUNT; j++) {
        curMatrix[k][j] = pprivate->interpolatedMatrix[k][j] + (pprivate->volumeMatrix[k][j] - pprivate->interpolatedMatrix[k][j]) * i / totalsamples;
      }
    }
    //float curAzimuth = pprivate->lastAzimuth + (azimuth - pprivate->lastAzimuth) * i / totalsamples;
    //CalculateVolumeMatrix(handle, curAzimuth, srcChannels);

    for(int outIndex = 0; outIndex < DEFAULT_CHANNEL_COUNT; outIndex++) {
      float output = 0.0f;
      for(int inIndex = 0; inIndex < srcChannels; inIndex++) {
        float input = pInBuf[inIndex + i * srcChannels];
        //float mixedInput = input * pprivate->volumeMatrix[inIndex][outIndex];
        float mixedInput = input * curMatrix[inIndex][outIndex];
        output += mixedInput;
      }
      pOutBuf[outIndex + i * DEFAULT_CHANNEL_COUNT] = output;
    }
  }

  for(int k = 0; k < MAX_CHANNEL_COUNT; k++) {
    for (int j = 0; j < MAX_CHANNEL_COUNT; j++) {
      pprivate->interpolatedMatrix[k][j] = pprivate->volumeMatrix[k][j];
    }
  }

  //pprivate->lastAzimuth = azimuth;

  return true;
}

bool ProcessOutOfPlace(void* handle, float azimuth, const float** pInBuf, float** pOutBuf, int srcChannels, long totalsamples)
{
  if (handle == NULL)
    return false;

  if (pInBuf == NULL)
	return false;

  if (pOutBuf == NULL)
	return false;

  //if (totalsamples > hear360_dsp_os_memory_AUDIOBUFFERSIZE)
	//return false;

  PRIVATE* pprivate = (PRIVATE*)handle;

  if (pprivate == NULL)
	return false;

  CalculateVolumeMatrix(handle, azimuth, srcChannels);

  float curMatrix[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];

  for(int i = 0; i < totalsamples; i++) {
    for(int k = 0; k < MAX_CHANNEL_COUNT; k++) {
      for (int j = 0; j < MAX_CHANNEL_COUNT; j++) {
        curMatrix[k][j] = pprivate->interpolatedMatrix[k][j] + (pprivate->volumeMatrix[k][j] - pprivate->interpolatedMatrix[k][j]) * i / totalsamples;
      }
    }
    //float curAzimuth = pprivate->lastAzimuth + (azimuth - pprivate->lastAzimuth) * i / totalsamples;
    //CalculateVolumeMatrix(handle, curAzimuth, srcChannels);

    for(int outIndex = 0; outIndex < DEFAULT_CHANNEL_COUNT; outIndex++) {
      float output = 0.0f;
      for(int inIndex = 0; inIndex < srcChannels; inIndex++) {
        float input = pInBuf[inIndex][i];
        float mixedInput = input * curMatrix[inIndex][outIndex];
        output += mixedInput;
      }
      pOutBuf[outIndex][i] = output;
    }
  }
/*
  for(int outIndex = 0; outIndex < DEFAULT_CHANNEL_COUNT; outIndex++) {
    memset(pprivate->buffer.temp, 0, hear360_dsp_os_memory_AUDIOBUFFERSIZE * sizeof(float));
    for(int inIndex = 0; inIndex < DEFAULT_CHANNEL_COUNT; inIndex++) {
        const float* input = pInBuf[inIndex];
        hear360_algr::MixMonoByScalarSIMD(pprivate->buffer.temp, input, pprivate->volumeMatrix[inIndex][outIndex], totalsamples);
    }
    hear360_algr::CopyMonoSIMD(pOutbuf[outIndex], pprivate->buffer.temp, totalsamples);
  }
*/
  for(int k = 0; k < MAX_CHANNEL_COUNT; k++) {
    for (int j = 0; j < MAX_CHANNEL_COUNT; j++) {
      pprivate->interpolatedMatrix[k][j] = pprivate->volumeMatrix[k][j];
    }
  }

  //pprivate->lastAzimuth = azimuth;

  return true;
}
//######################################################################################################################

} // namespace

//######################################################################################################################
