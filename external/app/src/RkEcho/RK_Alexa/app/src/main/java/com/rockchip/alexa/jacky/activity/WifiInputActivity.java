package com.rockchip.alexa.jacky.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.support.v7.widget.Toolbar;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.app.AuthManager;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.info.WifiInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.listener.OnSingleDialogListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.utils.SharedPreference;
import com.rockchip.alexa.jacky.views.LoadingView;

import org.json.JSONException;

import java.io.IOException;

/**
 * Created by Administrator on 2017/4/7.
 */

public class WifiInputActivity extends Activity {
    private static final String TAG = "WifiInputFragment";
    private LoadingView mLoadingView;
    private Toolbar mToolbar;
    private TextView mWifiSSID, mConfirm;
    private TextView mToolbarBack;
    private ImageView mWifiPwdLock;
    private boolean mShowPwd;
    private EditText mWifiPwd;

    private boolean mSmartConfiged;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wifi_input);


        Intent intent = getIntent();
        final String ssid = intent.getStringExtra("ssid");
        mToolbar = (Toolbar) findViewById(R.id.toolbar);
        mToolbarBack = (TextView) findViewById(R.id.toolbar_back);
        mToolbarBack.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                WifiInputActivity.this.finish();
            }
        });
        mWifiSSID = (TextView) findViewById(R.id.frag_wifi_input_ssid);
        mWifiSSID.setText(ssid);
        mWifiPwdLock = (ImageView) findViewById(R.id.frag_wifi_input_lock);
        mWifiPwd = (EditText) findViewById(R.id.frag_wifi_input_pwd);
        if (!SharedPreference.getString(ssid, "").equals("")) {
            mWifiPwd.setText(SharedPreference.getString(ssid, ""));
        }
        mConfirm = (TextView) findViewById(R.id.frag_wifi_input_confirm);
        mConfirm.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View v) {
                if (!Env.isWifiEnable(WifiInputActivity.this) || !Env.isWifiConnected(WifiInputActivity.this) || !Env.getConnectingSSID(WifiInputActivity.this).startsWith(Context.HOTSPOT_PREFIX)) {
                    Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.device_not_connected), Toast.LENGTH_SHORT).show();
                    return;
                }
                String pwd = mWifiPwd.getText().toString();
                if (pwd == null || pwd.length() < 8) {
                    Toast.makeText(BaseApplication.getApplication(), getResources().getString(R.string.error_pwd), Toast.LENGTH_SHORT).show();
                } else {
                    SharedPreference.putString(ssid, mWifiPwd.getText().toString());
                    if (mLoadingView != null && !mLoadingView.isShowing()) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mLoadingView.show();
                                mSmartConfiged = false;
                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        try {
                                            boolean result = false;
                                            if (!BaseApplication.getApplication().isSetupWifi()) {
                                                if (SystemClock.uptimeMillis() - BaseApplication.getApplication().getAuthorizeTime() > 100 * 1000) {
                                                    runOnUiThread(new Runnable() {
                                                        @Override
                                                        public void run() {
                                                            mLoadingView.dismiss();
                                                            Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.author_info_expire), Toast.LENGTH_SHORT).show();
                                                        }
                                                    });
                                                    return;
                                                }
                                                result = ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postCompanionProvisioningInfo(AuthManager.getAuthManager().getCompanionProvisioningInfo());//ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postWifiSetupInfo(ssid, mWifiPwd.getText().toString().trim());
                                                if (result) {
                                                    runOnUiThread(new Runnable() {
                                                        @Override
                                                        public void run() {
                                                            Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.author_info_send_success), Toast.LENGTH_SHORT).show();
                                                        }
                                                    });
                                                } else {
                                                    runOnUiThread(new Runnable() {
                                                        @Override
                                                        public void run() {
                                                            AlertDialog.Builder alert=new AlertDialog.Builder(WifiInputActivity.this);
                                                            alert.setTitle(getResources().getString(R.string.title_prompt));
                                                            alert.setMessage(getResources().getString(R.string.if_continue_setup_wifi_after_fail));

                                                            alert.setPositiveButton(getResources().getString(R.string.confirm), new OnSingleDialogListener() {
                                                                @Override
                                                                protected void onSingleClick(View v) {
                                                                    boolean result1 = ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postWifiSetupInfo(ssid, mWifiPwd.getText().toString().trim(), true);
                                                                    if (result1) {
                                                                        mSmartConfiged = true;
                                                                        runOnUiThread(new Runnable() {
                                                                            @Override
                                                                            public void run() {
                                                                                mLoadingView.dismiss();
                                                                                Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.wifi_send_success), Toast.LENGTH_SHORT).show();
//                                                                                ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postWifiSetupInfo(ssid, mWifiPwd.getText().toString().trim(), true);
                                                                            }
                                                                        });
                                                                    } else {
                                                                        runOnUiThread(new Runnable() {
                                                                            @Override
                                                                            public void run() {
                                                                                Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.wifi_send_fail), Toast.LENGTH_SHORT).show();
                                                                            }
                                                                        });
                                                                    }
                                                                }
                                                            });

                                                            alert.setNegativeButton(getResources().getString(R.string.cancel), new OnSingleDialogListener() {
                                                                @Override
                                                                protected void onSingleClick(View v) {

                                                                }
                                                            });
                                                            alert.create();
                                                            alert.show();
                                                        }
                                                    });
                                                    return;
                                                }
                                            }
                                            result = ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postWifiSetupInfo(ssid, mWifiPwd.getText().toString().trim(), true);
                                            if (result) {
                                                mSmartConfiged = true;
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        mLoadingView.dismiss();
                                                        Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.wifi_send_success), Toast.LENGTH_SHORT).show();
                                                    }
                                                });
//                                                ProvisioningClient.getProvisioningClient(WifiInputActivity.this).postWifiSetupInfo(ssid, mWifiPwd.getText().toString().trim(), true);
                                            } else {
                                                runOnUiThread(new Runnable() {
                                                    @Override
                                                    public void run() {
                                                        Toast.makeText(WifiInputActivity.this, getResources().getString(R.string.wifi_send_fail), Toast.LENGTH_SHORT).show();
                                                    }
                                                });
                                            }
                                        } catch (IOException e) {
                                            e.printStackTrace();
                                        } catch (JSONException e) {
                                            e.printStackTrace();
                                        }
                                    }
                                }).start();
                            }
                        });
                    }
                }
            }
        });

        mWifiPwdLock.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View v) {
                mShowPwd = !mShowPwd;
                if (mShowPwd) {
                    mWifiPwdLock.setImageResource(R.drawable.password_input_02);
                    mWifiPwd.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                } else {
                    mWifiPwdLock.setImageResource(R.drawable.password_input_01);
                    mWifiPwd.setTransformationMethod(PasswordTransformationMethod.getInstance());
                }
            }
        });

        mLoadingView = new LoadingView(this, getResources().getString(R.string.wifi_setting_up));
    }
}
