/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <jni.h>
#include <stdlib.h>
#include <android/log.h>
#include <hear360/plugin/generic/dll/hps-hrirfolddown.h>

extern "C" {
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>
#endif
}

#define LOG_TAG "hps_jni"
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, \
                   __VA_ARGS__))

#define HPS_FUNC(RETURN_TYPE, NAME, ...) \
  extern "C" { \
  JNIEXPORT RETURN_TYPE \
    Java_com_google_android_exoplayer2_ext_hps_HPSAudioDSP_ ## NAME \
      (JNIEnv *env, jobject thiz, ##__VA_ARGS__);\
  } \
  JNIEXPORT RETURN_TYPE \
    Java_com_google_android_exoplayer2_ext_hps_HPSAudioDSP_ ## NAME \
      (JNIEnv *env, jobject thiz, ##__VA_ARGS__)\

#define ERROR_STRING_BUFFER_LENGTH 256

/**
 * Outputs a log message describing the avcodec error number.
 */
void logError(const char *functionName, int errorNumber);

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  return JNI_VERSION_1_6;
}

HPS_FUNC(jlong, createInstance, jint samplerate) {
  return (jlong) HPS_HRIRFolddown_CreateInstance(samplerate);
}

HPS_FUNC(jint, deleteInstance, jlong context) {
  if (context) {
    return HPS_HRIRFolddown_DeleteInstance((HPS_HRIRFolddown_Instance_Handle *) context);
  }
  else {
    return 0;
  }
}

HPS_FUNC(void, loadIRs, jlong context, jint presetID) {
  if (context) {
    HPS_HRIRFolddown_LoadIRs((HPS_HRIRFolddown_Instance_Handle *) context, presetID);
  }
}

HPS_FUNC(jint, processInPlaceInterleaved, jlong context, jfloatArray pBuf, jint srcChannels, jlong totalsamples, jboolean hpsEnabled, jboolean warmEQEnabled) {
  if (context) {
    jfloat *pcBuf;
    pcBuf = env->GetFloatArrayElements(pBuf, 0);

    if(pcBuf == NULL) {
      return 0;
    }

    jint result = HPS_HRIRFolddown_ProcessInPlaceInterleaved((HPS_HRIRFolddown_Instance_Handle *) context, pcBuf, srcChannels, (long)totalsamples, (bool)hpsEnabled, (bool)warmEQEnabled);

    env->ReleaseFloatArrayElements(pBuf, pcBuf, 0);

    return result;
  }
  else {
    return 0;
  }
}

HPS_FUNC(jint, processOutOfPlaceInterleaved, jlong context, jfloatArray pInBuf, jfloatArray pOutBuf, jint srcChannels, jlong totalsamples, jboolean hpsEnabled, jboolean warmEQEnabled) {
  if (context) {
    jfloat *pcInBuf;
    pcInBuf = env->GetFloatArrayElements(pInBuf, 0);

    jfloat *pcOutBuf;
    pcOutBuf = env->GetFloatArrayElements(pOutBuf, 0);

    if(pcInBuf == NULL || pcOutBuf == NULL) {
      return 0;
    }

    jint result = HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved((HPS_HRIRFolddown_Instance_Handle *) context, pcInBuf, pcOutBuf, srcChannels, (long)totalsamples, (bool)hpsEnabled, (bool)warmEQEnabled);
/*
    if(totalsamples != 0) {
      jboolean hasNonZero = false;

      for(int i = 0; i < srcChannels * totalsamples; i++) {
        if(pcOutBuf[i] != 0) {
          hasNonZero = true;
          break;
        }
      }

      if(!hasNonZero) {
        static int counter = 0;
        counter++;
        if(counter > 10) {
          abort();
        }
      }
    }
*/
    //pcOutBuf[0] = 0.1f;
/*
    for(int i = 0; i < srcChannels * totalsamples; i++) {
      pcOutBuf[i] = pcInBuf[i];
    }
*/
    env->ReleaseFloatArrayElements(pInBuf, pcInBuf, 0);

    env->ReleaseFloatArrayElements(pOutBuf, pcOutBuf, 0);

    return result;
  }
  else {
    return 0;
  }
}
