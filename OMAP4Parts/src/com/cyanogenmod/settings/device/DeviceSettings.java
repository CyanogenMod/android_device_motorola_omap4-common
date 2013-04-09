package com.cyanogenmod.settings.device;

import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.os.SystemProperties;

public class DeviceSettings extends PreferenceActivity implements OnPreferenceChangeListener {

    private CheckBoxPreference mSwitchStoragePref=null;

    private static final String TAG = "OMAP4Parts";
    private static final String KEY_SWITCH_STORAGE = "key_switch_storage";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.main_preferences);

        mSwitchStoragePref = (CheckBoxPreference) getPreferenceScreen().findPreference(KEY_SWITCH_STORAGE);
        mSwitchStoragePref.setChecked((SystemProperties.getInt("persist.sys.vold.switchexternal", 0) == 1));
        mSwitchStoragePref.setOnPreferenceChangeListener(this);
        if (SystemProperties.get("ro.vold.switchablepair","").equals("")) {
            mSwitchStoragePref.setSummary(R.string.storage_switch_unavailable);
            mSwitchStoragePref.setEnabled(false);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if(preference == mSwitchStoragePref) {
            Log.d(TAG,"Setting persist.sys.vold.switchexternal to "+(mSwitchStoragePref.isChecked() ? "1" : "0"));
            SystemProperties.set("persist.sys.vold.switchexternal", ((Boolean) newValue) ? "1" : "0");
        }
        return true;
    }
}
