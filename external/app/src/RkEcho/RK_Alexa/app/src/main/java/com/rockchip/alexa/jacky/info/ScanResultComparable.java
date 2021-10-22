package com.rockchip.alexa.jacky.info;

import android.net.wifi.ScanResult;

/**
 * Created by Administrator on 2017/3/14.
 */
public class ScanResultComparable  implements Comparable<ScanResultComparable>  {
    private ScanResult mScanResult;
    private boolean mIsConnecting;
    private boolean mIsConnected;

    public ScanResultComparable (ScanResult scanResult) {
        this.mScanResult = scanResult;
    }

    public ScanResult getScanResult() {
        return mScanResult;
    }

    public void setScanResult(ScanResult scanResult) {
        this.mScanResult = scanResult;
    }

    public boolean isConnecting() {
        return mIsConnecting;
    }

    public void setConnecting(boolean isConnecting) {
        this.mIsConnecting = isConnecting;
    }

    public boolean isConnected() {
        return mIsConnected;
    }

    public void setConnected(boolean isConnected) {
        this.mIsConnected = isConnected;
    }

    @Override
    public int compareTo(ScanResultComparable scanResultComparable) {
        return mScanResult.level == scanResultComparable.mScanResult.level ? 0 : (mScanResult.level > scanResultComparable.mScanResult.level ? -1 : 1);
    }
}
