package com.rockchip.alexa.jacky.control;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.util.Log;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static android.content.Context.WIFI_SERVICE;

public class WifiControl {
    private static String TAG = "WifiControl";
    // 定义WifiManager对象
    private android.net.wifi.WifiManager mWifiManager;
    // 定义WifiInfo对象
    private WifiInfo mWifiInfo;
    // 扫描出的网络连接列表
    private List<com.rockchip.alexa.jacky.info.WifiInfo> mWifiList;
    // 网络连接列表
    private List<WifiConfiguration> mWifiConfiguration;
    // 定义一个WifiLock
    private WifiLock mWifiLock;

    private Context mContext;

    // 构造器
    public WifiControl(Context context) {
        mContext = context;
        // 取得WifiManager对象
        mWifiManager = (android.net.wifi.WifiManager) context.getSystemService(WIFI_SERVICE);
        mWifiConfiguration = new ArrayList<>();
    }

    // 打开WIFI
    public void openWifi(Context context) {
        if (!mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(true);
        }else if (mWifiManager.getWifiState() == 2) {
            Toast.makeText(context, context.getString(R.string.wifi_opening), Toast.LENGTH_SHORT).show();
        }else{
            Toast.makeText(context, context.getString(R.string.wifi_opened), Toast.LENGTH_SHORT).show();
        }
    }

    // 关闭wifi
    public void closeWifi(Context context) {
        if (mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(false);
        }else if(mWifiManager.getWifiState() == 1){
            Toast.makeText(context, context.getString(R.string.wifi_closed), Toast.LENGTH_SHORT).show();
        }else if (mWifiManager.getWifiState() == 0) {
            Toast.makeText(context, context.getString(R.string.wifi_closing), Toast.LENGTH_SHORT).show();
        }else{
            Toast.makeText(context, context.getString(R.string.wifi_clos_again), Toast.LENGTH_SHORT).show();
        }
    }

    // 检查当前WIFI状态
    public void checkState(Context context) {
        if (mWifiManager.getWifiState() == 0) {
            Toast.makeText(context, context.getString(R.string.wifi_closed), Toast.LENGTH_SHORT).show();
        } else if (mWifiManager.getWifiState() == 1) {
            Toast.makeText(context, context.getString(R.string.wifi_closing), Toast.LENGTH_SHORT).show();
        } else if (mWifiManager.getWifiState() == 2) {
            Toast.makeText(context, context.getString(R.string.wifi_opening), Toast.LENGTH_SHORT).show();
        } else if (mWifiManager.getWifiState() == 3) {
            Toast.makeText(context, context.getString(R.string.wifi_opened), Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(context, context.getString(R.string.wifi_state_not_get), Toast.LENGTH_SHORT).show();
        }
    }

    // 锁定WifiLock
    public void acquireWifiLock() {
        mWifiLock.acquire();
    }

    // 解锁WifiLock
    public void releaseWifiLock() {
        // 判断时候锁定
        if (mWifiLock.isHeld()) {
            mWifiLock.acquire();
        }
    }

    // 创建一个WifiLock
    public void creatWifiLock() {
        mWifiLock = mWifiManager.createWifiLock("Test");
    }

    public List<com.rockchip.alexa.jacky.info.WifiInfo> startScan() {
        mWifiManager.startScan();
        //得到扫描结果
        List<ScanResult> results = mWifiManager.getScanResults();
//        // 得到配置好的网络连接
//        mWifiConfiguration = mWifiManager.getConfiguredNetworks();
        if (results == null) {
            if(mWifiManager.getWifiState()==3){
                Toast.makeText(mContext, mContext.getString(R.string.wifi_not_scaned_this_region),Toast.LENGTH_SHORT).show();
            }else if(mWifiManager.getWifiState()==2){
                Toast.makeText(mContext, mContext.getString(R.string.wifi_opening_wait), Toast.LENGTH_SHORT).show();
            }else{
                Toast.makeText(mContext, mContext.getString(R.string.wifi_not_opened), Toast.LENGTH_SHORT).show();
            }
        } else {
            mWifiList = new ArrayList();
            boolean isConnected = isWifiConnected();
            String connectedSsid = "";
            if (isConnected) {
                connectedSsid = getConnectWifiSsid();
                Log.d("WifiControl", "已连接WIFI， ssid:" + connectedSsid);
            }
            for(ScanResult result : results){
                if (result.SSID == null || result.SSID.length() == 0 || result.capabilities.contains("[IBSS]")) {
                    continue;
                }
                com.rockchip.alexa.jacky.info.WifiInfo info = new com.rockchip.alexa.jacky.info.WifiInfo();
                info.setConnecting(false);
                info.setSsid(result.SSID);
                info.setSignalLevel("" + result.level);
                info.setFrequency("" + result.frequency);
                info.setBssid("" + result.BSSID);
                info.setConnected(result.SSID.equals(connectedSsid));
                info.setFlags(result.capabilities.toUpperCase());
                mWifiList.add(info);
            }
            Collections.sort(mWifiList);
            if (isConnected) {
                for (com.rockchip.alexa.jacky.info.WifiInfo comparable : mWifiList) {
                    if (comparable.getSsid().equals(connectedSsid)) {
                        Log.d("WifiControl", "找到已连接WIFI");
                        mWifiList.remove(comparable);
                        comparable.setConnecting(true);
                        mWifiList.add(0, comparable);
                        break;
                    }
                }
            }
        }
        return mWifiList;
    }

    // 得到网络列表
    public List<com.rockchip.alexa.jacky.info.WifiInfo> getWifiList() {
        return mWifiList;
    }

    // 查看扫描结果
    public StringBuilder lookUpScan() {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i = 0; i < mWifiList.size(); i++) {
            stringBuilder.append("Index_" + new Integer(i + 1).toString() + ":");
            // 将ScanResult信息转换成一个字符串包
            // 其中把包括：BSSID、SSID、capabilities、frequency、level
            stringBuilder.append((mWifiList.get(i)).toString());
            stringBuilder.append("/n");
        }
        return stringBuilder;
    }

    // 添加一个网络并连接
    public void addNetwork(final WifiConfiguration wcg) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "addNetwork start");
                int wcgID = mWifiManager.addNetwork(wcg);
                Log.d(TAG, "addNetwork wcgId:" + wcgID);
                boolean b =  mWifiManager.enableNetwork(wcgID, true);
                Log.d(TAG, "addNetwork end, result:" + b);
            }
        }).start();
    }

    // 断开指定ID的网络
    public void disconnectWifi(int netId) {
        mWifiManager.disableNetwork(netId);
        mWifiManager.disconnect();
    }
    public void removeWifi(int netId) {
        disconnectWifi(netId);
        mWifiManager.removeNetwork(netId);
    }

    //创建wifi热点的
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

    public int connect(String ssid, String pwd, int type) {
        Log.d(TAG, "connect. ssid:" + ssid + "; pwd:" + pwd + "; type:" + type);
        int netId = -1;
        mWifiConfiguration.clear();
        mWifiConfiguration.addAll(mWifiManager.getConfiguredNetworks());

        for (WifiConfiguration configuration : mWifiConfiguration) {
            String configId = configuration.SSID.replaceAll("\"", "");
            if (configId.equals(ssid)) {
                netId = configuration.networkId;
                break;
            }
        }
        if (netId != -1) {//已经连接配置过
            mWifiManager.disconnect();
            mWifiManager.enableNetwork(netId, true);
        } else {
            WifiConfiguration wifiConfig = createWifiInfo(ssid, pwd, type);
            netId = mWifiManager.addNetwork(wifiConfig);
            if (netId != -1) {
                mWifiManager.saveConfiguration();
            }
            mWifiManager.enableNetwork(netId, true);
        }

        int time = 0;
        for (;;) {
            if (isWifiConnected()) {
                Log.d(TAG, "Wifi Connected:" + getConnectWifiSsid());
                return getConnectWifiSsid().equals(ssid) ? 1 : -1;
            }
            try {
                if (time > 150) {
                    break;
                }
                time++;
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        return 0;
    }

    private WifiConfiguration IsExsits(String SSID) {
        List<WifiConfiguration> existingConfigs = mWifiManager.getConfiguredNetworks();
        Log.d(TAG, "IsExsits:  getConfiguredNetworks == null? " + ((existingConfigs==null)?"yes":"false"));
        if(existingConfigs == null || existingConfigs.size() == 0){
            return null;
        }
        for (WifiConfiguration existingConfig : existingConfigs) {
            if (existingConfig.SSID.equals("\""+SSID+"\"")) {
                return existingConfig;
            }
        }
        return null;
    }

    public boolean isWifiConnected() {
        ConnectivityManager connectivityManager = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo wifiNetworkInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if(wifiNetworkInfo.isConnected()) {
            return true ;
        }
        return false ;
    }

    public String getConnectWifiSsid(){
        if (!isWifiConnected())
            return "";
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        return wifiInfo.getSSID().substring(1, wifiInfo.getSSID().length() - 1);
    }
}
