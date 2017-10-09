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

import com.google.android.exoplayer2.C;

/**
 * Opus decoder.
 */
/* package */ final class HPSAudioDSP {

  private final long nativeDSPContext;

  public HPSAudioDSP(int samplerate) {
    nativeDSPContext = HPS_HRIRFolddown_CreateInstance(samplerate);
  }

  public ~HPSAudioDSP() {
    if(m_CPPClassID != 0) {
      HPS_HRIRFolddown_DeleteInstance(m_CPPClassID);
    }
  }

  private native long HPS_HRIRFolddown_CreateInstance(int samplerate);
  private native int HPS_HRIRFolddown_DeleteInstance(long handle);
  private native void HPS_HRIRFolddown_LoadIRs(long handle, int presetID);
  private native int HPS_HRIRFolddown_ProcessInPlace(long handle, float[][] pBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
  private native int HPS_HRIRFolddown_ProcessInPlaceInterleaved(long handle, float[] pBuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
  private native int HPS_HRIRFolddown_ProcessOutOfPlace(long handle, float[][] pInBuf, float[][] pOutbuf, int dstChannels, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);
  private native int HPS_HRIRFolddown_ProcessOutOfPlaceInterleaved(long handle, float[] pInBuf, float[] pOutbuf, int srcChannels, long totalsamples, boolean hpsEnabled, boolean warmEQEnabled);

  private native long HPS_HRIRConvolutionCore_CreateInstance(int samplerate);
  private native int HPS_HRIRConvolutionCore_DeleteInstance(long handle);
  private native int HPS_HRIRConvolutionCore_ProcessOutOfPlace(long handle, float[] pInBuf, float[][] pOutbuf, long totalsamples);
  private native void HPS_HRIRConvolutionCore_LoadIR(long handle, long channelID);
  private native void HPS_HRIRConvolutionCore_LoadIRFromInts(long handle, int[] leftIRs, int[] rightIRs, int irFrames);
}
