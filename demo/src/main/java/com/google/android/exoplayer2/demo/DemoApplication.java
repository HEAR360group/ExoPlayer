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
package com.google.android.exoplayer2.demo;

import android.app.Application;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.DefaultHttpDataSourceFactory;
import com.google.android.exoplayer2.upstream.HttpDataSource;
import com.google.android.exoplayer2.util.Util;
import com.google.android.gms.analytics.ExceptionReporter;
import com.google.android.gms.analytics.GoogleAnalytics;
import com.google.android.gms.analytics.Tracker;

/**
 * Placeholder application to facilitate overriding Application methods for debugging and testing.
 */
public class DemoApplication extends Application {
  private static GoogleAnalytics sAnalytics;
  private static Tracker sGATracker;

  protected String userAgent;

  @Override
  public void onCreate() {
    super.onCreate();
    userAgent = Util.getUserAgent(this, "ExoPlayerDemo");

    //Google Analytics Integration
    sAnalytics = GoogleAnalytics.getInstance(this);
    sAnalytics.setLocalDispatchPeriod(1800);
    sGATracker = sAnalytics.newTracker("UA-107888244-1");
    sGATracker.enableExceptionReporting(true);
    sGATracker.enableAdvertisingIdCollection(true);
    sGATracker.enableAutoActivityTracking(true);

    Thread.UncaughtExceptionHandler uncaughtExceptionHandler = Thread.getDefaultUncaughtExceptionHandler();
    if (uncaughtExceptionHandler instanceof ExceptionReporter) {
      ExceptionReporter exceptionReporter = (ExceptionReporter) uncaughtExceptionHandler;

      AnalyticsExceptionParser exceptionParser = new AnalyticsExceptionParser(this);
      exceptionReporter.setExceptionParser(exceptionParser);
    }

    LogUtil.exportLog(getApplicationContext());

    LogUtil.writeLogToFile(getApplicationContext(), "Launched");
  }

  public DataSource.Factory buildDataSourceFactory(DefaultBandwidthMeter bandwidthMeter) {
    return new DefaultDataSourceFactory(this, bandwidthMeter,
        buildHttpDataSourceFactory(bandwidthMeter));
  }

  public HttpDataSource.Factory buildHttpDataSourceFactory(DefaultBandwidthMeter bandwidthMeter) {
    return new DefaultHttpDataSourceFactory(userAgent, bandwidthMeter);
  }

  public boolean useExtensionRenderers() {
    return BuildConfig.FLAVOR.equals("withExtensions");
  }

  /**
   * Gets the default {@link Tracker} for this {@link Application}.
   * @return tracker
   */
  synchronized public Tracker getDefaultTracker() {
    // To enable debug logging use: adb shell setprop log.tag.GAv4 DEBUG
    if (sGATracker == null) {
      sGATracker = sAnalytics.newTracker(R.xml.global_tracker);
    }

    return sGATracker;
  }
}
