package com.rockchip.alexa.jacky.control;

import android.app.AlertDialog;
import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;

import java.util.ArrayList;
import java.util.List;

import static android.content.Context.WIFI_SERVICE;

/**
 * Created by GJK on 2018/4/16.
 */

public class WifiControl1 {
    private  Context mContext;
    private WifiManager wifiManager;
    private List<WifiConfiguration> wifiConfigList;

    // 构造器
    public WifiControl1(Context context) {
        mContext = context;
        // 取得WifiManager对象
        wifiManager = (android.net.wifi.WifiManager) context.getSystemService(WIFI_SERVICE);

        wifiConfigList = new ArrayList<>();
    }

    /**
     * 连接Wifi
     */
    public void connectWifi(final String ssid, final int wifiType) {
        int networkId = -1;
        wifiConfigList.clear();
        wifiConfigList.addAll(wifiManager.getConfiguredNetworks());
        for (WifiConfiguration configuration : wifiConfigList) {
            String configId = configuration.SSID.replaceAll("\"", "");
            if (configId.equals(ssid)) {
                networkId = configuration.networkId;
                break;
            }
        }
        if (networkId != -1) {//已经连接配置过
            wifiManager.disconnect();
            wifiManager.enableNetwork(networkId, true);
        } else {//新的连接
            WifiConfiguration wifiConfig = createWifiInfo(ssid, "", wifiType);
            int netId = wifiManager.addNetwork(wifiConfig);
            if (netId != -1) {
                wifiManager.saveConfiguration();
            }
            boolean flag = wifiManager.enableNetwork(netId, true);
        }
    }

    public WifiConfiguration createWifiInfo(String SSID, String password, int wifiType) {
        //清空config
        WifiConfiguration config = new WifiConfiguration();
        config.allowedAuthAlgorithms.clear();
        config.allowedGroupCiphers.clear();
        config.allowedKeyManagement.clear();
        config.allowedPairwiseCiphers.clear();
        config.allowedProtocols.clear();
        config.SSID = "\"" + SSID + "\""; //wifi名称

        if (wifiType == 0) {
            config.wepKeys[0] = "";
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            config.wepTxKeyIndex = 0;
        }
        if (wifiType == 1) {
            config.hiddenSSID = false;
            config.wepKeys[0] = "\"" + password + "\"";//密码
            config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.SHARED);
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.IEEE8021X);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
            config.status = WifiConfiguration.Status.ENABLED;
        }
        if (wifiType == 2) {
            config.hiddenSSID = false;
            config.preSharedKey = "\"" + password + "\"";
            config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.NONE);
            config.allowedProtocols.set(WifiConfiguration.Protocol.WPA); // For WPA
            config.allowedProtocols.set(WifiConfiguration.Protocol.RSN); // For WPA2
            config.status = WifiConfiguration.Status.ENABLED;
        }
        return config;
    }
}
