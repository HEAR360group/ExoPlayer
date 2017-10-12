package com.google.android.exoplayer2.demo;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by user1 on 10/11/17.
 */

public class LogUtil {
    private static final String TAG = "LogUtil";

    public static final boolean LOG_ENABLED = true;
    public static final String LOG_DIRECTORY = "Hear360_Logs";
    public static final String LOG_FILE = "Hear360Log";

    public static void writeLogToFile(Context ctx, String log) {
        if (!LOG_ENABLED)
            return;

        String methodName = Thread.currentThread().getStackTrace()[3].getMethodName();
        if(!methodName.equals("writeLogToFile"))
        {
            String fullClassName = Thread.currentThread().getStackTrace()[4].getClassName();
            String methodNameI = Thread.currentThread().getStackTrace()[4].getMethodName();
            int lineNumber = Thread.currentThread().getStackTrace()[4].getLineNumber();
            log += " @ line " + lineNumber + " @ class " + fullClassName + " @ method " + methodNameI;
        }

        SimpleDateFormat sdf = new SimpleDateFormat("MMM dd, HH:mm:ss.SSS");
        log += ", " + sdf.format(new Date()) + "\n";

        FileOutputStream outputStream;

        try {
            outputStream = ctx.openFileOutput(LOG_FILE, Context.MODE_APPEND | Context.MODE_PRIVATE);
            outputStream.write(log.getBytes());
            outputStream.close();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static File getOutputFile(Context ctx) {
        File mediaFile = new File(getOutputFileName(ctx));

        return mediaFile;
    }

    public static String getOutputFileName(Context ctx) {
        File mediaStorageDir = ctx.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS);
        //File mediaStorageDir = new File(Environment.getExternalStoragePublicDirectory(
                //Environment.DIRECTORY_DOCUMENTS), "");

        if (!mediaStorageDir.exists()) {
            if (!mediaStorageDir.mkdirs()) {
                Log.e(TAG, "failed to create directory");
                return null;
            }
        }

        // Create a media file name
        String mediaFileName = mediaStorageDir.getPath() + File.separator + LOG_FILE;

        //String timeStamp = new SimpleDateFormat("MMdd_HHmmss").format(new Date());
        //String mediaFileName = mediaStorageDir.getPath() + File.separator + GlobalConfig.LQ_LOG_FILE + timeStamp + ".csv";

        return mediaFileName;
    }

    public static void exportLog(Context ctx) {
        File writeFile = getOutputFile(ctx);

        if (writeFile != null && writeFile.exists()) {
            writeFile.delete();
        }

        try {
            writeFile.createNewFile();

            FileOutputStream out = new FileOutputStream(writeFile);
            out.write(readLogFile(ctx).toString().getBytes());
            out.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static StringBuilder readLogFile(Context ctx) {
        StringBuilder text = new StringBuilder();

        try {
            File readFile = new File(ctx.getFilesDir(), LOG_FILE);
            BufferedReader br = new BufferedReader(new FileReader(readFile));
            String line;

            while ((line = br.readLine()) != null) {
                text.append(line);
                text.append('\n');
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return text;
    }
}
