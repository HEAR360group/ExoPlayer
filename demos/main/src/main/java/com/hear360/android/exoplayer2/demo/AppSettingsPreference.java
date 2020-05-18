package com.hear360.android.exoplayer2.demo;

import android.content.Context;
import android.content.SharedPreferences;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class AppSettingsPreference {
    public static final String HEAR360_PREF = "Hear360Preferences";

    public static void setEQParameter(Context context, HashMap<String, float[]> eqs) {
        SharedPreferences prefs = context.getSharedPreferences(HEAR360_PREF, 0);
        SharedPreferences.Editor editor = prefs.edit();

        try {
            JSONObject allEQs = new JSONObject();
            for(String eqName : eqs.keySet()) {
                float[] eqBands = eqs.get(eqName);
                JSONArray eqBandsArray = new JSONArray();
                for(int i = 0; i < eqBands.length; i++) {
                    eqBandsArray.put((int)(eqBands[i] * 10.0f));
                }
                allEQs.put(eqName, eqBandsArray);
            }

            String allEQsStr = allEQs.toString();
            editor.putString("HeadphoneUserEQ", allEQsStr);
            editor.commit();
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public static void addEQParameter(Context context, String name, float[] eqBands) {
        HashMap<String, float[]> result = loadEQParameter(context);
        result.put(name, eqBands);
        setEQParameter(context, result);
    }

    public static HashMap<String, float[]> loadEQParameter(Context context) {
        SharedPreferences prefs = context.getSharedPreferences(HEAR360_PREF, 0);
        HashMap<String, float[]> result = new HashMap<>();

        BufferedReader br = new BufferedReader(new InputStreamReader(context.getResources().openRawResource(R.raw.headphone_eq_presets)));
        try {
            String jsonStr = br.readLine();
            JSONObject allEQsJson = new JSONObject(jsonStr);
            for (Iterator<String> it = allEQsJson.keys(); it.hasNext(); ) {
                String eqName = it.next();
                JSONArray eqBandsArray = (JSONArray) allEQsJson.get(eqName);
                int length = eqBandsArray.length();
                float[] eqBands = new float[length];
                for(int i = 0; i < length; i++) {
                    eqBands[i] = (float)eqBandsArray.getInt(i) / 10.0f;
                }
                result.put(eqName, eqBands);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        String jsonStr = prefs.getString("HeadphoneUserEQ", "");
        try {
            JSONObject allEQsJson = new JSONObject(jsonStr);
            for (Iterator<String> it = allEQsJson.keys(); it.hasNext(); ) {
                String eqName = it.next();
                JSONArray eqBandsArray = (JSONArray) allEQsJson.get(eqName);
                int length = eqBandsArray.length();
                float[] eqBands = new float[length];
                for(int i = 0; i < length; i++) {
                    eqBands[i] = (float)eqBandsArray.getInt(i) / 10.0f;
                }
                result.put(eqName, eqBands);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return result;
    }

//    public static HashMap<String, float[]> getEQParameterPresets(Context context) {
//        HashMap<String, float[]> result = new HashMap<>();
//
//        BufferedReader br = new BufferedReader(new InputStreamReader(context.getResources().openRawResource(R.raw.headphone_eq_presets)));
//        try {
//            String jsonStr = br.readLine();
//            JSONObject allEQsJson = new JSONObject(jsonStr);
//            for (Iterator<String> it = allEQsJson.keys(); it.hasNext(); ) {
//                String eqName = it.next();
//                JSONArray eqBandsArray = (JSONArray) allEQsJson.get(eqName);
//                int length = eqBandsArray.length();
//                float[] eqBands = new float[length];
//                for(int i = 0; i < length; i++) {
//                    eqBands[i] = (float)eqBandsArray.getInt(i) / 10.0f;
//                }
//                result.put(eqName, eqBands);
//            }
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//
//        return result;
//    }
}
