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
package com.google.android.exoplayer2.ext.hps;

//import com.google.android.exoplayer2.C;

/**
 * Opus decoder.
 */
public final class HPSAudioDSP {

  private final long dspContext;
  private float[] inputBuf;
  //private float[] htBuf;
  private float[] outputBuf;
  private float[] volumeMatrix;

  public HPSAudioDSP(int samplerate, int presetID) {
    dspContext = Hear360HPSCreateInstance(samplerate, presetID);

    inputBuf = new float[40000];
    //htBuf = new float[30000];
    outputBuf = new float[40000];
    volumeMatrix = new float[64];
  }

  public void release() {
    if(dspContext != 0) {
      Hear360HPSDeleteInstance(dspContext);
    }
  }

  public int ProcessInPlaceInterleavedFloat(float azimuth, float[] pBuf, int srcChannels, boolean stereoUpMix51, long totalsamples) {
    Hear360HPSProcessInPlaceInterleaved(dspContext, azimuth, pBuf, srcChannels, stereoUpMix51, totalsamples);
    //HPSHRIRFolddownProcessInPlaceInterleaved(nativeDSPContext, pBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);
    return 0;
  }

  public int ProcessOutOfPlaceInterleavedFloat(float azimuth, float[] pInBuf, float[] pOutBuf, int srcChannels, int dstChannels, boolean stereoUpMix51, long totalsamples) {
//    GetVolumeMatrix();

    Hear360HPSProcessOutOfPlaceInterleaved(dspContext, azimuth, pInBuf, pOutBuf, srcChannels, dstChannels, stereoUpMix51, totalsamples);

    return 0;
  }

  public int ProcessInPlaceInterleavedShort(short[] pShortBuf, int srcChannels, long totalsamples) {
    for(int i = 0; i < pShortBuf.length; i++) {
      inputBuf[i] = (float)pShortBuf[i] / 32767.0f;
    }

    int result = 0;
    //HPSHRIRFolddownProcessInPlaceInterleaved(nativeDSPContext, inputBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);

    for(int i = 0; i < pShortBuf.length; i++) {
      inputBuf[i] = (float)pShortBuf[i] * 32767.0f;
    }

    return result;
  }

  public int ProcessOutOfPlaceInterleavedShort(short[] pInShortBuf, short[] pOutShortBuf, int srcChannels, long totalsamples) {
    for(int i = 0; i < pInShortBuf.length; i++) {
      inputBuf[i] = (float)pInShortBuf[i] / 32767.0f;
    }

    int result = 0;
    //HPSHRIRFolddownProcessOutOfPlaceInterleaved(nativeDSPContext, inputBuf, outputBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);

    for(int i = 0; i < pOutShortBuf.length; i++) {
      outputBuf[i] = (float)pOutShortBuf[i] * 32767.0f;
    }

    return result;
  }

  public void GetVolumeMatrix() {
    Hear360HPSGetVolumeMatrix(dspContext, volumeMatrix);
    return;
  }

  private native long HPSHRIRFolddownCreateInstance(int samplerate);
  private native int HPSHRIRFolddownDeleteInstance(long handle);
  private native void HPSHRIRFolddownLoadIRs(long handle, int presetID);
  private native int HPSHRIRFolddownProcessInPlaceInterleaved(long handle, float[] pBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
  private native int HPSHRIRFolddownProcessOutOfPlaceInterleaved(long handle, float[] pInBuf, float[] pOutBuf, int srcChannels, int dstChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);

  private native void HPSHeadtrackingGetVolumeMatrix(long handle, float[] outVolumeMatrix);
  private native long HPSHeadtrackingCreateInstance(int samplerate);
  private native int HPSHeadtrackingDeleteInstance(long handle);
  private native int HPSHeadtrackingProcessOutOfPlaceInterleaved(long handle, float azimuth, float[] pInBuf, float[] pOutBuf, int srcChannels, long totalsamples);

  private native void Hear360HPSGetVolumeMatrix(long handle, float[] outVolumeMatrix);
  private native long Hear360HPSCreateInstance(int samplerate, int presetID);
  private native int Hear360HPSDeleteInstance(long handle);
  private native int Hear360HPSProcessOutOfPlaceInterleaved(long handle, float azimuth, float[] pInBuf, float[] pOutBuf, int srcChannels, int dstChannels, boolean stereoUpMix51, long totalsamples);
  private native int Hear360HPSProcessInPlaceInterleaved(long handle, float azimuth, float[] pBuf, int srcChannels, boolean stereoUpMix51, long totalsamples);
}
