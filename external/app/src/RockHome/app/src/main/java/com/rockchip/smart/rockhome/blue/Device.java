package com.rockchip.smart.rockhome.blue;

import android.bluetooth.BluetoothDevice;
import android.os.SystemClock;

/**
 * Created by GJK on 2018/11/9.
 */

public class Device {
    public BluetoothDevice device;
    public int rssi;
    public Long last_scanned;

    public Device(BluetoothDevice device, int rssi) {
        this.device = device;
        this.rssi = rssi;
        this.last_scanned = SystemClock.currentThreadTimeMillis();
    }

    public BluetoothDevice getDevice() {
        return device;
    }

    public void setDevice(BluetoothDevice device) {
        this.device = device;
    }

    public int getRssi() {
        return rssi;
    }

    public void setRssi(int rssi) {
        this.rssi = rssi;
    }

    public Long getLast_scanned() {
        return last_scanned;
    }

    public void setLast_scanned(Long last_scanned) {
        this.last_scanned = last_scanned;
    }
}
