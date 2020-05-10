//#include <stdio.h>
//#include <time.h>
//#include <math.h>

#include <hear360/test/AudioFile.h>
#include <hear360/test/Util.h>
#include <hear360/plugin/generic/dll/hps.h>
//#include <libs/ckfft/inc/ckfft/ckfft.h>
#include <hear360/plugin/generic/dll/hps-hrirfolddown.h>

#define PI (3.14159265897932)
#define SAMPLE_RATE (44100)
#define MAX_FRAMES (SAMPLE_RATE * 30)
#define BUFFER_SIZE (1024)

bool RunTest(void)
{
  AudioFile inputFile;
  inputFile.LoadWavFile("input.wav");
  unsigned short channels = inputFile.numChannels;
  // printf("channels:%d", channels);

  float *buffer = new float[MAX_FRAMES * channels];
  float *outputBuffer = new float[MAX_FRAMES * 2];

  unsigned char* content = inputFile.pContent;
  int totalFrames = inputFile.GetTotalFrames();
  if(totalFrames > MAX_FRAMES) {
      totalFrames = MAX_FRAMES;
  }
  int pages = totalFrames / BUFFER_SIZE;

  for(int i = 0; i < totalFrames; i++) {
    for(int j = 0; j < channels; j++) {
      buffer[channels * i + j] = (float)Util::ShortFrom16bitData(content + (channels * i + j) * 2) / Util::MAX_INT_16BIT;
    }
  }

  printf("channels:%d, frames:%d\n", channels, totalFrames);

  // HPS_HRIRConvolutionCore_Instance_Handle coreHandle = HPS_HRIRConvolutionCore_CreateInstance(SAMPLE_RATE);
  // HPS_HRIRConvolutionCore_LoadIR(coreHandle, 0, 0);

  HPS_HRIRFolddown_Instance_Handle hrirHandle = HPS_HRIRFolddown_CreateInstance(SAMPLE_RATE);
  HPS_HRIRFolddown_LoadIRs(hrirHandle, 0);

  // CkFftContext* context = CkFftInit(BUFFER_SIZE, kCkFftDirection_Both, NULL, NULL);
  Hear360_HPS_Instance_Handle handle = Hear360_HPS_CreateInstance(SAMPLE_RATE, 0);
  // Hear360_HPS_ProcessOutOfPlaceInterleaved(handle, 0, buffer, outputBuffer, channels, 2, false, BUFFER_SIZE);

  // float avgL = 0;
  // float avgR = 0;
  //
  // for(int i = 0; i < totalFrames; i++) {
  //     outputBuffer[2 * i] = buffer[channels * i];
  //     outputBuffer[2 * i + 1] = buffer[channels * i + 1];
  //     avgL +=   outputBuffer[2 * i];
  //     avgR +=   outputBuffer[2 * i + 1];
  // }
  // avgL /= totalFrames;
  // avgR /= totalFrames;
  //
  // printf("avgL:%f, avgR:%f\n", avgL, avgR);

  // float *temp = new float[MAX_FRAMES];
  // // float *temp2 = new float[MAX_FRAMES];
  // float **outTemp = new float*[2];
  // outTemp[0] = new float[BUFFER_SIZE];
  // outTemp[1] = new float[BUFFER_SIZE];
  // //
  // float **finalTemp = new float*[2];
  // finalTemp[0] = new float[MAX_FRAMES];
  // finalTemp[1] = new float[MAX_FRAMES];
  // //
  // for(int i = 0; i < totalFrames; i++) {
  //   temp[i] = buffer[channels * i];
  // }

  // float **temp = new float*[6];
  // temp[0] = new float[BUFFER_SIZE];
  // temp[1] = new float[BUFFER_SIZE];
  // temp[2] = new float[BUFFER_SIZE];
  // temp[3] = new float[BUFFER_SIZE];
  // temp[4] = new float[BUFFER_SIZE];
  // temp[5] = new float[BUFFER_SIZE];

  // CkFftComplex *comps = new CkFftComplex[BUFFER_SIZE];
  // CkFftComplex *tcomps = new CkFftComplex[BUFFER_SIZE];
  for(int i = 0; i < pages; i++) {
  //   for(int j = 0; j < BUFFER_SIZE; j++) {
  //     temp[0][j] = buffer[(i * BUFFER_SIZE + j) * 6];
  //     temp[1][j] = buffer[(i * BUFFER_SIZE + j) * 6 + 1];
  //     temp[2][j] = buffer[(i * BUFFER_SIZE + j) * 6 + 2];
  //     temp[3][j] = buffer[(i * BUFFER_SIZE + j) * 6 + 3];
  //     temp[4][j] = buffer[(i * BUFFER_SIZE + j) * 6 + 4];
  //     temp[5][j] = buffer[(i * BUFFER_SIZE + j) * 6 + 5];
  //   }
    // HPS_HRIRFolddown_ProcessInPlaceInterleaved(hrirHandle, buffer + channels * BUFFER_SIZE * i, channels, BUFFER_SIZE, true, false);

    // HPS_HRIRConvolutionCore_ProcessOutOfPlace(coreHandle, temp + BUFFER_SIZE * i, outTemp, BUFFER_SIZE);


    // CkFftRealForward(context, BUFFER_SIZE, temp + BUFFER_SIZE * i, comps);
    // CkFftRealInverse(context, BUFFER_SIZE, comps, temp2 + BUFFER_SIZE * i, tcomps);

    // for(int j = 0; j < BUFFER_SIZE; j++) {
    //     float src = buffer[channels * BUFFER_SIZE * i];
    //     if(src < -1.0f || src > 1.0f) {
    //       printf("input out of range: %f, at: %d\n", src, i);
    //       // if(src > 1.0f) {
    //       //   buffer[i] = 1.0f;
    //       // }
    //       // else if(src < -1.0f) {
    //       //   buffer[i] = -1.0f;
    //       // }
    //     }
    // }
    // for(int j = 0; j < BUFFER_SIZE; j++) {
    //     buffer[channels * BUFFER_SIZE * i + j] *= 5.0f;
    // }
    Hear360_HPS_ProcessInPlaceInterleaved(handle, 0, buffer + channels * BUFFER_SIZE * i, channels, false, BUFFER_SIZE);
    for(int j = 0; j < BUFFER_SIZE; j++) {
        float src = buffer[channels * BUFFER_SIZE * i + j];
        if(src < -1.0f || src > 1.0f) {
          printf("output out of range: %f, at: %d\n", src, channels * BUFFER_SIZE * i + j);
          // if(src > 1.0f) {
          //   buffer[i] = 1.0f;
          // }
          // else if(src < -1.0f) {
          //   buffer[i] = -1.0f;
          // }
        }
    }
    // Hear360_HPS_ProcessOutOfPlaceInterleaved(handle, 0, buffer + BUFFER_SIZE * channels * i, outputBuffer + BUFFER_SIZE * 2 * i, channels, 2, false, BUFFER_SIZE);

    // Hear360_HPS_ProcessOutOfPlace(handle, 0.0, temp, outTemp, channels, false, BUFFER_SIZE);
    //
    // for(int j = 0; j < BUFFER_SIZE; j++) {
    //   finalTemp[0][i * BUFFER_SIZE + j] = outTemp[0][j];
    //   finalTemp[1][i * BUFFER_SIZE + j] = outTemp[1][j];
    // }
  }
  // Hear360_HPS_ProcessOutOfPlaceInterleaved(handle, 0, buffer, outputBuffer, channels, 2, false, totalFrames);
  // Hear360_HPS_ProcessInPlaceInterleaved(handle, 0, buffer, channels, false, totalFrames);
  //

  // CkFftShutdown(context);

  // unsigned char* outputContent = new unsigned char[MAX_FRAMES * 2 * 2];
  // for(int i = 0; i < totalFrames; i++) {
  //   for(int j = 0; j < 2; j++) {
  //     unsigned char data[2];
  //     Util::floatTo16bitData(finalTemp[j][i] / Util::MAX_INT_16BIT, data);
  //     outputContent[(2 * i + j) * 2] = data[0];
  //     outputContent[(2 * i + j) * 2 + 1] = data[1];
  //   }
  // }

  unsigned char* outputContent = new unsigned char[MAX_FRAMES * 2 * 2];
  for(int i = 0; i < totalFrames; i++) {
    for(int j = 0; j < 2; j++) {
      unsigned char data[2];
      Util::floatTo16bitData(buffer[channels * i + j], data);
      outputContent[(2 * i + j) * 2] = data[0];
      outputContent[(2 * i + j) * 2 + 1] = data[1];
    }
  }

  // unsigned char* outputContent = new unsigned char[MAX_FRAMES * 2 * 2];
  // for(int i = 0; i < totalFrames; i++) {
  //   for(int j = 0; j < 2; j++) {
  //     unsigned char data[2];
  //     Util::floatTo16bitData(outputBuffer[2 * i + j] / Util::MAX_INT_16BIT, data);
  //     outputContent[(2 * i + j) * 2] = data[0];
  //     outputContent[(2 * i + j) * 2 + 1] = data[1];
  //   }
  // }

  // unsigned char* outputContent = new unsigned char[totalFrames * 2];
  // for(int i = 0; i < totalFrames; i++) {
  //     unsigned char data[2];
  //     Util::floatTo16bitData(outputBuffer[2 * i] / Util::MAX_INT_16BIT, data);
  //     outputContent[(2 * i)] = data[0];
  //     outputContent[(2 * i) + 1] = data[1];
  // }

  // unsigned char* outputContent = new unsigned char[totalFrames * 2];
  // for(int i = 0; i < totalFrames; i++) {
  //     unsigned char data[2];
  //     Util::floatTo16bitData(temp2[i] / BUFFER_SIZE / Util::MAX_INT_16BIT, data);
  //     outputContent[(2 * i)] = data[0];
  //     outputContent[(2 * i) + 1] = data[1];
  // }

  AudioFile outputFile;
  outputFile.SetMeta(SAMPLE_RATE, 16, 2, totalFrames);
  outputFile.pContent = outputContent;
  // outputFile.SetContent(outputContent, totalFrames * 2 * 2);

  outputFile.SaveWavFile("output.wav");
  outputFile.ClearFile();
  inputFile.ClearFile();
  //
  delete[] buffer;
  // delete[] content;

  return true;
}

int main(void)
{
  RunTest();
}
