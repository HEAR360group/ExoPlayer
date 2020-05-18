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
public final class EQAudioDSP {
  private final long dspContext;

  public EQAudioDSP(int samplerate) {
    dspContext = Hear360EQCreateInstance(samplerate);
  }

  public void release() {
    if(dspContext != 0) {
      Hear360EQDeleteInstance(dspContext);
    }
  }

  public int ProcessInPlaceInterleavedFloat(float[] pBuf, long totalsamples) {
    return Hear360EQProcessInPlaceInterleaved(dspContext, pBuf, totalsamples);
  }

  public int Update(float[] eqF, float[] eqG, float[] eqQ) {
    return Hear360EQUpdate(dspContext, eqF, eqG, eqQ);
  }

  private native long Hear360EQCreateInstance(int samplerate);
  private native int Hear360EQDeleteInstance(long handle);
  private native int Hear360EQProcessInPlaceInterleaved(long handle, float[] pBuf, long totalsamples);
  private native int Hear360EQUpdate(long handle, float[] eqF, float[] eqG, float[] eqQ);
}
