package com.google.android.exoplayer2.demo;

import android.content.Context;
import android.os.Handler;
import android.util.Log;

import com.google.android.gms.analytics.ExceptionParser;

import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * Created by user1 on 10/11/17.
 */

public class AnalyticsExceptionParser implements ExceptionParser {
    private static String TAG = "AnalyticsExceptionParse";
    private Context ctx;

    public AnalyticsExceptionParser(Context context)
    {
        ctx = context;
    }

    /*
     * (non-Javadoc)
     * @see com.google.analytics.tracking.android.ExceptionParser#getDescription(java.lang.String, java.lang.Throwable)
     */
    public String getDescription(String p_thread, Throwable p_throwable) {
        StringWriter errors = new StringWriter();
        p_throwable.printStackTrace(new PrintWriter(errors));

        //Output logs
        if(ctx != null) {
            Log.e(TAG, "OOPS CRASH: Thread:" + p_thread + ", Exception:" + errors.toString() + "\n");
            LogUtil.writeLogToFile(ctx, "OOPS CRASH: Thread:" + p_thread + ", Exception:" + errors.toString() + "\n");
        }

        new Handler(ctx.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                System.exit(1);
            }
        }, 500);

        return "Thread: " + p_thread + ", Exception: " + errors.toString();
    }
}
