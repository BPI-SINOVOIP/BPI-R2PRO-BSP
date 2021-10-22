package com.rockchip.alexa.jacky.async;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.control.WifiControl;
import com.rockchip.alexa.jacky.info.WifiInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Administrator on 2017/4/27.
 */

public class AsyncThread {

    private AsyncHandler mAsyncHandler;
    private WifiControl mWifiControl;
    private List<WifiInfo> mWifiList;
    private int mConnetedResult;

    private static final int MSG_SCAN_WIFI = 0x000000001;
    private static final int MSG_CONNECT_WIFI = 0x00000002;

    public AsyncThread() {
        HandlerThread ht = new HandlerThread("async");
        ht.start();
        mAsyncHandler = new AsyncHandler(ht.getLooper());
        mWifiControl = new WifiControl(BaseApplication.getApplication());
        mWifiList = new ArrayList<WifiInfo>();
    }

    private class AsyncHandler extends Handler {
        public AsyncHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SCAN_WIFI:
                    mWifiList = mWifiControl.startScan();
                    break;
                case MSG_CONNECT_WIFI:
                    Bundle bundle = (Bundle) msg.obj;
                    mConnetedResult = mWifiControl.connect(bundle.getString("SSID"), bundle.getString("PWD"), bundle.getInt("TYPE"));
                    break;
            }
        }

        /**
         * Waits for all the {@code Message} and {@code Runnable} currently in the queue
         * are processed.
         *
         * @return {@code false} if the wait was interrupted, {@code true} otherwise.
         */
        public boolean waitDone() {
            final Object waitDoneLock = new Object();
            final Runnable unlockRunnable = new Runnable() {
                @Override
                public void run() {
                    synchronized (waitDoneLock) {
                        waitDoneLock.notifyAll();
                    }
                }
            };

            synchronized (waitDoneLock) {
                mAsyncHandler.post(unlockRunnable);
                try {
                    waitDoneLock.wait();
                } catch (InterruptedException ex) {
                    return false;
                }
            }
            return true;
        }
    }

    public List<WifiInfo> scanWifi() {
        mWifiList.clear();
        mAsyncHandler.obtainMessage(MSG_SCAN_WIFI).sendToTarget();
        mAsyncHandler.waitDone();
        Log.d("AlexaActivity", "scan Wifi end. size:" + mWifiList.size());
        return mWifiList;
    }

    public List<WifiInfo> scanDeviceHotspot() {
        Log.d("AlexaActivity", "scanDeviceHotspot");
        List<WifiInfo> list = new ArrayList<>();
        mWifiList = scanWifi();
        for (WifiInfo info : mWifiList) {
            if (info.getSsid().trim().startsWith(Context.HOTSPOT_PREFIX)) {
                if (info.getFlags().contains("WEP") || info.getFlags().contains("PSK") || info.getFlags().contains("EAP"))
                    continue;
                list.add(info);
            }
        }
        return list;
    }

    public int connect(String ssid, String pwd, int type) {
        Bundle bundle = new Bundle();
        bundle.putString("SSID", ssid);
        bundle.putString("PWD", pwd);
        bundle.putInt("TYPE", type);
        mAsyncHandler.obtainMessage(MSG_CONNECT_WIFI, bundle).sendToTarget();
        mAsyncHandler.waitDone();
        return mConnetedResult;
    }

    public int connect(String ssid) {
        return connect(ssid, "", 0);
    }
}
