package com.rockchip.alexa.jacky.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.adapter.WifiListAdapter;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.async.AsyncFactory;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.info.WifiInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.listener.OnSingleItemClickListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import java.util.List;

import static com.rockchip.alexa.jacky.async.AsyncFactory.getAsyncThread;

/**
 * Created by Administrator on 2017/4/28.
 */

public class HotspotActivity extends Activity {
    private final String TAG = "RK_Alexa";

    private Button mBtnConfirm;
    private TextView mBtnBack, mBtnSkip;
    private LoadingView mLoadingView;
    private AlertDialog alert;

    private boolean isFromUpdateActivity = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hotspot);

        // check whether is the request from deviceUpDate activity
        Bundle bundle = getIntent().getExtras();
        if(bundle != null){
            isFromUpdateActivity = bundle.getBoolean("fromUpdateActivity");
            Log.d(TAG, "get request from DeviceUpdateActivity,to set hostPot");
        }

        mBtnConfirm = (Button) findViewById(R.id.activity_hotspot_confirm);
        mBtnConfirm.setOnClickListener(onSingleClickListener);
        mBtnBack = (TextView) findViewById(R.id.toolbar_back);
        mBtnBack.setOnClickListener(onSingleClickListener);
        mBtnSkip = (TextView) findViewById(R.id.toolbar_skip);
        mBtnSkip.setOnClickListener(onSingleClickListener);
        mLoadingView = new LoadingView(this, getResources().getString(R.string.wifi_connecting));
    }

    private void onClickConfirm() {
        if(isFromUpdateActivity){
            final List<WifiInfo> list = getAsyncThread().scanDeviceHotspot();
            View view1 = LayoutInflater.from(this).inflate(R.layout.wifi_list, null);

            ListView listView = (ListView) view1.findViewById(R.id.wifi_list);
            listView.setOnItemClickListener(new OnSingleItemClickListener() {
                @Override
                protected void onSingleItemClick(AdapterView<?> adapterView, View view, final int position, long id) {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            mLoadingView.show();
                            int res = -1;
                            for (int i=0; i<5; i++) {
                                res = AsyncFactory.getAsyncThread().connect(list.get(position).getSsid());
                                if (res == 1) {
                                    break;
                                }
                            }
                            if (mLoadingView != null && mLoadingView.isShowing()) {
                                mLoadingView.dismiss();
                            }
                            BaseApplication.getApplication().dismissDialog(alert);
                            final int result = res;
                            if (result == 1) {
                                setResult(RESULT_OK, new Intent());
                                finish();
                            } else {
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast.makeText(HotspotActivity.this, (result == -1) ? getResources().getString(R.string.wifi_connected_fail) : (result == 0) ? getResources().getString(R.string.wifi_connected_timeout) : "", Toast.LENGTH_LONG).show();
                                    }
                                });
                            }
                        }
                    }).start();
                }
            });
            WifiListAdapter wifiListAdapter = new WifiListAdapter(HotspotActivity.this, list);
            listView.setAdapter(wifiListAdapter);
            alert = new AlertDialog.Builder(HotspotActivity.this).create();
            alert.setTitle(getResources().getString(R.string.name_scan_device));
            alert.setView(view1);
            alert.show();
        }else {
            if (Env.isWifiEnable(this)) {
                if (Env.isWifiConnected(this) && Env.getConnectingSSID(this).startsWith(Context.HOTSPOT_PREFIX)) {
                    if (BaseApplication.getApplication().isSetupWifi()) {
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                if (ProvisioningClient.getProvisioningClient(HotspotActivity.this).isAuthorized()) {
                                    startActivity(new Intent(HotspotActivity.this, WifiListActivity.class));
                                } else {
                                    showNoAuthorizedDialog();
                                }
                            }
                        }).start();
                    } else if (BaseApplication.getApplication().isSetupBlu()) {
                        startActivity(new Intent(HotspotActivity.this, BluListActivity.class));
                    } else {
                        startActivity(new Intent(HotspotActivity.this, WifiListActivity.class));
                    }
                    return;
                }

                final List<WifiInfo> list = getAsyncThread().scanDeviceHotspot();
                View view1 = LayoutInflater.from(this).inflate(R.layout.wifi_list, null);

                ListView listView = (ListView) view1.findViewById(R.id.wifi_list);
                listView.setOnItemClickListener(new OnSingleItemClickListener() {
                    @Override
                    protected void onSingleItemClick(AdapterView<?> adapterView, View view, final int position, long id) {
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                mLoadingView.show();
                                int res = -1;
                                for (int i=0; i<5; i++) {
                                    res = AsyncFactory.getAsyncThread().connect(list.get(position).getSsid());
                                    if (res == 1) {
                                        break;
                                    }
                                }
                                boolean isAuthorized = true;
                                if (BaseApplication.getApplication().isSetupWifi()) {
                                    isAuthorized = ProvisioningClient.getProvisioningClient(HotspotActivity.this).isAuthorized();
                                }
                                if (mLoadingView != null && mLoadingView.isShowing()) {
                                    mLoadingView.dismiss();
                                }
                                BaseApplication.getApplication().dismissDialog(alert);
                                final int result = res;
                                if (result == 1) {
                                    if (isAuthorized) {
                                        startActivity(new Intent(HotspotActivity.this, WifiListActivity.class));
                                    } else {
                                        showNoAuthorizedDialog();
                                    }
                                } else {
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            Toast.makeText(HotspotActivity.this, (result == -1) ? getResources().getString(R.string.wifi_connected_fail) : (result == 0) ? getResources().getString(R.string.wifi_connected_timeout) : "", Toast.LENGTH_LONG).show();
                                        }
                                    });
                                }
                            }
                        }).start();
                    }
                });
                WifiListAdapter wifiListAdapter = new WifiListAdapter(HotspotActivity.this, list);
                listView.setAdapter(wifiListAdapter);
                alert = new AlertDialog.Builder(HotspotActivity.this).create();
                alert.setTitle(getResources().getString(R.string.name_scan_device));
                alert.setView(view1);
                alert.show();
            }
        }
    }

    private void showNoAuthorizedDialog() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder alert=new AlertDialog.Builder(HotspotActivity.this);
                alert.setTitle(getResources().getString(R.string.title_prompt));
                alert.setMessage(getResources().getString(R.string.if_continue_setup_wifi));

                alert.setPositiveButton(getResources().getString(R.string.confirm), new DialogInterface.OnClickListener(){
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        startActivity(new Intent(HotspotActivity.this, WifiListActivity.class));
                    }
                });

                alert.setNegativeButton(getResources().getString(R.string.cancel), new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        HotspotActivity.this.finish();
                    }
                });
                alert.create();
                alert.show();
            }
        });
    }

    private OnSingleClickListener onSingleClickListener = new OnSingleClickListener() {
        @Override
        protected void onSingleClick(View view) {
            switch (view.getId()) {
                case R.id.activity_hotspot_confirm:
                    onClickConfirm();
                    break;
                case R.id.toolbar_back:
                    HotspotActivity.this.finish();
                    break;
                case R.id.toolbar_skip:
                    HotspotActivity.this.finish();
                    break;
            }
        }
    };
}
