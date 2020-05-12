/*
 * Copyright (C) 2017 The Android Open Source Project
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
package com.google.android.exoplayer2.audio;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.ext.hps.HPSAudioDSP;
import com.google.android.exoplayer2.ext.hps.HPSLibrary;

import java.nio.ByteBuffer;

import androidx.annotation.Nullable;

/**
 * An {@link AudioProcessor} that applies a mapping from input channels onto specified output
 * channels. This can be used to reorder, duplicate or discard channels.
 */
@SuppressWarnings("nullness:initialization.fields.uninitialized")
/* package */ public final class HPSAudioProcessor extends BaseAudioProcessor {

  @Nullable private int[] pendingOutputChannels;
  @Nullable private int[] outputChannels;

  private int sampleRateHz;
  private int channelCount;
  private int earthChannelCount;
  private int skyChannelCount;

  private HPSLibrary hpsLibrary;
  private HPSAudioDSP hpsAudioDSPEarth;
  private HPSAudioDSP hpsAudioDSPSky;

  private int encoding;
  private ByteBuffer buffer;
  private ByteBuffer outputBuffer;
  private float[] inputBufEarth;
  private float[] inputBufSky;
  private float[] outputBufEarth;
  private float[] outputBufSky;

  //  private int fBufIndex = 0;
  private int fBufEarthIndex = 0;
  private int fBufSkyIndex = 0;
  private int curPage = 0;

  private static float soloToMixGainEffectOn = 1.0f;
  private static float allToMixGainEffectOn = 1.0f;
  private static float soloToMixGainEffectOff = 1.0f;
  private static float allToMixGainEffectOff = 1.0f;

  private static boolean[] channelsEnabled = new boolean[12];

//  public static final int FRONT_LEFT_CHANNEL_ID = 0;
//  public static final int FRONT_RIGHT_CHANNEL_ID = 1;
//  public static final int FRONT_CENTER_CHANNEL_ID = 2;
//  public static final int REAR_LEFT_CHANNEL_ID = 3;

  private static boolean isSoloOn = false;

  public static boolean gIsSonamiOn = false;

  public static void setSoloChannel(int index) {
    if(index == 4095) {
      isSoloOn = false;
    }
    else {
      isSoloOn = true;
    }

    for(int channelId = 0; channelId < 12; channelId++) {
      channelsEnabled[channelId] = false;
    }

    for(int channelId = 0; channelId < 12; channelId++) {
      if((index & (1 << channelId)) == (1 << channelId)) {
        channelsEnabled[channelId] = true;
      }
    }
  }

  public static void setGain(float gainHPSOn, float gainHPSOff) {
    soloToMixGainEffectOn = 25.0f * gainHPSOn / 3.0f;
    allToMixGainEffectOn = 12.5f * gainHPSOn / 3.0f;
    soloToMixGainEffectOff = 5.0f * gainHPSOff / 3.0f;
    allToMixGainEffectOff = 2.5f * gainHPSOff / 3.0f;
  }

  public HPSAudioProcessor() {
    boolean result = hpsLibrary.isAvailable();

    sampleRateHz = Format.NO_VALUE;
    channelCount = Format.NO_VALUE;
    encoding = C.ENCODING_INVALID;
    buffer = EMPTY_BUFFER;
    outputBuffer = EMPTY_BUFFER;

    inputBufEarth = new float[10000];
    inputBufSky = new float[10000];
    outputBufEarth = new float[10000];
    outputBufSky = new float[10000];

    //Reset global state
    gIsSonamiOn = false;
    for(int i = 0; i < channelsEnabled.length; i++) {
      channelsEnabled[i] = true;
    }
  }

  /**
   * Resets the channel mapping. After calling this method, call {@link #configure(AudioFormat)} to
   * start using the new channel map.
   *
   * @param outputChannels The mapping from input to output channel indices, or {@code null} to
   *     leave the input unchanged.
   * @see AudioSink#configure(int, int, int, int, int[], int, int)
   */
//  public void setChannelMap(@Nullable int[] outputChannels) {
//    pendingOutputChannels = outputChannels;
//  }

  @Override
  public AudioFormat onConfigure(AudioFormat inputAudioFormat)
          throws UnhandledAudioFormatException {
//    @Nullable int[] outputChannels = pendingOutputChannels;
//    if (outputChannels == null) {
//      return AudioFormat.NOT_SET;
//    }

    if (inputAudioFormat.encoding != C.ENCODING_PCM_16BIT) {
      throw new UnhandledAudioFormatException(inputAudioFormat);
    }
    if (this.sampleRateHz == inputAudioFormat.sampleRate && this.channelCount == inputAudioFormat.channelCount
            && this.encoding == inputAudioFormat.encoding) {
      return AudioFormat.NOT_SET;
    }

    this.sampleRateHz = inputAudioFormat.sampleRate;
    this.channelCount = inputAudioFormat.channelCount;
    this.earthChannelCount = channelCount <= 8 ? channelCount : 8;
    this.skyChannelCount = channelCount > 8 ? channelCount - 8 + 1 : 0;
    this.encoding = inputAudioFormat.encoding;

    boolean active = false;
    if(channelCount > 2) {
      active = true;
      hpsAudioDSPEarth = new HPSAudioDSP(sampleRateHz, 0);
      hpsAudioDSPSky = new HPSAudioDSP(sampleRateHz, 100);
    }

//    return AudioFormat.NOT_SET;

    return active
            ? new AudioFormat(inputAudioFormat.sampleRate, 2, C.ENCODING_PCM_16BIT)
            : AudioFormat.NOT_SET;
  }

  @Override
  public void queueInput(ByteBuffer inputBuffer) {
    // Prepare the output buffer.
    int position = inputBuffer.position();
    int limit = inputBuffer.limit();
    int frameCount = (limit - position) / inputAudioFormat.bytesPerFrame;
    int outputSize = frameCount * outputAudioFormat.bytesPerFrame;

    if (frameCount == 0) {
      return;
    }

    ByteBuffer buffer = replaceOutputBuffer(outputSize);

    int fBufIndex = 0;
//    int fBufEarthIndex = 0;
//    int fBufSkyIndex = 0;
    while (position < limit) {
      short input = inputBuffer.getShort(position);
      float inputFloat = (float)input / 32767.0f;
      int currentChannelIndex = fBufIndex % channelCount;
      if(currentChannelIndex < 8) {
        if(currentChannelIndex < 3) {
          if(channelsEnabled[currentChannelIndex]) {
            inputBufEarth[fBufEarthIndex++] = inputFloat;
          }
          else {
            inputBufEarth[fBufEarthIndex++] = 0;
          }
        }
        else if(currentChannelIndex == 3) {
          inputBufEarth[fBufEarthIndex++] = inputFloat;
        }
        else {
          if(channelsEnabled[currentChannelIndex - 1]) {
            inputBufEarth[fBufEarthIndex++] = inputFloat;
          }
          else {
            inputBufEarth[fBufEarthIndex++] = 0;
          }
        }
      }
      else {
        if(currentChannelIndex < 11) {
          if(channelsEnabled[currentChannelIndex - 1]) {
            inputBufSky[fBufSkyIndex++] = inputFloat;
          }
          else {
            inputBufSky[fBufSkyIndex++] = 0;
          }

          if(currentChannelIndex == 10) {
            inputBufSky[fBufSkyIndex++] = 0;
          }
        }
        else {
          if(channelsEnabled[currentChannelIndex - 1]) {
            inputBufSky[fBufSkyIndex++] = inputFloat;
          }
          else {
            inputBufSky[fBufSkyIndex++] = 0;
          }
        }
      }
      fBufIndex++;
      //(8byte per 8bits)16bit in total, multiple by 8 channels
      position += 2;
    }
//    curPage++;

//    if(curPage == 2) {
//
    if(gIsSonamiOn) {
      if (fBufEarthIndex != 0) {
        //hpsAudioDSPEarth.ProcessInPlaceInterleavedFloat(inputBufEarth, channelCount, frameCount, true, true);
        hpsAudioDSPEarth.ProcessOutOfPlaceInterleavedFloat(0, inputBufEarth, outputBufEarth, earthChannelCount, 2, false, frameCount);
      }

      if (fBufSkyIndex != 0) {
        hpsAudioDSPSky.ProcessOutOfPlaceInterleavedFloat(0, inputBufSky, outputBufSky, skyChannelCount, 2, false, frameCount);
      }
    }
    else {
      if (fBufEarthIndex != 0) {
        for (int i = 0; i < frameCount; i++) {
          //L&R
          outputBufEarth[i * 2] = inputBufEarth[i * earthChannelCount];
          outputBufEarth[i * 2 + 1] = inputBufEarth[i * earthChannelCount + 1];

          if (earthChannelCount >= 6) {
            //C
            outputBufEarth[i * 2] += inputBufEarth[i * earthChannelCount + 2] * 0.5f;
            outputBufEarth[i * 2 + 1] += inputBufEarth[i * earthChannelCount + 2] * 0.5f;
            //LFE
            outputBufEarth[i * 2] += inputBufEarth[i * earthChannelCount + 3] * 0.5f;
            outputBufEarth[i * 2 + 1] += inputBufEarth[i * earthChannelCount + 3] * 0.5f;
            //Ls&Rs
            outputBufEarth[i * 2] += inputBufEarth[i * earthChannelCount + 4];
            outputBufEarth[i * 2 + 1] += inputBufEarth[i * earthChannelCount + 5];
          }

          if (earthChannelCount >= 8) {
            //Lss&Rss
            outputBufEarth[i * 2] += inputBufEarth[i * earthChannelCount + 6];
            outputBufEarth[i * 2 + 1] += inputBufEarth[i * earthChannelCount + 7];
          }
        }
      }

      if (fBufSkyIndex != 0) {
        for (int i = 0; i < frameCount; i++) {
          //LH&RH
          outputBufSky[i * 2] = inputBufSky[i * skyChannelCount];
          outputBufSky[i * 2 + 1] = inputBufSky[i * skyChannelCount + 1];
          //LsH&RsH
          outputBufSky[i * 2] += inputBufSky[i * skyChannelCount + 2];
          outputBufSky[i * 2 + 1] += inputBufSky[i * skyChannelCount + 4];
          //Top
          outputBufSky[i * 2] += inputBufSky[i * skyChannelCount + 5] * 0.5f;
          outputBufSky[i * 2 + 1] += inputBufSky[i * skyChannelCount + 5] * 0.5f;
        }
      }
    }
//      curPage = 0;
//      fBufIndex = 0;
      fBufEarthIndex = 0;
      fBufSkyIndex = 0;
//    }
//    else {
//      //Output buffers are stereo (2 channels)
//      for(int i = 0; i < frameCount * 2; i++) {
//        outputBufEarth[i] = outputBufEarth[i + frameCount * 2];
//      }
//
//      for(int i = 0; i < frameCount * 2; i++) {
//        outputBufSky[i] = outputBufSky[i + frameCount * 2];
//      }
//    }

    float gain = (gIsSonamiOn ? (isSoloOn ? soloToMixGainEffectOn: allToMixGainEffectOn) : (isSoloOn ? soloToMixGainEffectOff : allToMixGainEffectOff)) / channelCount;

    for(int i = 0; i < frameCount; i++) {
      float l = (outputBufEarth[i * 2] + outputBufSky[i * 2]) * gain;
      float r = (outputBufEarth[i * 2 + 1] + outputBufSky[i * 2 + 1]) * gain;

      short inputFrontL = (short)(l * 32767.0f);
      short inputFrontR = (short)(r * 32767.0f);

      //Write the mixed stereo to the first 2 channels as the output
      buffer.putShort(inputFrontL);
      buffer.putShort(inputFrontR);
    }
//
//    while (position < limit) {
//      short input = inputBuffer.getShort(position);
//      buffer.putShort(input);
//      position += 2;
//    }
    inputBuffer.position(limit);
    buffer.flip();
  }

//  @Override
//  protected void onFlush() {
//    outputChannels = pendingOutputChannels;
//  }
//
//  @Override
//  protected void onReset() {
//    outputChannels = null;
//    pendingOutputChannels = null;
//  }

}
