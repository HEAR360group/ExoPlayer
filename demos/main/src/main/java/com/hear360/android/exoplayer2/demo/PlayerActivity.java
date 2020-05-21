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
package com.hear360.android.exoplayer2.demo;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.MediaDrm;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.util.Pair;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.constraintlayout.widget.Group;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.C.ContentType;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.PlaybackPreparer;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.RenderersFactory;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.audio.AudioAttributes;
import com.google.android.exoplayer2.audio.HPSAudioProcessor;
import com.hear360.android.exoplayer2.demo.Sample.UriSample;
import com.google.android.exoplayer2.drm.DefaultDrmSessionManager;
import com.google.android.exoplayer2.drm.DrmSessionManager;
import com.google.android.exoplayer2.drm.ExoMediaCrypto;
import com.google.android.exoplayer2.drm.FrameworkMediaDrm;
import com.google.android.exoplayer2.drm.HttpMediaDrmCallback;
import com.google.android.exoplayer2.drm.MediaDrmCallback;
import com.google.android.exoplayer2.mediacodec.MediaCodecRenderer.DecoderInitializationException;
import com.google.android.exoplayer2.mediacodec.MediaCodecUtil.DecoderQueryException;
import com.google.android.exoplayer2.offline.DownloadHelper;
import com.google.android.exoplayer2.offline.DownloadRequest;
import com.google.android.exoplayer2.source.BehindLiveWindowException;
import com.google.android.exoplayer2.source.ConcatenatingMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.MediaSourceFactory;
import com.google.android.exoplayer2.source.MergingMediaSource;
import com.google.android.exoplayer2.source.ProgressiveMediaSource;
import com.google.android.exoplayer2.source.SingleSampleMediaSource;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.source.ads.AdsLoader;
import com.google.android.exoplayer2.source.ads.AdsMediaSource;
import com.google.android.exoplayer2.source.dash.DashMediaSource;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.source.smoothstreaming.SsMediaSource;
import com.google.android.exoplayer2.trackselection.AdaptiveTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.MappingTrackSelector.MappedTrackInfo;
import com.google.android.exoplayer2.trackselection.RandomTrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;
import com.google.android.exoplayer2.ui.DebugTextViewHelper;
import com.google.android.exoplayer2.ui.PlayerControlView;
import com.google.android.exoplayer2.ui.PlayerView;
import com.google.android.exoplayer2.ui.spherical.SphericalGLSurfaceView;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.HttpDataSource;
import com.google.android.exoplayer2.util.ErrorMessageProvider;
import com.google.android.exoplayer2.util.EventLogger;
import com.google.android.exoplayer2.util.Util;

import org.w3c.dom.Text;

import java.lang.reflect.Constructor;
import java.net.CookieHandler;
import java.net.CookieManager;
import java.net.CookiePolicy;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/** An activity that plays media using {@link SimpleExoPlayer}. */
public class PlayerActivity extends AppCompatActivity
        implements OnClickListener, CompoundButton.OnCheckedChangeListener, SeekBar.OnSeekBarChangeListener, PlaybackPreparer, PlayerControlView.VisibilityListener, AdapterView.OnItemSelectedListener {

  // Activity extras.

  public static final String SPHERICAL_STEREO_MODE_EXTRA = "spherical_stereo_mode";
  public static final String SPHERICAL_STEREO_MODE_MONO = "mono";
  public static final String SPHERICAL_STEREO_MODE_TOP_BOTTOM = "top_bottom";
  public static final String SPHERICAL_STEREO_MODE_LEFT_RIGHT = "left_right";

  // Actions.

  public static final String ACTION_VIEW = "com.google.android.exoplayer.demo.action.VIEW";
  public static final String ACTION_VIEW_LIST =
      "com.google.android.exoplayer.demo.action.VIEW_LIST";

  // Player configuration extras.

  public static final String ABR_ALGORITHM_EXTRA = "abr_algorithm";
  public static final String ABR_ALGORITHM_DEFAULT = "default";
  public static final String ABR_ALGORITHM_RANDOM = "random";

  // SONAMI sample related extras.

  public static final String CHANNELS_MASK_EXTRA = "channels_mask";
  public static final String GAIN_HPS_ON = "gain_hps_on";
  public static final String GAIN_HPS_OFF = "gain_hps_off";

  // Media item configuration extras.

  public static final String URI_EXTRA = "uri";
  public static final String EXTENSION_EXTRA = "extension";
  public static final String IS_LIVE_EXTRA = "is_live";

  public static final String DRM_SCHEME_EXTRA = "drm_scheme";
  public static final String DRM_LICENSE_URL_EXTRA = "drm_license_url";
  public static final String DRM_KEY_REQUEST_PROPERTIES_EXTRA = "drm_key_request_properties";
  public static final String DRM_MULTI_SESSION_EXTRA = "drm_multi_session";
  public static final String PREFER_EXTENSION_DECODERS_EXTRA = "prefer_extension_decoders";
  public static final String TUNNELING_EXTRA = "tunneling";
  public static final String AD_TAG_URI_EXTRA = "ad_tag_uri";
  public static final String SUBTITLE_URI_EXTRA = "subtitle_uri";
  public static final String SUBTITLE_MIME_TYPE_EXTRA = "subtitle_mime_type";
  public static final String SUBTITLE_LANGUAGE_EXTRA = "subtitle_language";
  // For backwards compatibility only.
  public static final String DRM_SCHEME_UUID_EXTRA = "drm_scheme_uuid";

  // Saved instance state keys.

  private static final String KEY_TRACK_SELECTOR_PARAMETERS = "track_selector_parameters";
  private static final String KEY_WINDOW = "window";
  private static final String KEY_POSITION = "position";
  private static final String KEY_AUTO_PLAY = "auto_play";

  private static final CookieManager DEFAULT_COOKIE_MANAGER;
  static {
    DEFAULT_COOKIE_MANAGER = new CookieManager();
    DEFAULT_COOKIE_MANAGER.setCookiePolicy(CookiePolicy.ACCEPT_ORIGINAL_SERVER);
  }

  private PlayerView playerView;
  private LinearLayout debugRootView;
  private Button selectTracksButton;
  private TextView debugTextView;
  private boolean isShowingTrackSelectionDialog;

  private Switch swSonami;
  private Switch swChannels;
  private Switch swEQ;
  private VerticalSeekBar[] eqSliders;
  private TextView[] lblEQs;
  private RadioButton[] rbChannels;
  private RadioButton rbAllChannel;
//  private Group layoutChannels;
  private Group layoutEQ;
  private Spinner lstEQPicker;
  private Button btnSaveEQ;

  private DataSource.Factory dataSourceFactory;
  private SimpleExoPlayer player;
  private MediaSource mediaSource;
  private DefaultTrackSelector trackSelector;
  private DefaultTrackSelector.Parameters trackSelectorParameters;
  private DebugTextViewHelper debugViewHelper;
  private TrackGroupArray lastSeenTrackGroupArray;

  private boolean startAutoPlay;
  private int startWindow;
  private long startPosition;

  // Fields used only for ad playback. The ads loader is loaded via reflection.

  private AdsLoader adsLoader;
  private Uri loadedAdTagUri;

  //SONAMI related variables
  private int channelsMask;
  private double gainHPSOn;
  private double gainHPSOff;
  private float[] eqBands;
  HashMap<String, float[]> allEQs;
  List<String> eqNames;

  // Activity lifecycle

  @Override
  public void onCreate(Bundle savedInstanceState) {
//    View decorView = getWindow().getDecorView();
//    int uiOptions = View.SYSTEM_UI_FLAG_IMMERSIVE;
//    decorView.setSystemUiVisibility(uiOptions);


    Intent intent = getIntent();
//    trackSelector.getCurrentMappedTrackInfo().getRendererCount();

    String sphericalStereoMode = intent.getStringExtra(SPHERICAL_STEREO_MODE_EXTRA);
    if (sphericalStereoMode != null) {
      setTheme(R.style.PlayerTheme_Spherical);
    }
    super.onCreate(savedInstanceState);
    dataSourceFactory = buildDataSourceFactory();
    if (CookieHandler.getDefault() != DEFAULT_COOKIE_MANAGER) {
      CookieHandler.setDefault(DEFAULT_COOKIE_MANAGER);
    }

    setContentView(R.layout.player_activity);
    debugRootView = findViewById(R.id.controls_root);
    debugTextView = findViewById(R.id.debug_text_view);
    selectTracksButton = findViewById(R.id.select_tracks_button);
    selectTracksButton.setOnClickListener(this);

    swSonami = findViewById(R.id.sw_sonami);
    swSonami.setOnCheckedChangeListener(this);
    swChannels = findViewById(R.id.sw_channels);
    swChannels.setOnCheckedChangeListener(this);
    swEQ = findViewById(R.id.sw_eq);
    swEQ.setOnCheckedChangeListener(this);

    channelsMask = intent.getIntExtra(CHANNELS_MASK_EXTRA, 0);

//    layoutChannels = findViewById(R.id.channels_control_group);
    layoutEQ = findViewById(R.id.eq_control_group);

    rbChannels = new RadioButton[12];
    rbChannels[0] = findViewById(R.id.rbLChannel);
    rbChannels[1] = findViewById(R.id.rbRChannel);
    rbChannels[2] = findViewById(R.id.rbCChannel);
    rbChannels[3] = findViewById(R.id.rbLsChannel);
    rbChannels[4] = findViewById(R.id.rbRsChannel);
    rbChannels[5] = findViewById(R.id.rbLssChannel);
    rbChannels[6] = findViewById(R.id.rbRssChannel);
    rbChannels[7] = findViewById(R.id.rbLhChannel);
    rbChannels[8] = findViewById(R.id.rbRhChannel);
    rbChannels[9] = findViewById(R.id.rbLshChannel);
    rbChannels[10] = findViewById(R.id.rbRshChannel);
    rbChannels[11] = findViewById(R.id.rbTChannel);
    for(int i = 0; i < rbChannels.length; i++) {
      rbChannels[i].setOnClickListener(this);
    }

    rbAllChannel = findViewById(R.id.rbAllChannel);
    rbAllChannel.setOnClickListener(this);

    hideAllChannelButtons();

    eqSliders = new VerticalSeekBar[10];
    eqSliders[0] = findViewById(R.id.sldEQBand0);
    eqSliders[1] = findViewById(R.id.sldEQBand1);
    eqSliders[2] = findViewById(R.id.sldEQBand2);
    eqSliders[3] = findViewById(R.id.sldEQBand3);
    eqSliders[4] = findViewById(R.id.sldEQBand4);
    eqSliders[5] = findViewById(R.id.sldEQBand5);
    eqSliders[6] = findViewById(R.id.sldEQBand6);
    eqSliders[7] = findViewById(R.id.sldEQBand7);
    eqSliders[8] = findViewById(R.id.sldEQBand8);
    eqSliders[9] = findViewById(R.id.sldEQBand9);
    for(int i = 0; i < eqSliders.length; i++) {
      eqSliders[i].setOnSeekBarChangeListener(this);
    }

    lblEQs = new TextView[10];
    lblEQs[0] = findViewById(R.id.lblEQBand0);
    lblEQs[1] = findViewById(R.id.lblEQBand1);
    lblEQs[2] = findViewById(R.id.lblEQBand2);
    lblEQs[3] = findViewById(R.id.lblEQBand3);
    lblEQs[4] = findViewById(R.id.lblEQBand4);
    lblEQs[5] = findViewById(R.id.lblEQBand5);
    lblEQs[6] = findViewById(R.id.lblEQBand6);
    lblEQs[7] = findViewById(R.id.lblEQBand7);
    lblEQs[8] = findViewById(R.id.lblEQBand8);
    lblEQs[9] = findViewById(R.id.lblEQBand9);

    lstEQPicker = findViewById(R.id.lstEQPicker);
    btnSaveEQ = findViewById(R.id.btnSaveEQ);
    btnSaveEQ.setOnClickListener(this);

    eqBands = new float[10];
    allEQs = new HashMap<>();
    eqNames = new ArrayList<String>();

    playerView = findViewById(R.id.player_view);
    playerView.setControllerVisibilityListener(this);
    playerView.setErrorMessageProvider(new PlayerErrorMessageProvider());
    playerView.requestFocus();

    if (sphericalStereoMode != null) {
      int stereoMode;
      if (SPHERICAL_STEREO_MODE_MONO.equals(sphericalStereoMode)) {
        stereoMode = C.STEREO_MODE_MONO;
      } else if (SPHERICAL_STEREO_MODE_TOP_BOTTOM.equals(sphericalStereoMode)) {
        stereoMode = C.STEREO_MODE_TOP_BOTTOM;
      } else if (SPHERICAL_STEREO_MODE_LEFT_RIGHT.equals(sphericalStereoMode)) {
        stereoMode = C.STEREO_MODE_LEFT_RIGHT;
      } else {
        showToast(R.string.error_unrecognized_stereo_mode);
        finish();
        return;
      }
      ((SphericalGLSurfaceView) playerView.getVideoSurfaceView()).setDefaultStereoMode(stereoMode);
    }

    if (savedInstanceState != null) {
      trackSelectorParameters = savedInstanceState.getParcelable(KEY_TRACK_SELECTOR_PARAMETERS);
      startAutoPlay = savedInstanceState.getBoolean(KEY_AUTO_PLAY);
      startWindow = savedInstanceState.getInt(KEY_WINDOW);
      startPosition = savedInstanceState.getLong(KEY_POSITION);
    } else {
      DefaultTrackSelector.ParametersBuilder builder =
          new DefaultTrackSelector.ParametersBuilder(/* context= */ this);
      boolean tunneling = intent.getBooleanExtra(TUNNELING_EXTRA, false);
      if (Util.SDK_INT >= 21 && tunneling) {
        builder.setTunnelingAudioSessionId(C.generateAudioSessionIdV21(/* context= */ this));
      }
      trackSelectorParameters = builder.build();
      clearStartPosition();
    }

    gainHPSOn = intent.getDoubleExtra(GAIN_HPS_ON, 1.0);
    gainHPSOff = intent.getDoubleExtra(GAIN_HPS_OFF, 1.0);

    HPSAudioProcessor.setGain((float)gainHPSOn, (float)gainHPSOff, channelsMask);
    HPSAudioProcessor.fade(HPSAudioProcessor.VolumeRamperType.Silent);
    HPSAudioProcessor.fade(HPSAudioProcessor.VolumeRamperType.FadeIn);

    reloadAllBandEQs();

    lstEQPicker.setOnItemSelectedListener(this);

    int index = eqNames.indexOf("Flat");
    lstEQPicker.setSelection(index);
  }

  public void showAllChannelButtons() {
    for(int i = 0; i < rbChannels.length; i++) {
      if((channelsMask & (1 << i)) != 0) {
        rbChannels[i].setVisibility(View.VISIBLE);
      }
      else {
        rbChannels[i].setVisibility(View.GONE);
      }
    }
    rbAllChannel.setVisibility(View.VISIBLE);
  }

  public void hideAllChannelButtons() {
    for(int i = 0; i < rbChannels.length; i++) {
      rbChannels[i].setVisibility(View.GONE);
    }
    rbAllChannel.setVisibility(View.GONE);
  }

  public void reloadAllBandEQs() {
    HashMap<String, float[]> dict = AppSettingsPreference.loadEQParameter(this);
    allEQs.clear();
    allEQs.putAll(dict);

    eqNames.clear();
    eqNames.addAll(allEQs.keySet());
    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, eqNames);
    dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    lstEQPicker.setAdapter(dataAdapter);
  }

  @Override
  public void onNewIntent(Intent intent) {
    super.onNewIntent(intent);
    releasePlayer();
    releaseAdsLoader();
    clearStartPosition();
    setIntent(intent);
  }

  @Override
  public void onStart() {
    super.onStart();
    if (Util.SDK_INT > 23) {
      initializePlayer();
      if (playerView != null) {
        playerView.onResume();
      }
    }
  }

  @Override
  public void onResume() {
    super.onResume();
    if (Util.SDK_INT <= 23 || player == null) {
      initializePlayer();
      if (playerView != null) {
        playerView.onResume();
      }
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    if (Util.SDK_INT <= 23) {
      if (playerView != null) {
        playerView.onPause();
      }
      releasePlayer();
    }
  }

  @Override
  public void onStop() {
    super.onStop();
    if (Util.SDK_INT > 23) {
      if (playerView != null) {
        playerView.onPause();
      }
      releasePlayer();
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    releaseAdsLoader();
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
      @NonNull int[] grantResults) {
    if (grantResults.length == 0) {
      // Empty results are triggered if a permission is requested while another request was already
      // pending and can be safely ignored in this case.
      return;
    }
    if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
      initializePlayer();
    } else {
      showToast(R.string.storage_permission_denied);
      finish();
    }
  }

  @Override
  public void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    updateTrackSelectorParameters();
    updateStartPosition();
    outState.putParcelable(KEY_TRACK_SELECTOR_PARAMETERS, trackSelectorParameters);
    outState.putBoolean(KEY_AUTO_PLAY, startAutoPlay);
    outState.putInt(KEY_WINDOW, startWindow);
    outState.putLong(KEY_POSITION, startPosition);
  }

  // Activity input

  @Override
  public boolean dispatchKeyEvent(KeyEvent event) {
    // See whether the player view wants to handle media or DPAD keys events.
    return playerView.dispatchKeyEvent(event) || super.dispatchKeyEvent(event);
  }

  public void popupSaveEQDialog() {
    final EditText input = new EditText(this);
    input.setHint("EQ Preset Name");
    LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT);
    input.setLayoutParams(lp);

    AlertDialog.Builder alertBuilder = new AlertDialog.Builder(this);
    alertBuilder.setTitle("");
    alertBuilder.setMessage("");
    alertBuilder.setPositiveButton("Save", new DialogInterface.OnClickListener() {
      public void onClick(DialogInterface dialog, int which) {
        String name = input.getText().toString();
        AppSettingsPreference.addEQParameter(PlayerActivity.this, name, eqBands);
        reloadAllBandEQs();
        int index = eqNames.indexOf(name);
        lstEQPicker.setSelection(index);
      }
    });
    alertBuilder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
      public void onClick(DialogInterface dialog, int which) {

      }
    });


    alertBuilder.setView(input);
    alertBuilder.create().show();
  }

  // OnClickListener methods

  @Override
  public void onClick(View view) {
    if (view == selectTracksButton
        && !isShowingTrackSelectionDialog
        && TrackSelectionDialog.willHaveContent(trackSelector)) {
      isShowingTrackSelectionDialog = true;
      TrackSelectionDialog trackSelectionDialog =
          TrackSelectionDialog.createForTrackSelector(
              trackSelector,
              /* onDismissListener= */ dismissedDialog -> isShowingTrackSelectionDialog = false);
      trackSelectionDialog.show(getSupportFragmentManager(), /* tag= */ null);
    }
    else if(view == rbAllChannel) {
      //Solo off
      for (int i = 0; i < rbChannels.length; i++) {
        rbChannels[i].setChecked(false);
      }

      HPSAudioProcessor.fade(HPSAudioProcessor.VolumeRamperType.FadeOutThenIn);
      new Handler().postDelayed(new Runnable() {
        @Override
        public void run() {
          HPSAudioProcessor.setSoloChannel(4095);

        }
      }, 200); //Time in milisecond
    }
    else if(view == btnSaveEQ) {
      popupSaveEQDialog();
    }
    else {
      int selectedSoloChannelId = 0;
      for (int i = 0; i < rbChannels.length; i++) {
        if (rbChannels[i] == view) {
          selectedSoloChannelId = i;
        } else {
          rbChannels[i].setChecked(false);
          rbAllChannel.setChecked(false);
        }
      }

      HPSAudioProcessor.fade(HPSAudioProcessor.VolumeRamperType.FadeOutThenIn);
      final int fSelectedSoloChannelId = selectedSoloChannelId;
      new Handler().postDelayed(new Runnable() {
        @Override
        public void run() {
          HPSAudioProcessor.setSoloChannel(1 << fSelectedSoloChannelId);

        }
      }, 200); //Time in milisecond
    }
  }

  @Override
  public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
    if(buttonView == swSonami) {
      if(isChecked) {
//        swChannels.setVisibility(View.VISIBLE);
        swEQ.setVisibility(View.VISIBLE);
      }
      else {
//        swChannels.setVisibility(View.GONE);
        swEQ.setVisibility(View.GONE);
//        layoutChannels.setVisibility(View.GONE);
        layoutEQ.setVisibility(View.GONE);
      }

      HPSAudioProcessor.fade(HPSAudioProcessor.VolumeRamperType.FadeOutThenIn);
      new Handler().postDelayed(new Runnable() {
        @Override
        public void run() {
          HPSAudioProcessor.gIsSonamiOn = isChecked;

        }
      }, 200); //Time in milisecond
    }
    else if(buttonView == swChannels) {
      if(isChecked) {
//        layoutChannels.setVisibility(View.VISIBLE);
        showAllChannelButtons();

      }
      else {
//        layoutChannels.setVisibility(View.GONE);
        hideAllChannelButtons();
      }
    }
    else if(buttonView == swEQ) {
      if(isChecked) {
        layoutEQ.setVisibility(View.VISIBLE);
      }
      else {
        layoutEQ.setVisibility(View.GONE);
        //Debug saving parameters
//        HashMap<String, float[]> fuck = new HashMap<>();
////        float[] eqBands = new float[10];
////        for(int i = 0; i < 10; i++) {
////          eqBands[i] = Float.parseFloat(lblEQs[i].getText().toString());
////        }
//        fuck.put("fuck", eqBands);
//        AppSettingsPreference.setEQParameter(this, fuck);
//
//        HashMap<String, float[]> fuck2 = AppSettingsPreference.loadEQParameter(this);
//        return;
      }
    }
//    else if(buttonView == swAllChannel) {
//      if(isChecked) {
//        //Solo off
//        for (int i = 0; i < swEachChannels.length; i++) {
//          swEachChannels[i].setChecked(false);
//        }
//      }
////      else {
//////        swAllChannel.setChecked(true);
////      }
//    }
//    else {
//      if(isChecked) {
//        int selectedSoloChannelId = 0;
//        for (int i = 0; i < swEachChannels.length; i++) {
//          if (swEachChannels[i] == buttonView) {
//            selectedSoloChannelId = i;
//          } else {
//            swEachChannels[i].setChecked(false);
//            swAllChannel.setChecked(false);
//          }
//        }
//      }
//    }
  }

  // PlaybackControlView.PlaybackPreparer implementation

  public void updateEQFromSliders() {
    for(int i = 0; i < eqSliders.length; i++) {
      float gain = (eqSliders[i].getProgress() - 100) / 10.0f;
      lblEQs[i].setText(String.format("%.1fdB", gain));
      eqBands[i] = gain;
    }
    HPSAudioProcessor.setEQ(eqBands);
  }

  @Override
  public void onStopTrackingTouch(SeekBar seekBar) {
    // TODO Auto-generated method stub
  }

  @Override
  public void onStartTrackingTouch(SeekBar seekBar) {
    // TODO Auto-generated method stub
  }

  @Override
  public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
//    if(fromUser) {
      updateEQFromSliders();
//    }
  }

  @Override
  public void preparePlayback() {
    player.retry();
  }

  // PlaybackControlView.VisibilityListener implementation

  @Override
  public void onVisibilityChange(int visibility) {
    debugRootView.setVisibility(visibility);


    View decorView = getWindow().getDecorView();
//    int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
    //| View.SYSTEM_UI_FLAG_FULLSCREEN;
    if(visibility == 0) {
      decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
    }
    else {
      decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE);
    }
  }

  // Internal methods

  private void initializePlayer() {
    if (player == null) {
      Intent intent = getIntent();

      mediaSource = createTopLevelMediaSource(intent);
      if (mediaSource == null) {
        return;
      }

      TrackSelection.Factory trackSelectionFactory;
      String abrAlgorithm = intent.getStringExtra(ABR_ALGORITHM_EXTRA);
      if (abrAlgorithm == null || ABR_ALGORITHM_DEFAULT.equals(abrAlgorithm)) {
        trackSelectionFactory = new AdaptiveTrackSelection.Factory();
      } else if (ABR_ALGORITHM_RANDOM.equals(abrAlgorithm)) {
        trackSelectionFactory = new RandomTrackSelection.Factory();
      } else {
        showToast(R.string.error_unrecognized_abr_algorithm);
        finish();
        return;
      }

      boolean preferExtensionDecoders =
          intent.getBooleanExtra(PREFER_EXTENSION_DECODERS_EXTRA, false);
      RenderersFactory renderersFactory =
          ((DemoApplication) getApplication()).buildRenderersFactory(preferExtensionDecoders);

      trackSelector = new DefaultTrackSelector(/* context= */ this, trackSelectionFactory);
      trackSelector.setParameters(trackSelectorParameters);
      lastSeenTrackGroupArray = null;

      player =
          new SimpleExoPlayer.Builder(/* context= */ this, renderersFactory)
              .setTrackSelector(trackSelector)
              .build();
      player.addListener(new PlayerEventListener());
      player.setAudioAttributes(AudioAttributes.DEFAULT, /* handleAudioFocus= */ true);
      player.setPlayWhenReady(startAutoPlay);
      player.addAnalyticsListener(new EventLogger(trackSelector));
      playerView.setPlayer(player);
      playerView.setPlaybackPreparer(this);
      debugViewHelper = new DebugTextViewHelper(player, debugTextView);
      debugViewHelper.start();
      if (adsLoader != null) {
        adsLoader.setPlayer(player);
      }
    }
    boolean haveStartPosition = startWindow != C.INDEX_UNSET;
    if (haveStartPosition) {
      player.seekTo(startWindow, startPosition);
    }
    player.prepare(mediaSource, !haveStartPosition, false);
    updateButtonVisibility();


  }

  @Nullable
  private MediaSource createTopLevelMediaSource(Intent intent) {
    String action = intent.getAction();
    boolean actionIsListView = ACTION_VIEW_LIST.equals(action);
    if (!actionIsListView && !ACTION_VIEW.equals(action)) {
      showToast(getString(R.string.unexpected_intent_action, action));
      finish();
      return null;
    }

    Sample intentAsSample = Sample.createFromIntent(intent);
    UriSample[] samples =
        intentAsSample instanceof Sample.PlaylistSample
            ? ((Sample.PlaylistSample) intentAsSample).children
            : new UriSample[] {(UriSample) intentAsSample};

    boolean seenAdsTagUri = false;
    for (UriSample sample : samples) {
      seenAdsTagUri |= sample.adTagUri != null;
      if (!Util.checkCleartextTrafficPermitted(sample.uri)) {
        showToast(R.string.error_cleartext_not_permitted);
        return null;
      }
      if (Util.maybeRequestReadExternalStoragePermission(/* activity= */ this, sample.uri)) {
        // The player will be reinitialized if the permission is granted.
        return null;
      }
    }

    MediaSource[] mediaSources = new MediaSource[samples.length];
    for (int i = 0; i < samples.length; i++) {
      mediaSources[i] = createLeafMediaSource(samples[i]);
      Sample.SubtitleInfo subtitleInfo = samples[i].subtitleInfo;
      if (subtitleInfo != null) {
        if (Util.maybeRequestReadExternalStoragePermission(
            /* activity= */ this, subtitleInfo.uri)) {
          // The player will be reinitialized if the permission is granted.
          return null;
        }
        Format subtitleFormat =
            Format.createTextSampleFormat(
                /* id= */ null,
                subtitleInfo.mimeType,
                C.SELECTION_FLAG_DEFAULT,
                subtitleInfo.language);
        MediaSource subtitleMediaSource =
            new SingleSampleMediaSource.Factory(dataSourceFactory)
                .createMediaSource(subtitleInfo.uri, subtitleFormat, C.TIME_UNSET);
        mediaSources[i] = new MergingMediaSource(mediaSources[i], subtitleMediaSource);
      }
    }
    MediaSource mediaSource =
        mediaSources.length == 1 ? mediaSources[0] : new ConcatenatingMediaSource(mediaSources);

    if (seenAdsTagUri) {
      Uri adTagUri = samples[0].adTagUri;
      if (actionIsListView) {
        showToast(R.string.unsupported_ads_in_concatenation);
      } else {
        if (!adTagUri.equals(loadedAdTagUri)) {
          releaseAdsLoader();
          loadedAdTagUri = adTagUri;
        }
        MediaSource adsMediaSource = createAdsMediaSource(mediaSource, adTagUri);
        if (adsMediaSource != null) {
          mediaSource = adsMediaSource;
        } else {
          showToast(R.string.ima_not_loaded);
        }
      }
    } else {
      releaseAdsLoader();
    }

    return mediaSource;
  }

  private MediaSource createLeafMediaSource(UriSample parameters) {
    Sample.DrmInfo drmInfo = parameters.drmInfo;
    int errorStringId = R.string.error_drm_unknown;
    DrmSessionManager<ExoMediaCrypto> drmSessionManager = null;
    if (drmInfo == null) {
      drmSessionManager = DrmSessionManager.getDummyDrmSessionManager();
    } else if (Util.SDK_INT < 18) {
      errorStringId = R.string.error_drm_unsupported_before_api_18;
    } else if (!MediaDrm.isCryptoSchemeSupported(drmInfo.drmScheme)) {
      errorStringId = R.string.error_drm_unsupported_scheme;
    } else {
      MediaDrmCallback mediaDrmCallback =
          createMediaDrmCallback(drmInfo.drmLicenseUrl, drmInfo.drmKeyRequestProperties);
      drmSessionManager =
          new DefaultDrmSessionManager.Builder()
              .setUuidAndExoMediaDrmProvider(drmInfo.drmScheme, FrameworkMediaDrm.DEFAULT_PROVIDER)
              .setMultiSession(drmInfo.drmMultiSession)
              .build(mediaDrmCallback);
    }

    if (drmSessionManager == null) {
      showToast(errorStringId);
      finish();
      return null;
    }

    DownloadRequest downloadRequest =
        ((DemoApplication) getApplication())
            .getDownloadTracker()
            .getDownloadRequest(parameters.uri);
    if (downloadRequest != null) {
      return DownloadHelper.createMediaSource(downloadRequest, dataSourceFactory);
    }
    return createLeafMediaSource(parameters.uri, parameters.extension, drmSessionManager);
  }

  private MediaSource createLeafMediaSource(
      Uri uri, String extension, DrmSessionManager<?> drmSessionManager) {
    @ContentType int type = Util.inferContentType(uri, extension);
    switch (type) {
      case C.TYPE_DASH:
        return new DashMediaSource.Factory(dataSourceFactory)
            .setDrmSessionManager(drmSessionManager)
            .createMediaSource(uri);
      case C.TYPE_SS:
        return new SsMediaSource.Factory(dataSourceFactory)
            .setDrmSessionManager(drmSessionManager)
            .createMediaSource(uri);
      case C.TYPE_HLS:
        return new HlsMediaSource.Factory(dataSourceFactory)
            .setDrmSessionManager(drmSessionManager)
            .createMediaSource(uri);
      case C.TYPE_OTHER:
        return new ProgressiveMediaSource.Factory(dataSourceFactory)
            .setDrmSessionManager(drmSessionManager)
            .createMediaSource(uri);
      default:
        throw new IllegalStateException("Unsupported type: " + type);
    }
  }

  private HttpMediaDrmCallback createMediaDrmCallback(
      String licenseUrl, String[] keyRequestPropertiesArray) {
    HttpDataSource.Factory licenseDataSourceFactory =
        ((DemoApplication) getApplication()).buildHttpDataSourceFactory();
    HttpMediaDrmCallback drmCallback =
        new HttpMediaDrmCallback(licenseUrl, licenseDataSourceFactory);
    if (keyRequestPropertiesArray != null) {
      for (int i = 0; i < keyRequestPropertiesArray.length - 1; i += 2) {
        drmCallback.setKeyRequestProperty(keyRequestPropertiesArray[i],
            keyRequestPropertiesArray[i + 1]);
      }
    }
    return drmCallback;
  }

  private void releasePlayer() {
    if (player != null) {
      updateTrackSelectorParameters();
      updateStartPosition();
      debugViewHelper.stop();
      debugViewHelper = null;
      player.release();
      player = null;
      mediaSource = null;
      trackSelector = null;
    }
    if (adsLoader != null) {
      adsLoader.setPlayer(null);
    }
  }

  private void releaseAdsLoader() {
    if (adsLoader != null) {
      adsLoader.release();
      adsLoader = null;
      loadedAdTagUri = null;
      playerView.getOverlayFrameLayout().removeAllViews();
    }
  }

  private void updateTrackSelectorParameters() {
    if (trackSelector != null) {
      trackSelectorParameters = trackSelector.getParameters();
    }
  }

  private void updateStartPosition() {
    if (player != null) {
      startAutoPlay = player.getPlayWhenReady();
      startWindow = player.getCurrentWindowIndex();
      startPosition = Math.max(0, player.getContentPosition());
    }
  }

  private void clearStartPosition() {
    startAutoPlay = true;
    startWindow = C.INDEX_UNSET;
    startPosition = C.TIME_UNSET;
  }

  /** Returns a new DataSource factory. */
  private DataSource.Factory buildDataSourceFactory() {
    return ((DemoApplication) getApplication()).buildDataSourceFactory();
  }

  /** Returns an ads media source, reusing the ads loader if one exists. */
  @Nullable
  private MediaSource createAdsMediaSource(MediaSource mediaSource, Uri adTagUri) {
    // Load the extension source using reflection so the demo app doesn't have to depend on it.
    // The ads loader is reused for multiple playbacks, so that ad playback can resume.
    try {
      Class<?> loaderClass = Class.forName("com.google.android.exoplayer2.ext.ima.ImaAdsLoader");
      if (adsLoader == null) {
        // Full class names used so the LINT.IfChange rule triggers should any of the classes move.
        // LINT.IfChange
        Constructor<? extends AdsLoader> loaderConstructor =
            loaderClass
                .asSubclass(AdsLoader.class)
                .getConstructor(android.content.Context.class, android.net.Uri.class);
        // LINT.ThenChange(../../../../../../../../proguard-rules.txt)
        adsLoader = loaderConstructor.newInstance(this, adTagUri);
      }
      MediaSourceFactory adMediaSourceFactory =
          new MediaSourceFactory() {

            private DrmSessionManager<?> drmSessionManager =
                DrmSessionManager.getDummyDrmSessionManager();

            @Override
            public MediaSourceFactory setDrmSessionManager(DrmSessionManager<?> drmSessionManager) {
              this.drmSessionManager = drmSessionManager;
              return this;
            }

            @Override
            public MediaSource createMediaSource(Uri uri) {
              return PlayerActivity.this.createLeafMediaSource(
                  uri, /* extension=*/ null, drmSessionManager);
            }

            @Override
            public int[] getSupportedTypes() {
              return new int[] {C.TYPE_DASH, C.TYPE_SS, C.TYPE_HLS, C.TYPE_OTHER};
            }
          };
      return new AdsMediaSource(mediaSource, adMediaSourceFactory, adsLoader, playerView);
    } catch (ClassNotFoundException e) {
      // IMA extension not loaded.
      return null;
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  // User controls

  private void updateButtonVisibility() {
    selectTracksButton.setEnabled(
        player != null && TrackSelectionDialog.willHaveContent(trackSelector));
  }

  private void showControls() {
    debugRootView.setVisibility(View.VISIBLE);
  }

  private void showToast(int messageId) {
    showToast(getString(messageId));
  }

  private void showToast(String message) {
    Toast.makeText(getApplicationContext(), message, Toast.LENGTH_LONG).show();
  }

  private static boolean isBehindLiveWindow(ExoPlaybackException e) {
    if (e.type != ExoPlaybackException.TYPE_SOURCE) {
      return false;
    }
    Throwable cause = e.getSourceException();
    while (cause != null) {
      if (cause instanceof BehindLiveWindowException) {
        return true;
      }
      cause = cause.getCause();
    }
    return false;
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
    String eqName = eqNames.get(position);
    float[] eqBands = allEQs.get(eqName);
    for(int i = 0; i < eqBands.length; i++) {
      int progress = (int)(eqBands[i] * 10 + 100);
      eqSliders[i].setProgress(progress);
      eqSliders[i].updateThumb();
    }

    updateEQFromSliders();
  }

  @Override
  public void onNothingSelected(AdapterView<?> parent) {
    return;
  }

  private class PlayerEventListener implements Player.EventListener {

    @Override
    public void onPlayerStateChanged(boolean playWhenReady, @Player.State int playbackState) {
      if (playbackState == Player.STATE_ENDED) {
        showControls();
      }
      updateButtonVisibility();
    }

    @Override
    public void onPlayerError(ExoPlaybackException e) {
      if (isBehindLiveWindow(e)) {
        clearStartPosition();
        initializePlayer();
      } else {
        updateButtonVisibility();
        showControls();
      }
    }

    @Override
    @SuppressWarnings("ReferenceEquality")
    public void onTracksChanged(TrackGroupArray trackGroups, TrackSelectionArray trackSelections) {
      updateButtonVisibility();
      if (trackGroups != lastSeenTrackGroupArray) {
        MappedTrackInfo mappedTrackInfo = trackSelector.getCurrentMappedTrackInfo();
        if (mappedTrackInfo != null) {
          if (mappedTrackInfo.getTypeSupport(C.TRACK_TYPE_VIDEO)
              == MappedTrackInfo.RENDERER_SUPPORT_UNSUPPORTED_TRACKS) {
            showToast(R.string.error_unsupported_video);
          }
          if (mappedTrackInfo.getTypeSupport(C.TRACK_TYPE_AUDIO)
              == MappedTrackInfo.RENDERER_SUPPORT_UNSUPPORTED_TRACKS) {
            showToast(R.string.error_unsupported_audio);
          }
        }
        lastSeenTrackGroupArray = trackGroups;
      }
    }
  }

  private class PlayerErrorMessageProvider implements ErrorMessageProvider<ExoPlaybackException> {

    @Override
    public Pair<Integer, String> getErrorMessage(ExoPlaybackException e) {
      String errorString = getString(R.string.error_generic);
      if (e.type == ExoPlaybackException.TYPE_RENDERER) {
        Exception cause = e.getRendererException();
        if (cause instanceof DecoderInitializationException) {
          // Special case for decoder initialization failures.
          DecoderInitializationException decoderInitializationException =
              (DecoderInitializationException) cause;
          if (decoderInitializationException.codecInfo == null) {
            if (decoderInitializationException.getCause() instanceof DecoderQueryException) {
              errorString = getString(R.string.error_querying_decoders);
            } else if (decoderInitializationException.secureDecoderRequired) {
              errorString =
                  getString(
                      R.string.error_no_secure_decoder, decoderInitializationException.mimeType);
            } else {
              errorString =
                  getString(R.string.error_no_decoder, decoderInitializationException.mimeType);
            }
          } else {
            errorString =
                getString(
                    R.string.error_instantiating_decoder,
                    decoderInitializationException.codecInfo.name);
          }
        }
      }
      return Pair.create(0, errorString);
    }
  }
}
