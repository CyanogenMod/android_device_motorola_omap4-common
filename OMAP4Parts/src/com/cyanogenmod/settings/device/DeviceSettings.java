/*
* Copyright (C) 2013 The CyanogenMod Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.cyanogenmod.settings.device;

import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.os.SystemProperties;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

public class DeviceSettings extends PreferenceActivity implements OnPreferenceChangeListener {

    private CheckBoxPreference mSwitchStoragePref=null;
    private CheckBoxPreference mGSMSignalStrengthPref=null;

    private static final String TAG = "OMAP4Parts";
    private static final String KEY_SWITCH_STORAGE = "key_switch_storage";
    private static final String KEY_GSM_SIGNALSTRENGTH = "gsm_signalstrength";
    private static final String AID = "ro.telephony.ril.v3=writeaidonly";
    private static final String AID_SIGNAL = "ro.telephony.ril.v3=writeaidonly,signalstrength";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.main_preferences);

        if (SystemProperties.get("ro.telephony.ril.v3","").equals("writeaidonly")) {
            SystemProperties.set("persist.sys.gsm_fix", "0");
        } else if (SystemProperties.get("ro.telephony.ril.v3","").equals(
                "writeaidonly,signalstrength")) {
            SystemProperties.set("persist.sys.gsm_fix", "1");
        }
        mGSMSignalStrengthPref = (CheckBoxPreference) getPreferenceScreen().findPreference(
                KEY_GSM_SIGNALSTRENGTH);
        mGSMSignalStrengthPref.setChecked((SystemProperties.getInt("persist.sys.gsm_fix", 0) == 1));
        mGSMSignalStrengthPref.setOnPreferenceChangeListener(this);
        if ((SystemProperties.get("ro.product.device","").equals("umts_spyder")) ||
                (SystemProperties.get("ro.product.device","").equals("edison"))) {
            mGSMSignalStrengthPref.setSummaryOff(R.string.gsm_signalstrength_unavailable);
            mGSMSignalStrengthPref.setEnabled(false);
        }
        mSwitchStoragePref = (CheckBoxPreference) getPreferenceScreen().findPreference(
            KEY_SWITCH_STORAGE);
        mSwitchStoragePref.setChecked((SystemProperties.getInt(
                "persist.sys.vold.switchexternal", 0) == 1));
        mSwitchStoragePref.setOnPreferenceChangeListener(this);
        if (SystemProperties.get("ro.vold.switchablepair","").equals("")) {
            mSwitchStoragePref.setSummaryOff(R.string.storage_switch_unavailable);
            mSwitchStoragePref.setEnabled(false);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if(preference == mGSMSignalStrengthPref) {
            Log.d(TAG,"Setting persist.sys.gsm_fix to "+(
                    mGSMSignalStrengthPref.isChecked() ? "1" : "0"));
            SystemProperties.set("persist.sys.gsm_fix", ((Boolean) newValue) ? "1" : "0");
            if ("0".equals(SystemProperties.get("persist.sys.gsm_fix"))) {
                File file = new File("/data/local.prop");
                if (!file.exists())
                try {
                    file.createNewFile();
                }catch(Exception e){
                    e.printStackTrace();
                }
                FileWriter fWriter;
                try {
                     fWriter = new FileWriter(file);
                     fWriter.write(AID);
                     fWriter.flush();
                     fWriter.close();
                }catch(Exception e){
                    e.printStackTrace();
                }
            } else if ("1".equals(SystemProperties.get("persist.sys.gsm_fix"))) {
                File file =new File("/data/local.prop");
                if (!file.exists())
                try {
                    file.createNewFile();
                }catch(Exception e){
                    e.printStackTrace();
                }
                 FileWriter fWriter;
                 try {
                     fWriter = new FileWriter(file);
                     fWriter.write(AID_SIGNAL);
                     fWriter.flush();
                     fWriter.close();
                }catch(Exception e){
                    e.printStackTrace();
                }
            }
        }
        if(preference == mSwitchStoragePref) {
            Log.d(TAG,"Setting persist.sys.vold.switchexternal to "+(
                    mSwitchStoragePref.isChecked() ? "1" : "0"));
            SystemProperties.set("persist.sys.vold.switchexternal", (
                    (Boolean) newValue) ? "1" : "0");
        }
        return true;
    }
}
