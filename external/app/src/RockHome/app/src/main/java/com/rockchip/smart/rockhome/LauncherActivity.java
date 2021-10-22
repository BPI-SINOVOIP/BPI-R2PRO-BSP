package com.rockchip.smart.rockhome;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class LauncherActivity extends AppCompatActivity {

    private static final int REQUEST_READ_PHONE_STATE = 0x00000001;
    private static final int REQUEST_PERMISSION_STORAGE = 0x00000002;

    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);

        requestPermission(this);
    }

    private void requestPermission(Context context) {
        if (ContextCompat.checkSelfPermission(context, Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_PHONE_STATE}, REQUEST_READ_PHONE_STATE);
        } else {
            if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP && ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, REQUEST_PERMISSION_STORAGE);
            } else {
                new Handler().postDelayed(new Runnable() {

                    @Override
                    public void run() {
                        Intent intent = new Intent(LauncherActivity.this, RockHomeActivity.class);
                        startActivity(intent);
                        finish();
                    }
                }, 2000);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQUEST_READ_PHONE_STATE:
                if ((grantResults.length > 0) && (grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP && ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                        ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, REQUEST_PERMISSION_STORAGE);
                    } else {

                        //TODO
                        new Handler().postDelayed(new Runnable() {

                            @Override
                            public void run() {
                                Intent intent = new Intent(LauncherActivity.this, RockHomeActivity.class);
                                startActivity(intent);
                                finish();
                            }
                        }, 2000);
                    }
                } else {
                    finish();
                }
                break;
            case REQUEST_PERMISSION_STORAGE:
                if ((grantResults.length > 0) && (grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    //TODO
                    new Handler().postDelayed(new Runnable() {

                        @Override
                        public void run() {
                            Intent intent = new Intent(LauncherActivity.this, RockHomeActivity.class);
                            startActivity(intent);
                            finish();
                        }
                    }, 2000);
                } else {
                    finish();
                }
                break;
        }
    }
}
