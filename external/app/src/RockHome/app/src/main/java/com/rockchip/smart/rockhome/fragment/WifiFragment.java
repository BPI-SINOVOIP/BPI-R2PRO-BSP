package com.rockchip.smart.rockhome.fragment;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.smart.rockhome.R;
import com.rockchip.smart.rockhome.WifiListDialogFragment;
import com.rockchip.smart.rockhome.WifiListFragment;
import com.rockchip.smart.rockhome.WifiInfo;
import com.rockchip.smart.rockhome.softap.Constants;
import com.rockchip.smart.rockhome.softap.SoftapDeviceAdapter;
import com.rockchip.smart.rockhome.softap.WifiManager;
import com.rockchip.smart.rockhome.softap.http.ProvisioningClient;
import com.rockchip.smart.rockhome.view.LoadingView;

import org.json.JSONException;

import java.io.IOException;
import java.util.List;

/**
 * Created by GJK on 2018/11/9.
 */

public class WifiFragment extends Fragment implements View.OnClickListener {
    private static final String TAG = WifiFragment.class.getSimpleName();
    private static ContentFragment mContentFragment;

    private static final int PERMISSION_REQUEST_CODE = 0;
    private static final String[] NEEDED_PERMISSIONS = new String[]{
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_NETWORK_STATE,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.CHANGE_NETWORK_STATE,
            Manifest.permission.CHANGE_WIFI_MULTICAST_STATE,
            Manifest.permission.CHANGE_WIFI_STATE,
    };
    private boolean mHasPermission;

    // UI
    private LinearLayout mLlGuide;
    private LinearLayout mLlDeviceList;
    private LinearLayout mLlNoDevice;
    private LinearLayout mLlWifi;
    private TextView mTvLlWifiSoftApName;
    private TextView mTvLlWifiSsid;
    private EditText mEtLlWifiPwd;
    private TextView mBtLlWifiConfirm;
    private Button mBtSoftApScan;
    private ListView mLvDeviceList;
    private ImageView mIvPwdLock;
    private ImageView mIvExpand;
    private SoftapDeviceAdapter mSoftapDeviceAdapter;
    private LoadingView mLoadingView;

    private WifiManager mWifiManager;
    private ProvisioningClient mClient;

    private WifiListDialogFragment mWifiListDialogFragment;

    private boolean mShowPwd;

    private STATE mState = STATE.Layout_Guide;
    private enum STATE {
        Layout_Guide,
        Layout_NoDevice,
        Layout_DeviceList,
        Layout_Wifi;
    }

    public static WifiFragment newInstance(ContentFragment contentFragment) {
        mContentFragment = contentFragment;
        WifiFragment fragment = new WifiFragment();

        return fragment;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mWifiManager = WifiManager.getInstance(getActivity());
        mSoftapDeviceAdapter = new SoftapDeviceAdapter(getActivity(), new SoftapDeviceAdapter.SoftapDeviceAdapterCallback() {
            @Override
            public void onItemClick(WifiInfo info) {
                mWifiManager.connect(info, "", new WifiManager.WifiConnectListener() {
                    @Override
                    public void onConnectStart(WifiInfo info) {
                        Log.d(TAG, "正在连接到'" + info.getSsid() + "'");
                        mLoadingView.show("正在连接到'" + info.getSsid() + "'");
                    }

                    @Override
                    public void onConnectFailed(WifiInfo info, int error) {
                        mLoadingView.dismiss();
                        Log.d(TAG, "onConnectFailed '" + info.getSsid() + "'" + ", error:" + error);
                        if (error == -1) {
                            showMessage(getActivity().getResources().getString(R.string.need_remove_network_first));
                        } else {
                            showMessage(getActivity().getResources().getString(R.string.connect_wifi_fail));
                        }
                    }

                    @Override
                    public void onConnectSuccess(WifiInfo info, int ip) {
                        Log.d(TAG, "onConnectSuccess '" + info.getSsid() + "'");
                        mLoadingView.dismiss();
                        showState(STATE.Layout_Wifi);
                        showMessage(getActivity().getResources().getString(R.string.connect_wifi_success));
                    }
                });
            }
        });
        mLoadingView = new LoadingView(getActivity());
        mClient = ProvisioningClient.getProvisioningClient(getActivity());
        mWifiListDialogFragment = WifiListDialogFragment.createDialog(new WifiListFragment.WifiCallback() {
            @Override
            public void onWifiPicked(WifiInfo info) {
                Log.d(TAG, "onWifiPicked " + info.getSsid());
                mWifiListDialogFragment.dismiss();
                mTvLlWifiSsid.setText(info.getSsid());
            }

            @Override
            public void onWifiPickCancelled() {
                Log.d(TAG, "onWifiPickCancelled");
            }
        });
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_activity_rockhome_wifi, container, false);
        mBtSoftApScan = (Button) rootView.findViewById(R.id.search_devices);
        mBtSoftApScan.setOnClickListener(this);

        mLlGuide = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_ll_guide);

        mLlDeviceList = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_ll_deviceList);
        mLvDeviceList = (ListView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_lv_deviceList);
        mLvDeviceList.setAdapter(mSoftapDeviceAdapter);

        mLlNoDevice = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_ll_nodevice);

        mLlWifi = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_ll_wifi);
        mTvLlWifiSoftApName = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_tv_ap);
        mTvLlWifiSsid = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_tv_input_ssid);
        mTvLlWifiSsid.setOnClickListener(this);
        mEtLlWifiPwd = (EditText) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_et_pwd);
        mIvExpand = (ImageView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_iv_expand);
        mIvExpand.setOnClickListener(this);
        mIvPwdLock = (ImageView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_iv_pwd_lock);
        mIvPwdLock.setOnClickListener(this);
        mBtLlWifiConfirm = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_wifi_tv_confirm);
        mBtLlWifiConfirm.setOnClickListener(this);

        mHasPermission = checkPermission();
        if (!mHasPermission) {
            requestPermission();
        }
        return rootView;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.search_devices:
                mWifiManager.search(Constants.SOFTAP_REGEX, new WifiManager.SearchWifiListener() {
                    @Override
                    public void onScanStart() {
                        Log.d(TAG, "onScanStart...");
                        mLoadingView.show("正在扫描附近设备");
                    }

                    @Override
                    public void onScanFailed(WifiManager.ErrorType errorType) {
                        Log.d(TAG, "onScanFailed..." + errorType);
                        mLoadingView.dismiss();
                        switch (errorType) {
                            case NO_WIFI_FOUND:
                                break;
                            case SEARCH_WIFI_TIMEOUT:
                                break;
                        }
                    }

                    @Override
                    public void onScanSuccess(List<WifiInfo> results) {
                        Log.d(TAG, "onScanSuccess..." + results.size());
                        for (WifiInfo info : results) {
                            Log.d(TAG, info.toString());
                        }
                        mLoadingView.dismiss();
                        if (results.isEmpty()) {
                            showState(STATE.Layout_NoDevice);
                        } else {
                            mSoftapDeviceAdapter.setDevices(results);
                            mLvDeviceList.removeHeaderView(mHeader);
                            initHeader();
                            mLvDeviceList.addHeaderView(mHeader);
                            showState(STATE.Layout_DeviceList);
                        }
                    }
                });
                break;
            case R.id.fragment_activity_rockhome_wifi_iv_expand:
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            mLoadingView.show(getActivity().getResources().getString(R.string.dialog_getting_wifilist));
                            List<WifiInfo> list = mClient.getWifiListInfo();
                            int times = 0;
                            while (list.isEmpty() && times < 3) {
                                times++;
                                list = mClient.getWifiListInfo();
                            }

                            mWifiListDialogFragment.setWifiList(list);

                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mWifiListDialogFragment.show(getActivity().getFragmentManager(), "WifiList");
                                }
                            });
                        } catch (JSONException | IOException e) {
                            Log.d(TAG, Log.getStackTraceString(e));
                        } finally {
                            mLoadingView.dismiss();
                        }
                    }
                }).start();
                break;

            case R.id.fragment_activity_rockhome_wifi_tv_confirm:
                final String ssid = mTvLlWifiSsid.getText().toString().trim();
                final String pwd = mEtLlWifiPwd.getText().toString().trim();
                if (ssid == null || ssid.isEmpty() || ssid.equals(getString(R.string.no_wifi))) {
                    showMessage(getString(R.string.select_wifi));
                    return;
                } else if (!mContentFragment.verifyWifiPwd(pwd)) {
                    showMessage(getString(R.string.pwd_invalid));
                    return;
                }
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            mLoadingView.show(getActivity().getResources().getString(R.string.dialog_wifi_configuring));
                            boolean result = mClient.postWifiSetupInfo(ssid, pwd);
                            Thread.sleep(3000);
                            if (result) {
                                for (int i=0; i<20; i++) {
                                    result = mClient.checkWifiState();

                                    if (result) {
                                        showMessage(getActivity().getResources().getString(R.string.connect_wifi_success));
                                        mClient.postConnectResult(true);
                                        showState(STATE.Layout_Guide);
                                        mWifiManager.unregisterNetworkCallback();
                                        break;
                                    } else {
                                        Thread.sleep(3000);
                                    }
                                }
                                if (!result) {
                                    showMessage(getActivity().getResources().getString(R.string.connect_wifi_fail));
                                    mClient.postConnectResult(false);
                                }
                            } else {
                                showMessage(getActivity().getResources().getString(R.string.send_msg_error));
                            }
                        } catch (Exception e) {
                            Log.d(TAG, Log.getStackTraceString(e));
                        } finally {
                            mLoadingView.dismiss();
                        }
                    }
                }).start();
                break;

            case R.id.fragment_activity_rockhome_wifi_iv_pwd_lock:
                mShowPwd = !mShowPwd;
                if (mShowPwd) {
                    mIvPwdLock.setImageResource(R.drawable.password_input_02);
                    mEtLlWifiPwd.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                } else {
                    mIvPwdLock.setImageResource(R.drawable.password_input_01);
                    mEtLlWifiPwd.setTransformationMethod(PasswordTransformationMethod.getInstance());
                }
                break;
        }
    }

    private View mHeader;
    private View initHeader() {
        TextView tv = new TextView(getContext());
        tv.setText(getActivity().getResources().getString(R.string.devices_searched));
        tv.setTextColor(Color.BLACK);

        return (mHeader = tv);
    }

    private void showState(final STATE state) {
        mState = state;
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (state) {
                    case Layout_Guide:
                        mBtSoftApScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.VISIBLE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.GONE);
                        break;
                    case Layout_NoDevice:
                        mBtSoftApScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.VISIBLE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.GONE);
                        break;
                    case Layout_DeviceList:
                        mBtSoftApScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.VISIBLE);
                        mLlWifi.setVisibility(View.GONE);
                        break;
                    case Layout_Wifi:
                        mBtSoftApScan.setVisibility(View.GONE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.VISIBLE);

                        Log.d(TAG, "setText wifiSSid:" + mWifiManager.getConnectWifiSsid());
                        mTvLlWifiSoftApName.setText(mWifiManager.getConnectWifiSsid());
                        break;
                }
            }
        });
    }

    private void showMessage(final String msg) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(getActivity(), msg, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private boolean checkPermission() {
        for (String permission : NEEDED_PERMISSIONS) {
            if (ActivityCompat.checkSelfPermission(getActivity(), permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private void requestPermission() {
        ActivityCompat.requestPermissions(getActivity(),
                NEEDED_PERMISSIONS, PERMISSION_REQUEST_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
