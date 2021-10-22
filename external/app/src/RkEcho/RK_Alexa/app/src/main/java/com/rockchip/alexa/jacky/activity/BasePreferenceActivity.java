package com.rockchip.alexa.jacky.activity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatDelegate;
import android.util.Log;
import android.view.MenuItem;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;

import static com.rockchip.alexa.jacky.activity.WifiListActivity.TAG;

/**
 * Created by cjs on 2017/5/8.
 */

public class BasePreferenceActivity extends PreferenceActivity{
    public Context mContext;

    public Preference prefCheckVersion;
    public Preference prefDownload;

    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupActionBar();
        mContext = this;
        verifyStoragePermissions();
    }

    public void verifyStoragePermissions() {
        Log.d(TAG, "verifyStoragePermissions");
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(mContext,
                Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions((Activity)mContext, PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE);
        }
    }

    public void setupActionBar() {
        ActionBar actionBar = AppCompatDelegate.create(this, null).getSupportActionBar();
        if (actionBar != null) {
            // Show the Up button in the action bar.
            actionBar.setTitle(R.string.pref_activity_update_title);
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            this.onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void showToastOnUiThread(final String text, final int duration){
        this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(mContext,text,duration).show();
            }
        });
    }
}
