package com.rockchip.smart.rockhome.fragment;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
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

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.rockchip.smart.rockhome.R;
import com.rockchip.smart.rockhome.Utils;
import com.rockchip.smart.rockhome.WifiListDialogFragment;
import com.rockchip.smart.rockhome.WifiListFragment;
import com.rockchip.smart.rockhome.blue.BluetoothDeviceAdapter;
import com.rockchip.smart.rockhome.blue.BluetoothGattManager;
import com.rockchip.smart.rockhome.blue.Device;
import com.rockchip.smart.rockhome.view.LoadingView;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by GJK on 2018/11/9.
 */

public class BlueFragment extends Fragment implements View.OnClickListener {
    private static final String TAG = BlueFragment.class.getSimpleName();
    private static ContentFragment mContentFragment;

    private Handler mHandler;

    // UI
    private LinearLayout mLlGuide;
    private LinearLayout mLlDeviceList;
    private LinearLayout mLlNoDevice;
    private LinearLayout mLlWifi;
    private ListView mLvDeviceList;
    private Button mBtLeScan;
    private TextView mTvLlWifiAddr;
    private TextView mTvLlWifiSsid;
    private EditText mEtLlWifiPwd;
    private ImageView mIvPwdLock;
    private ImageView mIvExpand;
    private TextView mBtLlWifiConfirm;
    private WifiListDialogFragment mWifiListDialogFragment;

    private BluetoothGatt mBluetoothGatt;
    private BluetoothGattManager mBluetoothGattManager;
    private BluetoothGattCallback mBluetoothGattCallback;

    private LoadingView mLoadingView;
    private boolean mShowPwd;
    private boolean mIsActivityVisible;

    private STATE mState = STATE.Layout_Guide;
    private enum STATE {
        Layout_Guide,
        Layout_NoDevice,
        Layout_DeviceList,
        Layout_Wifi;
    }

    private int mMtu = 20;

    public static BlueFragment newInstance(ContentFragment contentFragment) {
        mContentFragment = contentFragment;
        BlueFragment fragment = new BlueFragment();

        return fragment;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate...");

        mHandler = new Handler();
        mBluetoothGattManager = new BluetoothGattManager(getContext());
        mBluetoothGattManager.setBluetoothDeviceAdapterCallback(new BluetoothDeviceAdapter.BluetoothDeviceAdapterCallback() {
            @Override
            public void onLeScanStart() {
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mBtLeScan.setText(getActivity().getResources().getString(R.string.blue_stop_lescan));
                    }
                });
            }

            @Override
            public void onLeScan(Device device, int position) {
                Log.d(TAG, "onLeScan position:" + position);
            }

            @Override
            public void onLeScanStop(final int size) {
                Log.d(TAG, "Scaned devices " + size);
                if (!mIsActivityVisible)
                    return;

                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mBtLeScan.setText(getActivity().getResources().getString(R.string.blue_stop_lescan));
                        if (size == 0)
                            showState(STATE.Layout_NoDevice);
                    }
                });
            }

            @Override
            public void onBluetoothDeviceClick(Device device, int position) {
                Log.d(TAG, "onBluetoothDeviceClick name:" + device.getDevice().getName() + " - " + position);
                mLoadingView.show(getActivity().getResources().getString(R.string.dialog_connect_to) + device.getDevice().getAddress());
                mHandler.removeCallbacks(mLeScanRunnable);
                if (mBluetoothGattManager.isLeScanning())
                    mBluetoothGattManager.stopLeScan();
                mBluetoothGatt = mBluetoothGattManager.connectGatt(device.getDevice().getAddress(), mBluetoothGattCallback);
            }
        });
        mBluetoothGattCallback = new BluetoothGattCallback() {
            private StringBuilder builder = new StringBuilder();

            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                super.onConnectionStateChange(gatt, status, newState);
                boolean isOk = (status == 0);
                boolean isConnected = (newState == BluetoothAdapter.STATE_CONNECTED);

                Log.d(TAG, "onConnectionStateChange isOk:" + isOk + "; isConnected:" + isConnected);
                if (isOk && isConnected) {
                    isOk = gatt.discoverServices();
                }

                if (!isOk || !isConnected) {
                    gatt.close();
                    updateLeScanState(false);
                    showState(STATE.Layout_Guide);
                }
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                super.onServicesDiscovered(gatt, status);
                Log.d(TAG, "onServicesDiscovered status:" + status);
                if (status != 0) {
                    gatt.close();
                } else {
                    mBluetoothGattManager.setNotify(false);
                    mLoadingView.dismiss();
                    showState(STATE.Layout_Wifi);
                    gatt.requestMtu(134 + 3);
                }
            }

            @Override
            public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                super.onCharacteristicRead(gatt, characteristic, status);
                if (status == 0) {
                    try {
                        handleCharacteristicRead(gatt, characteristic);
                    } catch (Exception e) {
                        Log.d(TAG, Log.getStackTraceString(e));
                    }
                }
                mBluetoothGattManager.next();
            }

            private void handleCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) throws JSONException, UnsupportedEncodingException {

            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                super.onCharacteristicWrite(gatt, characteristic, status);
                if (status == 0) {
                    try {
                        handleCharacteristicWrite(gatt, characteristic);
                    } catch (Exception e) {

                    }
                }
                mBluetoothGattManager.next();
            }

            private void handleCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {

            }

            @Override
            public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
                super.onCharacteristicChanged(gatt, characteristic);
                try {
                    handleCharacteristicChanged(gatt, characteristic);
                } catch (JSONException | UnsupportedEncodingException e) {

                }
            }

            private void handleCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) throws JSONException, UnsupportedEncodingException {
                String value = characteristic.getStringValue(0);
                byte b[] = value.getBytes("utf-8");
                // 忽略APK 发送出去的数据
                if (b[0] == 0x01 && b[b.length - 1] == 0x04)
                    return;

                if (b.length == 1) {
                    if (b[0] == 0x01) {
                        if (mLoadingView.isShowing())
                            mLoadingView.dismiss();
                        showMessage(getActivity().getResources().getString(R.string.toast_config_success));
                        mBluetoothGatt.disconnect();
                    } else if (b[0] == 0x02) {
                        if (mLoadingView.isShowing())
                            mLoadingView.dismiss();
                        showMessage(getActivity().getResources().getString(R.string.toast_config_fail));
                    }
                    return;
                }

                builder.append(value);

                Log.d(TAG, "handleCharacteristicChanged mtu:" + mMtu + "; bytes:" + value.getBytes("utf-8").length);
                if (value.getBytes("utf-8").length < mMtu || isJSONValid(builder.toString())) {
                    Utils.saveFile("ble_wifilist.txt", builder.toString());
                    final List<com.rockchip.smart.rockhome.WifiInfo> wifis = new ArrayList<>();
                    JSONObject jsonRoot = (JSONObject) JSON.parse(builder.toString());
                    if (jsonRoot == null || jsonRoot.getString("cmd") == null)
                        return;

                    String cmd = jsonRoot.getString("cmd");
                    if (cmd.equals("wifilists")) {
                        JSONArray content = jsonRoot.getJSONArray("ret");
                        for (int i = 0, size = content.size(); i < size; i++) {
                            JSONObject object = content.getJSONObject(i);
                            com.rockchip.smart.rockhome.WifiInfo wifi = new com.rockchip.smart.rockhome.WifiInfo();
                            String ssid = object.getString("ssid");
                            if (ssid == null || ssid.isEmpty())
                                continue;

                            wifi.setSsid(object.getString("ssid"));
                            wifi.setSignalLevel(object.getString("rssi"));
                            wifi.setFlags(object.getString("flags"));
                            wifis.add(wifi);
                        }
                        builder = new StringBuilder();
                        mLoadingView.dismiss();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mWifiListDialogFragment.setWifiList(wifis);
                                mWifiListDialogFragment.show(getActivity().getFragmentManager(), "WifiList");
                            }
                        });
                    }
                }
            }

            @Override
            public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
                super.onDescriptorRead(gatt, descriptor, status);
                if (status == 0) {
                    try {
                        handleDescriptorRead(gatt, descriptor);
                    } catch (Exception e) {

                    }
                }
                mBluetoothGattManager.next();
            }

            private void handleDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor) {

            }

            @Override
            public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
                super.onDescriptorWrite(gatt, descriptor, status);
                if (status == 0) {
                    try {
                        handleDescriptorWrite(gatt, descriptor);
                    } catch (Exception e) {

                    }
                }
                mBluetoothGattManager.next();
            }

            private void handleDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor) {

            }

            @Override
            public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
                super.onMtuChanged(gatt, mtu, status);
                Log.d(TAG, "onMtuChanged mtu:" + mtu + "; status:" + status);
//                mBluetoothGattManager.notify(gatt);
                mMtu = mtu - 3;
            }
        };
        mLoadingView = new LoadingView(getActivity());
        mWifiListDialogFragment = WifiListDialogFragment.createDialog(new WifiListFragment.WifiCallback() {
            @Override
            public void onWifiPicked(com.rockchip.smart.rockhome.WifiInfo info) {
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

    private View mHeader;
    private View initHeader() {
        TextView tv = new TextView(getContext());
        tv.setText(getActivity().getResources().getString(R.string.devices_searched));
        tv.setTextColor(Color.BLACK);

        return (mHeader = tv);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView...");
        View rootView = inflater.inflate(R.layout.fragment_activity_rockhome_blue, container, false);
        mLlGuide = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_blue_ll_guide);
        mLlDeviceList = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_blue_ll_deviceList);
        mLvDeviceList = (ListView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_lv_deviceList);
        mLvDeviceList.setAdapter(mBluetoothGattManager.getBluetoothDeviceAdapter());
        mBtLeScan = (Button) rootView.findViewById(R.id.fragment_activity_rockhome_blue_bt_search);
        mBtLeScan.setOnClickListener(this);

        mLlNoDevice = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_blue_ll_nodevice);
        mLlWifi = (LinearLayout) rootView.findViewById(R.id.fragment_activity_rockhome_blue_ll_wifi);
        mTvLlWifiAddr = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_tv_addr);
        mTvLlWifiSsid = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_tv_input_ssid);
        mIvExpand = (ImageView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_iv_expand);
        mIvExpand.setOnClickListener(this);
        mIvPwdLock = (ImageView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_iv_pwd_lock);
        mIvPwdLock.setOnClickListener(this);
        mEtLlWifiPwd = (EditText) rootView.findViewById(R.id.fragment_activity_rockhome_blue_et_pwd);
        mBtLlWifiConfirm = (TextView) rootView.findViewById(R.id.fragment_activity_rockhome_blue_tv_confirm);
        mBtLlWifiConfirm.setOnClickListener(this);

        requestPermission(getActivity());

        return rootView;
    }

    @Override
    public void onStart() {
        super.onStart();
        mIsActivityVisible = true;
    }

    @Override
    public void onStop() {
        super.onStop();
        mIsActivityVisible = false;
        mHandler.removeCallbacks(mLeScanRunnable);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.fragment_activity_rockhome_blue_bt_search:
                handleLeScan();
                break;
            case R.id.fragment_activity_rockhome_blue_iv_expand:
                mLoadingView.show(getActivity().getResources().getString(R.string.dialog_getting_wifilist));
                mBluetoothGattManager.notify(mBluetoothGatt);
                mBluetoothGattManager.sendCmdWifiLists(mBluetoothGatt);
                break;
            case R.id.fragment_activity_rockhome_blue_tv_confirm:
                String ssid = mTvLlWifiSsid.getText().toString().trim();
                String pwd = mEtLlWifiPwd.getText().toString().trim();

                if (ssid == null || ssid.isEmpty() || ssid.equals(getString(R.string.no_wifi))) {
                    showMessage(getString(R.string.select_wifi));
                    return;
                } else if (!mContentFragment.verifyWifiPwd(pwd)) {
                    showMessage(getString(R.string.pwd_invalid));
                    return;
                }

                mLoadingView.show(getActivity().getResources().getString(R.string.dialog_wifi_configuring));
                Log.d(TAG, "ssid:" + ssid + "; pwd:" + pwd);
                mBluetoothGattManager.notify(mBluetoothGatt);
                mBluetoothGattManager.sendCmdWifiSetup(mBluetoothGatt, ssid, pwd);
                break;
            case R.id.fragment_activity_rockhome_blue_iv_pwd_lock:
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

    private Runnable mLeScanRunnable = new Runnable() {
        @Override
        public void run() {
            handleLeScan();
        }
    };
    private void handleLeScan() {
        if (mBluetoothGattManager.isLeScanning()) {
            mBluetoothGattManager.stopLeScan();
            updateLeScanState(false);
        } else {
            if (mBluetoothGattManager.startLeScan()) {
                mLvDeviceList.removeHeaderView(mHeader);
                initHeader();
                mLvDeviceList.addHeaderView(mHeader);
                showState(STATE.Layout_DeviceList);
                mHandler.postDelayed(mLeScanRunnable, 10 * 1000);
                updateLeScanState(true);
            }
        }
    }

    private void updateLeScanState(boolean scanning) {
        mBtLeScan.setText(getActivity().getResources().getString(scanning ? R.string.blue_stop_lescan : R.string.blue_start_lescan));
    }

    private void showState(final STATE state) {
        mState = state;
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (state) {
                    case Layout_Guide:
                        mBtLeScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.VISIBLE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.GONE);
                        break;
                    case Layout_NoDevice:
                        mBtLeScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.VISIBLE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.GONE);
                        mBtLeScan.setText(getActivity().getResources().getString(R.string.blue_start_lescan));
                        break;
                    case Layout_DeviceList:
                        mBtLeScan.setVisibility(View.VISIBLE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.VISIBLE);
                        mLlWifi.setVisibility(View.GONE);
                        break;
                    case Layout_Wifi:
                        mBtLeScan.setVisibility(View.GONE);
                        mLlGuide.setVisibility(View.GONE);
                        mLlNoDevice.setVisibility(View.GONE);
                        mLlDeviceList.setVisibility(View.GONE);
                        mLlWifi.setVisibility(View.VISIBLE);

                        mTvLlWifiAddr.setText(mBluetoothGatt.getDevice().getAddress());
                        initWifi();
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

    private void initWifi() {
        WifiManager wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        if (wifiManager.isWifiEnabled() && wifiInfo != null && !wifiInfo.getSSID().isEmpty()) {
            mTvLlWifiSsid.setText(removeQuote(wifiInfo.getSSID().toString()));
        } else {
            mTvLlWifiSsid.setText(getActivity().getResources().getString(R.string.no_wifi));
        }
        mEtLlWifiPwd.setText("");
    }

    private String removeQuote(String value) {
        if (!TextUtils.isEmpty(value)) {
            if (value.startsWith("\"")) {
                value = value.substring(1);
            }
            if (value.endsWith("\"")) {
                value = value.substring(0, value.length() - 1);
            }
            return value;
        }
        return value;
    }

    private void requestPermission(Activity activity) {
        if (ContextCompat.checkSelfPermission(activity, Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(activity,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION,
                            Manifest.permission.ACCESS_FINE_LOCATION},
                    1);
        }
    }

    private boolean isJSONValid(String str) {
        try {
            JSONObject.parseObject(str);
        } catch (JSONException ex) {
            try {
                JSONObject.parseArray(str);
            } catch (JSONException ex1) {
                return false;
            }
        }
        return true;
    }
}
