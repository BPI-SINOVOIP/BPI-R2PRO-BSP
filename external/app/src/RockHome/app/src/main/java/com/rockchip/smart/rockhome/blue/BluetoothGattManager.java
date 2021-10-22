package com.rockchip.smart.rockhome.blue;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.util.Log;

/**
 * Created by GJK on 2018/11/9.
 */

public class BluetoothGattManager {
    private static final String TAG = BluetoothGattManager.class.getSimpleName();

    private Context mContext;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothGattCallback mBluetoothGattCallback;
    private BluetoothAdapter.LeScanCallback mLeScanCallback;
    private BluetoothDeviceAdapter mBluetoothDeviceAdapter;
    private BluetoothDeviceAdapter.BluetoothDeviceAdapterCallback mBluetoothDeviceAdapterCallback;
    private final GattUtils.RequestQueue mRequestQueue = GattUtils.createRequestQueue();
    private boolean mIsLeScanning = false;

    public BluetoothGattManager(Context context) {
        this.mContext = context;
        BluetoothManager bluetoothManager = (BluetoothManager) context.getApplicationContext().getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            mBluetoothAdapter = bluetoothManager.getAdapter();
        }
        mBluetoothDeviceAdapter = new BluetoothDeviceAdapter(mContext);

        mBluetoothGattCallback = new BluetoothGattCallback() {
            @Override
            public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
                super.onConnectionStateChange(gatt, status, newState);
                boolean error = (status != 0);
                boolean isConnected = (newState == BluetoothAdapter.STATE_CONNECTED);

                Log.d(TAG, "onConnectionStateChange error:" + error + "; isConnected:" + isConnected);
                if (!error && isConnected) {
                    error = gatt.discoverServices();
                }

                if (error) {
                    gatt.close();
                }
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                super.onServicesDiscovered(gatt, status);
                Log.d(TAG, "onServicesDiscovered status:" + status);
                if (status != 0) {
                    gatt.close();
                } else {

                }
            }

            @Override
            public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                super.onCharacteristicRead(gatt, characteristic, status);
            }

            @Override
            public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
                super.onCharacteristicWrite(gatt, characteristic, status);
            }

            @Override
            public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
                super.onCharacteristicChanged(gatt, characteristic);
            }

            @Override
            public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
                super.onDescriptorRead(gatt, descriptor, status);
            }

            @Override
            public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
                super.onDescriptorWrite(gatt, descriptor, status);
            }
        };
    }

    public void setBluetoothDeviceAdapterCallback(BluetoothDeviceAdapter.BluetoothDeviceAdapterCallback bluetoothDeviceAdapterCallback) {
        mBluetoothDeviceAdapterCallback = bluetoothDeviceAdapterCallback;
        mBluetoothDeviceAdapter.setCallback(bluetoothDeviceAdapterCallback);
    }

    public BluetoothDeviceAdapter getBluetoothDeviceAdapter() {
        return mBluetoothDeviceAdapter;
    }

    public boolean startLeScan() {
        if (mBluetoothAdapter == null)
            return false;

        if (mLeScanCallback == null) {
            mLeScanCallback = new BluetoothAdapter.LeScanCallback() {
                @Override
                public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
                    mBluetoothDeviceAdapter.onLeScan(device, rssi, scanRecord);
                }
            };
        }

        mBluetoothDeviceAdapter.getDevices().clear();
        boolean res = mBluetoothAdapter.startLeScan(/*new UUID[]{Constants.WIFI_SERVICE_UUID}, */mLeScanCallback);

        if (res) {
            mIsLeScanning = true;
            if (mBluetoothDeviceAdapterCallback != null)
                mBluetoothDeviceAdapterCallback.onLeScanStart();
        }

        return res;
    }

    public boolean stopLeScan() {
        if (mBluetoothAdapter == null)
            return false;

        mBluetoothAdapter.stopLeScan(mLeScanCallback);
        mIsLeScanning = false;
        if (mBluetoothDeviceAdapterCallback != null)
            mBluetoothDeviceAdapterCallback.onLeScanStop(mBluetoothDeviceAdapter.getCount());
        return true;
    }

    public boolean isLeScanning() {
        return mIsLeScanning;
    }

    public BluetoothGatt connectGatt(String address, BluetoothGattCallback callback) {
        BluetoothDevice bluetoothDevice = mBluetoothAdapter.getRemoteDevice(address);
        if (bluetoothDevice == null)
            return null;

        return bluetoothDevice.connectGatt(mContext,false, callback);
    }

    private static boolean mNotified = false;

    public void setNotify(boolean notify) {
        mNotified = notify;
    }

    public void notify(BluetoothGatt gatt) {
        if (mNotified)
            return;

        mNotified = true;
        BluetoothGattCharacteristic characteristic = null;
        characteristic = GattUtils.getCharacteristic(gatt, Constants.WIFI_SERVICE_UUID, Constants.WIFI_CHARACTERISTIC_UUID);
        if (characteristic == null) {
            Log.d(TAG, "BluetoothGattManager notify get characteristic failed.");
            return;
        }

        BluetoothGattDescriptor descriptor = null;
        descriptor = GattUtils.getDescriptor(gatt,
                Constants.WIFI_SERVICE_UUID,
                Constants.WIFI_CHARACTERISTIC_UUID,
                Constants.CLIENT_CONFIG_DESCRIPTOR_UUID);
        if(descriptor != null) {
            gatt.setCharacteristicNotification(characteristic, true);
            BluetoothGattDescriptor descriptor1 = new BluetoothGattDescriptor(descriptor.getUuid(), descriptor.getPermissions());
            descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
//        descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
            mRequestQueue.addWriteDescriptor(gatt, descriptor, BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
            //具有NOTIFY|INDICATE 属性，根据属性设置相应的值，这里先默认设置为ENABLE_NOTIFICATION_VALUE, tiantian
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
//        descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
            mRequestQueue.addWriteDescriptor(gatt, descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);

            mRequestQueue.execute();
        }
    }

    public void sendCmdWifiLists(BluetoothGatt gatt) {
        BluetoothGattCharacteristic characteristic = null;

        characteristic = GattUtils.getCharacteristic(gatt, Constants.WIFI_SERVICE_UUID, Constants.WIFI_CHARACTERISTIC_UUID);

        byte b[] = new byte[14];
        String str = "wifilists";
        b[0] = 0x01;
        for (int i = 0, j = 1; i < 12; i++, j++) {
            if (i <str.getBytes().length) {
                b[j] = str.getBytes()[i];
            } else {
                b[j] = 0x00;
            }
        }
        b[13] = 0x04;

        characteristic.setValue(b);
        mRequestQueue.addWriteCharacteristic(gatt, characteristic);

        mRequestQueue.execute();
    }

    public void sendCmdWifiSetup(BluetoothGatt gatt, String ssid, String pwd) {
        sendCmdWifiSetup(gatt, ssid, pwd, null);
    }

    private void sendCmdWifiSetup(BluetoothGatt gatt, String ssid, String pwd, String userdata) {
        BluetoothGattCharacteristic characteristic = null;

        characteristic = GattUtils.getCharacteristic(gatt, Constants.WIFI_SERVICE_UUID, Constants.WIFI_CHARACTERISTIC_UUID);

        byte b[] = new byte[78];
        String str = "wifisetup";
        b[0] = 0x01;

        for (int i = 0, j = 1; i < 12; i++, j++) {
            if (i <str.getBytes().length) {
                b[j] = str.getBytes()[i];
            } else {
                b[j] = 0x00;
            }
        }

        for (int i = 0, j = 13; i < 32; i++, j++) {
            if (i < ssid.getBytes().length) {
                b[j] = ssid.getBytes()[i];
            } else {
                b[j] = 0x00;
            }
        }

        for (int i = 0, j = 45; i < 32; i++, j++) {
            if (i < pwd.getBytes().length) {
                b[j] = pwd.getBytes()[i];
            } else {
                b[j] = 0x00;
            }
        }
        b[77] = 0x04;

        StringBuilder builder = new StringBuilder();
        builder.append("{");
        for (int i = 0; i < b.length; i++) {
            builder.append(b[i]).append(",");
        }
        builder.append("}");
        Log.d(TAG, "sendCmdWifiSetup:\"" + builder.toString() + "\"");

        characteristic.setValue(b);
        mRequestQueue.addWriteCharacteristic(gatt, characteristic);

        mRequestQueue.execute();
    }

    private Object lock = new Object();
    public void next() {
        synchronized (lock) {
            mRequestQueue.next();
        }
    }
}
