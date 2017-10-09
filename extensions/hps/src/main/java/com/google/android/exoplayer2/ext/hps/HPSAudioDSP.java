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

  private final long nativeDSPContext;
  private float[] inputBuf;
  private float[] outputBuf;

  public HPSAudioDSP(int samplerate) {
    nativeDSPContext = createInstance(samplerate);

    inputBuf = new float[30000];
    outputBuf = new float[30000];
  }

  public void release() {
    if(nativeDSPContext != 0) {
      deleteInstance(nativeDSPContext);
    }
  }

  public void LoadIRsJava(int presetID) {
    loadIRs(nativeDSPContext, presetID);
  }

  public int ProcessInPlaceInterleavedFloat(float[] pBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled) {
    processInPlaceInterleaved(nativeDSPContext, pBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);
    return 0;
  }

  public int ProcessOutOfPlaceInterleavedFloat(float[] pInBuf, float[] pOutBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled) {
    processOutOfPlaceInterleaved(nativeDSPContext, pInBuf, pOutBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);
    return 0;
  }

  public int ProcessInPlaceInterleavedShort(short[] pShortBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled) {
    for(int i = 0; i < pShortBuf.length; i++) {
      inputBuf[i] = (float)pShortBuf[i] / 32767.0f;
    }

    int result = 0;
    processInPlaceInterleaved(nativeDSPContext, inputBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);

    for(int i = 0; i < pShortBuf.length; i++) {
      inputBuf[i] = (float)pShortBuf[i] * 32767.0f;
    }

    return result;
  }

  public int ProcessOutOfPlaceInterleavedShort(short[] pInShortBuf, short[] pOutShortBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled) {
    for(int i = 0; i < pInShortBuf.length; i++) {
      inputBuf[i] = (float)pInShortBuf[i] / 32767.0f;
    }

    int result = 0;
    processOutOfPlaceInterleaved(nativeDSPContext, inputBuf, outputBuf, srcChannels, totalsamples, hpsEnabled, warmEQEnabled);

    for(int i = 0; i < pOutShortBuf.length; i++) {
      outputBuf[i] = (float)pOutShortBuf[i] * 32767.0f;
    }

    return result;
  }

  private native long createInstance(int samplerate);
  private native int deleteInstance(long handle);
  private native void loadIRs(long handle, int presetID);
  private native int processInPlaceInterleaved(long handle, float[] pBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
  private native int processOutOfPlaceInterleaved(long handle, float[] pInBuf, float[] pOutBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
}
