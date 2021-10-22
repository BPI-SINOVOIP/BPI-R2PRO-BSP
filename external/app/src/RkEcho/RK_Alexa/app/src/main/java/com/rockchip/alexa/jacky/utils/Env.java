package com.rockchip.alexa.jacky.utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

import static android.content.Context.WIFI_SERVICE;

/**
 * Created by Administrator on 2017/3/22.
 */

public class Env {

    public static enum WifiEncType {
        WEP, WPA, OPEN
    }

    public static boolean isWifiEnable(Context context) {
        return getWifiManager(context).isWifiEnabled();
    }

    public static boolean isWifiConnected(Context context) {
        NetworkInfo wifiNetworkInfo = getConnectivityManager(context).getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        return wifiNetworkInfo.isConnected();
    }

    public static boolean openWifi(Context context) {
        WifiManager manager = getWifiManager(context);
        if (manager.isWifiEnabled()) {
            return true;
        } else {
            return manager.setWifiEnabled(true);
        }
    }

    public static boolean closeWifi(Context context) {
        WifiManager manager = getWifiManager(context);
        if (!manager.isWifiEnabled()) {
            return true;
        } else {
            return manager.setWifiEnabled(false);
        }
    }

    public static String getConnectingSSID(Context context) {
        WifiInfo wifiInfo = getWifiManager(context).getConnectionInfo();
        String ssid = wifiInfo.getSSID();
        return ssid.substring(1, ssid.length() - 1);
    }

    public static void connectWifi(Context context, String targetSsid, String targetPsd, WifiEncType enc) {
        String ssid = "\"" + targetSsid + "\"";
        String psd = "\"" + targetPsd + "\"";

        WifiConfiguration conf = new WifiConfiguration();
        conf.SSID = ssid;
        switch (enc) {
            case WEP:
                conf.wepKeys[0] = psd;
                conf.wepTxKeyIndex = 0;
                conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
                break;
            case WPA:
                conf.preSharedKey = psd;
                break;
            case OPEN:
                conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        }

        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        int netId = wifiManager.addNetwork(conf);
        wifiManager.disconnect();
        wifiManager.enableNetwork(netId, true);
        wifiManager.reconnect();
    }

    public static void connectWifi(Context context, String ssid) {
        connectWifi(context, ssid, "", WifiEncType.OPEN);
    }

    private static WifiManager getWifiManager(Context context) {
        return (WifiManager) context.getSystemService(WIFI_SERVICE);
    }

    private static ConnectivityManager getConnectivityManager(Context context) {
        return (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
    }
}
