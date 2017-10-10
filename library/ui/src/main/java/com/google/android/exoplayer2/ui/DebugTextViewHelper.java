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
package com.google.android.exoplayer2.ui;

import android.annotation.SuppressLint;
import android.widget.TextView;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.decoder.DecoderCounters;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;

import java.text.DecimalFormat;
import java.util.Locale;

import javax.vecmath.Vector3d;

/**
 * A helper class for periodically updating a {@link TextView} with debug information obtained from
 * a {@link SimpleExoPlayer}.
 */
public final class DebugTextViewHelper implements Runnable, Player.EventListener {

  private static final int REFRESH_INTERVAL_MS = 100;

  private final SimpleExoPlayer player;
  private final TextView textView;

  private boolean started;

  private final int MAX_CHANNEL_COUNT = 8;
  private final int DEFAULT_CHANNEL_COUNT = 8;
  private final int LFE_CHANNEL_ID = 3;
  private final boolean HAS_LFE = true;
  private final Vector3d FRONT_VEC = new Vector3d(0, 0, 1);

  public class DegreeDiff {
    public double dotValue;
    public double crossValue;
    public double angleValue;
    public int speakerIndex;
  }


  private double azimuth;
  private double[][] volumeMatrix = new double[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];

  private static DecimalFormat df2 = new DecimalFormat("0.00");

  /**
   * @param player The {@link SimpleExoPlayer} from which debug information should be obtained.
   * @param textView The {@link TextView} that should be updated to display the information.
   */
  public DebugTextViewHelper(SimpleExoPlayer player, TextView textView) {
    this.player = player;
    this.textView = textView;
  }

  /**
   * Starts periodic updates of the {@link TextView}. Must be called from the application's main
   * thread.
   */
  public void start() {
    if (started) {
      return;
    }
    started = true;
    player.addListener(this);
    updateAndPost();
  }

  /**
   * Stops periodic updates of the {@link TextView}. Must be called from the application's main
   * thread.
   */
  public void stop() {
    if (!started) {
      return;
    }
    started = false;
    player.removeListener(this);
    textView.removeCallbacks(this);
  }

  // Player.EventListener implementation.

  @Override
  public void onLoadingChanged(boolean isLoading) {
    // Do nothing.
  }

  @Override
  public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
    updateAndPost();
  }

  @Override
  public void onRepeatModeChanged(int repeatMode) {
    // Do nothing.
  }

  @Override
  public void onPositionDiscontinuity() {
    updateAndPost();
  }

  @Override
  public void onPlaybackParametersChanged(PlaybackParameters playbackParameters) {
    // Do nothing.
  }

  @Override
  public void onTimelineChanged(Timeline timeline, Object manifest) {
    // Do nothing.
  }

  @Override
  public void onPlayerError(ExoPlaybackException error) {
    // Do nothing.
  }

  @Override
  public void onTracksChanged(TrackGroupArray tracks, TrackSelectionArray selections) {
    // Do nothing.
  }

  // Runnable implementation.

  @Override
  public void run() {
    updateAndPost();
  }

  // Private methods.

  @SuppressLint("SetTextI18n")
  private void updateAndPost() {
    textView.setText(getPlayerStateString() + getPlayerWindowIndexString() + getVideoString()
        + getAudioString() + getSpatialAudioString() + getVolumeMatrix());
    textView.removeCallbacks(this);
    textView.postDelayed(this, REFRESH_INTERVAL_MS);
  }

  private String getPlayerStateString() {
    String text = "playWhenReady:" + player.getPlayWhenReady() + " playbackState:";
    switch (player.getPlaybackState()) {
      case Player.STATE_BUFFERING:
        text += "buffering";
        break;
      case Player.STATE_ENDED:
        text += "ended";
        break;
      case Player.STATE_IDLE:
        text += "idle";
        break;
      case Player.STATE_READY:
        text += "ready";
        break;
      default:
        text += "unknown";
        break;
    }
    return text;
  }

  private String getPlayerWindowIndexString() {
    return " window:" + player.getCurrentWindowIndex();
  }

  private String getVideoString() {
    Format format = player.getVideoFormat();
    if (format == null) {
      return "";
    }
    return "\n" + format.sampleMimeType + "(id:" + format.id + " r:" + format.width + "x"
        + format.height + getPixelAspectRatioString(format.pixelWidthHeightRatio)
        + getDecoderCountersBufferCountString(player.getVideoDecoderCounters()) + ")";
  }

  private String getAudioString() {
    Format format = player.getAudioFormat();
    if (format == null) {
      return "";
    }
    return "\n" + format.sampleMimeType + "(id:" + format.id + " hz:" + format.sampleRate + " ch:"
        + format.channelCount
        + getDecoderCountersBufferCountString(player.getAudioDecoderCounters()) + ")";
  }

  private String getSpatialAudioString() {
    return "\nAzimuth:" + df2.format(Math.toDegrees(-azimuth));
  }

  private String getVolumeMatrix() {
    return "\nL:" + getVolumeMatrixChannel(0) + "\nR:" + getVolumeMatrixChannel(1) +
            "\nC:" + getVolumeMatrixChannel(2) + "\nLFE: " + getVolumeMatrixChannel(3) +
            "\nLSR: " + getVolumeMatrixChannel(4) + "\nRSR: " + getVolumeMatrixChannel(5) +
            "\nLSS: " + getVolumeMatrixChannel(6) + "\nRSS: " + getVolumeMatrixChannel(7);
  }

  private String getVolumeMatrixChannel(int i) {
    String msg = "";
    updateVolumeMatrix(volumeMatrix);
    for(int j = 0; j < volumeMatrix[i].length; j++) {
      msg += df2.format(volumeMatrix[i][j]) + ", ";
    }
    return msg.substring(0, msg.length() - 2);
  }

  public void setAzimuth(double azimuth) {
    this.azimuth = -azimuth;
  }
/*
  public static void setVolumeMatrix(double[][] matrix) {
    //volumeMatrix = matrix;
    for(int i = 0; i < matrix.length; i++) {
      for(int j = 0; j < matrix[i].length; i++) {
        volumeMatrix[i][j] = matrix[i][j];
      }
    }
  }
*/
  private void updateVolumeMatrix(double[][] volumeMatrix) {
    double[] speakerPos = new double[MAX_CHANNEL_COUNT];
    Vector3d[] speakerVec = new Vector3d[MAX_CHANNEL_COUNT];
    Vector3d[] rotatedSpeakerVec = new Vector3d[MAX_CHANNEL_COUNT];

    Vector3d rotatedFrontVec = rotate(azimuth, FRONT_VEC);
/*
    speakerPos[0] = Math.toRadians(-30);
    speakerPos[1] = Math.toRadians(30);
    speakerPos[2] = Math.toRadians(0);
    speakerPos[3] = Math.toRadians(0);
    speakerPos[4] = Math.toRadians(-120);
    speakerPos[5] = Math.toRadians(120);
*/

        speakerPos[0] = Math.toRadians(-45);
        speakerPos[1] = Math.toRadians(45);
        speakerPos[2] = Math.toRadians(0);
        speakerPos[3] = Math.toRadians(0);
        speakerPos[4] = Math.toRadians(-135);
        speakerPos[5] = Math.toRadians(135);
        speakerPos[6] = Math.toRadians(-90);
        speakerPos[7] = Math.toRadians(90);

    for (int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
      speakerVec[i] = rotate(speakerPos[i], FRONT_VEC);
    }

    for (int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
      if (HAS_LFE && i == LFE_CHANNEL_ID)
        continue;

      rotatedSpeakerVec[i] = rotate(speakerPos[i], rotatedFrontVec);
    }


    for (int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
      if (HAS_LFE && i == LFE_CHANNEL_ID)
        continue;

      DegreeDiff[] degreeDiffArrayL = new DegreeDiff[MAX_CHANNEL_COUNT];
      DegreeDiff[] degreeDiffArrayR = new DegreeDiff[MAX_CHANNEL_COUNT];
      int degreeDiffArrayCountL = 0;
      int degreeDiffArrayCountR = 0;

      for (int speakerPosID = 0; speakerPosID < DEFAULT_CHANNEL_COUNT; speakerPosID++) {
        if (HAS_LFE && speakerPosID == LFE_CHANNEL_ID)
          continue;

        //double dotValue = rotatedSpeakerVec[i].dot(speakerVec[speakerPosID]);
        Vector3d crossVec = new Vector3d();
        crossVec.cross(rotatedSpeakerVec[i], speakerVec[speakerPosID]);
        double angleValue = rotatedSpeakerVec[i].angle(speakerVec[speakerPosID]);
        //if(dotValue <= 0)
        //continue;

        DegreeDiff degreeDiff = new DegreeDiff();
        //degreeDiff.dotValue = dotValue;
        degreeDiff.crossValue = crossVec.y;
        degreeDiff.angleValue = angleValue;
        degreeDiff.speakerIndex = speakerPosID;

        if (degreeDiff.angleValue == 0 /*|| Math.abs(degreeDiff.dotValue) < 0.001*/) {
          degreeDiffArrayCountL = 0;
          degreeDiffArrayCountR = 0;
          break;
        }
        //Select the nearest speaker from left
        else if (degreeDiff.crossValue < 0) {
          orderInsert(degreeDiffArrayL, 0, degreeDiffArrayCountL, degreeDiff);
          degreeDiffArrayCountL++;
        }
        //Select the nearest speaker from right
        else {
          orderInsert(degreeDiffArrayR, 0, degreeDiffArrayCountR, degreeDiff);
          degreeDiffArrayCountR++;
        }
      }

      if (degreeDiffArrayCountL != 0 && degreeDiffArrayCountR != 0) {
        //Calculate volume distribution
        int speakerIndex0 = degreeDiffArrayL[0].speakerIndex;
        double angle0 = degreeDiffArrayL[0].angleValue;
        //double dot0 = degreeDiffArrayL[0].dotValue;
        int speakerIndex1 = degreeDiffArrayR[0].speakerIndex;
        double angle1 = degreeDiffArrayR[0].angleValue;
        //double dot1 = degreeDiffArrayR[0].dotValue;

        //double actAngle0 = Math.sin(angle0);
        //double actAngle1 = Math.cos(angle1);
        double speaker0Vol = angle1 / (angle0 + angle1);
        double speaker1Vol = angle0 / (angle0 + angle1);
        //double speaker0ActVol = Math.sin(speaker0Vol * Math.PI / 2);
        //double speaker1ActVol = Math.cos(speaker0Vol * Math.PI / 2);

        //double speaker0Vol = dot0 / (dot0 + dot1);
        //double speaker1Vol = dot1 / (dot0 + dot1);

        volumeMatrix[i][speakerIndex0] = speaker0Vol;
        volumeMatrix[i][speakerIndex1] = speaker1Vol;

        for (int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
          if (j == speakerIndex0 || j == speakerIndex1)
            continue;

          if (HAS_LFE && j == LFE_CHANNEL_ID)
            continue;

          volumeMatrix[i][j] = 0;
        }
      } else {
        volumeMatrix[i][i] = 1;

        for (int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
          if (j == i)
            continue;

          if (HAS_LFE && j == LFE_CHANNEL_ID)
            continue;

          volumeMatrix[i][j] = 0;
        }
      }
    }

    if(HAS_LFE)
      volumeMatrix[LFE_CHANNEL_ID][LFE_CHANNEL_ID] = 1;
  }

  private Vector3d rotate(double theta, Vector3d in) {
    double cosTheta = Math.cos(theta);
    double sinTheta = Math.sin(theta);
    Vector3d out = new Vector3d();
    out.x = in.x * cosTheta + in.z * sinTheta;
    out.y = in.y;
    out.z = -in.x * sinTheta + in.z * cosTheta;
    out.normalize();
    return out;
  }

  private int orderInsert(DegreeDiff[] arr, int first, int last, DegreeDiff target) {
    int i = last;
    while((i > first) && (target.angleValue < arr[i - 1].angleValue)) {
      //while((i > first) && (target.dotValue > arr[i - 1].dotValue)) {
      arr[i] = arr[i - 1];
      i = i - 1;
    }
    arr[i] = target;
    return i;
  }

  private static String getDecoderCountersBufferCountString(DecoderCounters counters) {
    if (counters == null) {
      return "";
    }
    counters.ensureUpdated();
    return " rb:" + counters.renderedOutputBufferCount
        + " sb:" + counters.skippedOutputBufferCount
        + " db:" + counters.droppedOutputBufferCount
        + " mcdb:" + counters.maxConsecutiveDroppedOutputBufferCount;
  }

  private static String getPixelAspectRatioString(float pixelAspectRatio) {
    return pixelAspectRatio == Format.NO_VALUE || pixelAspectRatio == 1f ? ""
        : (" par:" + String.format(Locale.US, "%.02f", pixelAspectRatio));
  }

}
