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
import com.google.android.exoplayer2.ext.hps.EQAudioDSP;
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
  private EQAudioDSP eq;

  private int encoding;
  private ByteBuffer buffer;
  private ByteBuffer outputBuffer;
  private float[] inputBufEarth;
  private float[] inputBufSky;
  private float[] outputBufEarth;
  private float[] outputBufSky;
  private float[] masterBuf;

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

  public static void setGain(float gainHPSOn, float gainHPSOff, int hex) {
    int channelCount = ChannelCountFromHex(hex) + 1;
    soloToMixGainEffectOn = 25.0f / channelCount * gainHPSOn;
    allToMixGainEffectOn = 12.5f / channelCount * gainHPSOn;
    soloToMixGainEffectOff = 5.0f / channelCount * gainHPSOff;
    allToMixGainEffectOff = 2.5f / channelCount * gainHPSOff;
    return;
  }

  private static float kTransistingStep = 1.0f / 50;
  private static float curFadeVolume = 0.0f;
  private static VolumeRamperState curFadeState = VolumeRamperState.Idle;

  public static enum VolumeRamperType {
    Silent,
    FadeIn,
    FadeOutThenIn
  }

  public static enum VolumeRamperState {
    Idle,
    FadingIn,
    FadingOutThenIn
  }

  public static void fade(VolumeRamperType type) {
    switch(type) {
      case FadeIn:
        curFadeState = VolumeRamperState.FadingIn;
        break;
      case Silent:
        curFadeVolume = 0.0f;
        break;
      case FadeOutThenIn:
        curFadeState = VolumeRamperState.FadingOutThenIn;
        break;
      default:
        break;
    }
  }

  //Greg's measurements
  private static final float mEQQ[] = {.615f, .5996f, .613f, .645f, .604f, .619f, .559f, .664f, .592f, .348f, 0.9f, 0.9f};
  private static final float mEQF[] = {31.0f, 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f, 16000.0f, 16000.0f};
  private static float mEQG[] = new float[12];

  private static boolean eqChanged = false;

  public static void setEQ(float[] eqG) {
    for(int i = 0; i < 10; i++) {
      mEQG[i] = eqG[i];
    }
    mEQG[10] = 0.0f;
    mEQG[11] = 0.0f;

    eqChanged = true;
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
    masterBuf = new float[10000];

    //Reset global state
    gIsSonamiOn = false;
    for(int i = 0; i < channelsEnabled.length; i++) {
      channelsEnabled[i] = true;
    }
//    curFadeVolume = 0.0f;
//    curFadeState = VolumeRamperState.Idle;
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
      eq = new EQAudioDSP(sampleRateHz);
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
    int fBufEarthIndex = 0;
    int fBufSkyIndex = 0;
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
//      fBufEarthIndex = 0;
//      fBufSkyIndex = 0;
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

    float gain = (gIsSonamiOn ? (isSoloOn ? soloToMixGainEffectOn: allToMixGainEffectOn) : (isSoloOn ? soloToMixGainEffectOff : allToMixGainEffectOff)) / 2.0f;

    switch (curFadeState) {
      case FadingIn:
        curFadeVolume += kTransistingStep;
        if(curFadeVolume >= 1.0f) {
          curFadeVolume = 1.0f;
          curFadeState = VolumeRamperState.Idle;
        }
        break;
      case FadingOutThenIn:
        curFadeVolume -= kTransistingStep;
        if(curFadeVolume <= 0.0f) {
          curFadeVolume = 0.0f;
          curFadeState = VolumeRamperState.FadingIn;
        }
        break;
      default:
        break;
    }

    for(int i = 0; i < frameCount * 2; i++) {
      masterBuf[i] = outputBufEarth[i] + outputBufSky[i];
    }

    if(gIsSonamiOn) {
      if(eqChanged) {
        eq.Update(mEQF, mEQG, mEQQ);
        eqChanged = false;
      }

      eq.ProcessInPlaceInterleavedFloat(masterBuf, frameCount);
    }

    for(int i = 0; i < frameCount; i++) {
      float l = masterBuf[i * 2] * gain * curFadeVolume;
      float r = masterBuf[i * 2 + 1] * gain * curFadeVolume;

      float fl = (l < -0.99f) ? -0.99f : ((l > 0.99f) ? 0.99f : l);
      float fr = (r < -0.99f) ? -0.99f : ((r > 0.99f) ? 0.99f : r);

      short inputFrontL = (short)(fl * 32767.0f);
      short inputFrontR = (short)(fr * 32767.0f);

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

  public static int ChannelCountFromHex(int hex) {
    int result = 0;
    int flag = 0x1;

    for(int i = 0; i < 32; i++) {
      if((flag & hex) != 0) {
        result++;
      }
      flag = flag << 1;
    }
    return result;
  }
}
