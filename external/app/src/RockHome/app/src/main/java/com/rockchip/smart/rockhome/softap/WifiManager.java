package com.rockchip.smart.rockhome.softap;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.MacAddress;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkSpecifier;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiNetworkSpecifier;
import android.os.Build;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.util.Log;

import com.rockchip.smart.rockhome.WifiInfo;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by GJK on 2018/11/14.
 */

public class WifiManager {
    private final String TAG = "WifiManager";
    private Context mContext;
    private android.net.wifi.WifiManager mWifiManager;
    private ConnectivityManager mConnectivityManager;
    private android.net.wifi.WifiManager.WifiLock mWifiLock;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private boolean mIsRequestNetwork = false;

    private static volatile WifiManager mInstance;

    public static WifiManager getInstance(Context context) {
        if (mInstance == null) {
            synchronized (WifiManager.class) {
                if (mInstance == null) {
                    mInstance = new WifiManager(context);
                }
            }
        }
        return mInstance;
    }

    private WifiManager(Context context) {
        mContext = context;
        mWifiManager = (android.net.wifi.WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mConnectivityManager = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        mWifiLock = mWifiManager.createWifiLock("Rock Home");
        mWifiReceiver = new WifiReceiver();

        mNetworkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                // do success processing here..
                Log.d(TAG, "onAvailable");
            }

            @Override
            public void onUnavailable() {
                // do failure processing here..
                Log.d(TAG, "onUnavailable");
            }

            @Override
            public void onCapabilitiesChanged(@NonNull Network network,
                                              @NonNull NetworkCapabilities networkCapabilities) {

                Log.d(TAG, "onCapabilitiesChanged");
                if(networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET))
                    Log.d(TAG, "NET_CAPABILITY_INTERNET");

                if(networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED))
                    Log.d(TAG, "NET_CAPABILITY_VALIDATED");
            }
        };
    }

    public void unregisterNetworkCallback() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q && mIsRequestNetwork) {
            Log.d(TAG, "unregisterNetworkCallback");
            mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
            mIsRequestNetwork = false;
        }
    }

    public void acquireWifiLock() {
        if (!mWifiLock.isHeld())
            mWifiLock.acquire();
    }

    public void releaseWifiLock() {
        if (mWifiLock.isHeld())
            mWifiLock.release();
    }

    public boolean isWifiEnabled() {
        return mWifiManager.isWifiEnabled();
    }

    public boolean setWifiEnabled(boolean enable) {
        return mWifiManager.setWifiEnabled(enable);
    }

    public boolean isWifiConnected() {
        NetworkInfo wifiNetworkInfo = mConnectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if(wifiNetworkInfo.isConnected()) {
            return true ;
        }
        return false ;
    }

    public String getConnectWifiSsid(){
        android.net.wifi.WifiManager wifiManager = (android.net.wifi.WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        android.net.wifi.WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        return removeQuote(wifiInfo.getSSID());
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

    private Data getType(WifiInfo info) {
        if (info == null || info.getFlags() == null || info.getFlags().isEmpty()) {
            return null;
        }
        String flags = info.getFlags();

        if (flags.contains("WPA2")) {
            return Data.WIFI_CIPHER_WPA2;
        } else if (flags.contains("WPA")) {
            return Data.WIFI_CIPHER_WPA;
        } else if (flags.contains("WPS")) {
            return Data.WIFI_CIPHER_WEP;
        } else {
            return Data.WIFI_CIPHER_NOPASS;
        }
    }

    private int getConnectionNetworkId() {
        if (!isWifiConnected())
            return -1;

        return mWifiManager.getConnectionInfo().getNetworkId();
    }

    private List<ScanResult> scan() {
        mWifiManager.startScan();
        return mWifiManager.getScanResults();
    }

    private List<WifiInfo> transformResults(List<ScanResult> results) {
        List<WifiInfo> list = new ArrayList<>();
        if (results != null) {
            boolean isConnected = isWifiConnected();
            String connectedSsid = "";
            if (isConnected) {
                connectedSsid = getConnectWifiSsid();
            }
            for (ScanResult result : results) {
                if(result.SSID != null)
                    Log.d(TAG, "ScanResult: ssid = " + result.SSID.trim() + ", capabilities = " + result.capabilities);

                if (result.SSID == null || result.SSID.length() == 0 || result.capabilities.contains("[IBSS]")
                        || !result.SSID.trim().startsWith("Rockchip-SoftAp")) {
                    continue;
                }
                WifiInfo info = new WifiInfo();
                info.setConnecting(false);
                info.setSsid(result.SSID);
                info.setSignalLevel("" + result.level);
                info.setFrequency("" + result.frequency);
                info.setBssid("" + result.BSSID);
                info.setConnected(result.SSID.equals(connectedSsid));
                info.setFlags(result.capabilities.toUpperCase());

                list.add(info);
            }
            Collections.sort(list);
        }

        return list;
    }

    public List<WifiInfo> scanWifi() {
        return transformResults(scan());
    }

    public boolean connCustomNetWorkQ(String ssid, String bssid, Data type) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            NetworkSpecifier specifier = new WifiNetworkSpecifier.Builder()
                            .setSsid(ssid)
                            .setBssid(MacAddress.fromString(bssid))
                            .build();

            NetworkRequest request = new NetworkRequest.Builder()
                            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                            .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                            .setNetworkSpecifier(specifier)
                            .build();

            mConnectivityManager.requestNetwork(request, mNetworkCallback);
            mIsRequestNetwork = true;
        }

        return true;
    }

    public boolean connCustomNetWork(String ssid, String pwd, Data type) {
        int netId = -1;
        List<WifiConfiguration> wifiConfigurationList = mWifiManager.getConfiguredNetworks();
        for (WifiConfiguration configuration : wifiConfigurationList) {
            String configId = configuration.SSID.replaceAll("\"", "");
            if (configId.equals(ssid)) {
                netId = configuration.networkId;
                break;
            }
        }

        if (netId != -1) {
            boolean res = mWifiManager.removeNetwork(netId);
            Log.d(TAG, "Wifi had Configuration, remove first. " + netId + "; res:" + res);
            if (!res) {
                return false;
            }
        }

        WifiConfiguration configuration = createWifiConfig(ssid, pwd, type);
        netId = mWifiManager.addNetwork(configuration);
        if (netId == -1) {
            Log.d(TAG, "Add configuration failed...");
            return false;
        }

        Method connectMethod = connectWifiByReflectMethod(netId);
        if (connectMethod == null) {
            Log.d(TAG, "connectWifiByReflectMethod failed...");
            return mWifiManager.enableNetwork(netId, true);
        }
        return  true;
    }

    public boolean connCustomNetWorkBySdk(String ssid, String pwd, String bssid, Data type) {
        Log.d(TAG, "Build.VERSION.SDK_INT: " + Build.VERSION.SDK_INT);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q)
            return connCustomNetWorkQ(ssid, bssid, type);
        else
            return connCustomNetWork(ssid, pwd, type);
    }

    public void connToAp(String ssid) {
        Log.d(TAG, "connToAp ssid:" + ssid);
        connCustomNetWorkBySdk(ssid, "", "", Data.WIFI_CIPHER_NOPASS);
    }

    /**
     * 断开某一个WiFi网络
     */
    public void disconnectNetWork() {
        if (isWifiConnected()) {
            mWifiManager.disableNetwork(getConnectionNetworkId());
            mWifiManager.removeNetwork(getConnectionNetworkId());
            mWifiManager.disconnect();
        }
    }

    /**
     * 创建WifiConfiguration
     * 三个安全性的排序为：WEP < WPA < WPA2。
     * WEP是Wired Equivalent Privacy的简称，有线等效保密（WEP）协议是对在两台设备间无线传输的数据进行加密的方式，
     * 用以防止非法用户窃听或侵入无线网络
     * WPA全名为Wi-Fi Protected Access，有WPA和WPA2两个标准，是一种保护无线电脑网络（Wi-Fi）安全的系统，
     * 它是应研究者在前一代的系统有线等效加密（WEP）中找到的几个严重的弱点而产生的
     * WPA是用来替代WEP的。WPA继承了WEP的基本原理而又弥补了WEP的缺点：WPA加强了生成加密密钥的算法，
     * 因此即便收集到分组信息并对其进行解析，也几乎无法计算出通用密钥；WPA中还增加了防止数据中途被篡改的功能和认证功能
     * WPA2是WPA的增强型版本，与WPA相比，WPA2新增了支持AES的加密方式
     *
     * @param SSID
     * @param password
     * @param type
     * @return
     **/
    private WifiConfiguration createWifiConfig(String SSID, String password, Data type) {
        Log.d(TAG, "SSID = " + SSID + "; password = " + password + "; type = " + type);
        WifiConfiguration config = new WifiConfiguration();

        config.allowedAuthAlgorithms.clear();
        config.allowedGroupCiphers.clear();
        config.allowedKeyManagement.clear();
        config.allowedPairwiseCiphers.clear();
        config.allowedProtocols.clear();
        config.SSID = "\"" + SSID + "\"";

        if (type == Data.WIFI_CIPHER_NOPASS) {
//            config.wepKeys[0] = "\"" + password + "\"";
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
//            config.wepTxKeyIndex = 0;
        } else if (type == Data.WIFI_CIPHER_WEP) {
            config.hiddenSSID = true;
            config.wepKeys[0] = "\"" + password + "\"";
            config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.SHARED);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            config.wepTxKeyIndex = 0;
        } else if (type == Data.WIFI_CIPHER_WPA) {
            config.preSharedKey = "\"" + password + "\"";
            config.hiddenSSID = true;
            config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
            config.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
            config.status = WifiConfiguration.Status.ENABLED;
        } else if (type == Data.WIFI_CIPHER_WPA2) {
            config.preSharedKey = "\"" + password + "\"";
            config.hiddenSSID = true;
            config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
            config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
            config.status = WifiConfiguration.Status.ENABLED;
        }

        return config;
    }

    /**
     * 密码加密类型
     */
    public enum Data {
        WIFI_CIPHER_NOPASS(0), WIFI_CIPHER_WEP(1), WIFI_CIPHER_WPA(2), WIFI_CIPHER_WPA2(3);

        private final int value;

        //构造器默认也只能是private, 从而保证构造函数只能在内部使用
        Data(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * 通过反射出不同版本的connect方法来连接Wifi
     *
     * @author jiangping.li
     * @param netId
     * @return
     * @since MT 1.0
     *
     */
    private Method connectWifiByReflectMethod(int netId) {
        Method connectMethod = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            // 反射方法： connect(int, listener) , 4.2 <= phone's android version
            Log.d(TAG, "1111");
            for (Method methodSub : mWifiManager.getClass().getDeclaredMethods()) {
                if ("connect".equalsIgnoreCase(methodSub.getName())) {
                    Class<?>[] types = methodSub.getParameterTypes();
                    if (types != null && types.length > 0) {
                        if ("int".equalsIgnoreCase(types[0].getName())) {
                            connectMethod = methodSub;
                        }
                    }
                }
            }
            if (connectMethod != null) {
                try {
                    Log.d(TAG, "2222");
                    connectMethod.invoke(mWifiManager, netId, null);
                } catch (Exception e) {
                    Log.d(TAG, "3333" + Log.getStackTraceString(e));
                    e.printStackTrace();
                    return null;
                }
            }
        } else if (Build.VERSION.SDK_INT == Build.VERSION_CODES.JELLY_BEAN) {
            // 反射方法: connect(Channel c, int networkId, ActionListener listener)
            // 暂时不处理4.1的情况 , 4.1 == phone's android version
            return null;
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH
                && Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            // 反射方法：connectNetwork(int networkId) ,
            // 4.0 <= phone's android version < 4.1
            for (Method methodSub : mWifiManager.getClass()
                    .getDeclaredMethods()) {
                if ("connectNetwork".equalsIgnoreCase(methodSub.getName())) {
                    Class<?>[] types = methodSub.getParameterTypes();
                    if (types != null && types.length > 0) {
                        if ("int".equalsIgnoreCase(types[0].getName())) {
                            connectMethod = methodSub;
                        }
                    }
                }
            }
            if (connectMethod != null) {
                try {
                    connectMethod.invoke(mWifiManager, netId);
                } catch (Exception e) {
                    e.printStackTrace();
                    return null;
                }
            }
        } else {
            // < android 4.0
            return null;
        }
        return connectMethod;
    }


    private static final int WIFI_SEARCH_TIMEOUT = 20; //扫描WIFI的超时时间
    private WifiReceiver mWifiReceiver;
    private Lock mLock;
    private Condition mCondition;
    private Lock mLock1;
    private Condition mCondition1;
    private SearchWifiListener mSearchWifiListener;
    private WifiConnectListener mWifiConnectListener;
    private boolean mIsWifiScanCompleted = false;
    private boolean mIsWifiConnectCompleted = false;
    private boolean mIsWifiReceiverRegisted = false;

    private void registerWifiReceiver(IntentFilter filter) {
        if (mIsWifiReceiverRegisted)
            unregisterWifiReceiver();

        mIsWifiReceiverRegisted = true;
        mContext.registerReceiver(mWifiReceiver, filter);
    }

    private void unregisterWifiReceiver() {
        if (!mIsWifiReceiverRegisted)
            return;

        mIsWifiReceiverRegisted = false;
        mContext.unregisterReceiver(mWifiReceiver);
    }

    public static enum ErrorType {
        SEARCH_WIFI_TIMEOUT, //扫描WIFI超时（一直搜不到结果）
        NO_WIFI_FOUND,       //扫描WIFI结束，没有找到任何WIFI信号
    }

    //扫描结果通过该接口返回给Caller
    public interface SearchWifiListener {
        void onScanStart();
        void onScanFailed(ErrorType errorType);
        void onScanSuccess(List<WifiInfo> results);
    }

    public interface WifiConnectListener {
        void onConnectStart(WifiInfo info);
        void onConnectFailed(WifiInfo info, int error);
        void onConnectSuccess(WifiInfo info, int ip);
    };

    private class WifiReceiver extends BroadcastReceiver {

        private WifiInfo info;

        public WifiReceiver(WifiInfo info) {
            this.info = info;
        }

        public WifiReceiver() {

        }

        public void setInfo(WifiInfo info) {
            this.info = info;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            //Log.d(TAG, "onReceive: " + intent.getAction());
            switch (intent.getAction()) {
                case android.net.wifi.WifiManager.WIFI_STATE_CHANGED_ACTION:
                    handleWifiStateChanged(intent);
                    break;
                case android.net.wifi.WifiManager.SCAN_RESULTS_AVAILABLE_ACTION:
                    handleScanResultsAvailable(intent);
                    break;
                case android.net.wifi.WifiManager.NETWORK_STATE_CHANGED_ACTION:
                    handleNetworkStateChanged(intent);
                    break;
            }
        }

        private void handleWifiStateChanged(Intent intent) {
            int wifiState = intent.getIntExtra(android.net.wifi.WifiManager.EXTRA_WIFI_STATE, 0);
            switch (wifiState) {
                case android.net.wifi.WifiManager.WIFI_STATE_ENABLED:
                    break;
                case android.net.wifi.WifiManager.WIFI_STATE_ENABLING:
                    break;
                case android.net.wifi.WifiManager.WIFI_STATE_DISABLED:
                    break;
                case android.net.wifi.WifiManager.WIFI_STATE_DISABLING:
                    break;
                case android.net.wifi.WifiManager.WIFI_STATE_UNKNOWN:
                    break;
            }
        }

        private void handleScanResultsAvailable(Intent intent) {
            List<String> ssidResults = new ArrayList<String>();
            List<ScanResult> scanResults = mWifiManager.getScanResults();
            for(ScanResult result : scanResults ) {
                ssidResults.add(result.SSID);
            }

            //检测扫描结果
            if( ssidResults.isEmpty() ) {
                if (mSearchWifiListener != null)
                    mSearchWifiListener.onScanFailed(ErrorType.NO_WIFI_FOUND);
            } else {
                if (mSearchWifiListener != null) {
                    mSearchWifiListener.onScanSuccess(transformResults(scanResults));
                }
            }

            mLock.lock();
            mIsWifiScanCompleted = true;
            mCondition.signalAll();
            mLock.unlock();
        }

        private void handleNetworkStateChanged(Intent intent) {
            Log.d(TAG, "handleNetworkStateChanged...");
            NetworkInfo networkInfo = intent.getParcelableExtra(android.net.wifi.WifiManager.EXTRA_NETWORK_INFO);
            if(null != networkInfo)
                Log.d(TAG, "networkInfo state: " + networkInfo.getState());

            if (null != networkInfo && networkInfo.isConnected()) {
                android.net.wifi.WifiInfo wifiInfo = null;

                wifiInfo = intent.getParcelableExtra(android.net.wifi.WifiManager.EXTRA_WIFI_INFO);
                if(wifiInfo == null)
                    wifiInfo = mWifiManager.getConnectionInfo(); //for android 9 and 9+

                if (wifiInfo != null && info != null) {
                    if (wifiInfo.getBSSID().equals(info.getBssid()) && !TextUtils.isEmpty(getConnectWifiSsid())) {
                        if (mWifiConnectListener != null) {
                            mWifiConnectListener.onConnectSuccess(info, wifiInfo.getIpAddress());
                        }

                        mLock1.lock();
                        mIsWifiConnectCompleted = true;
                        mCondition1.signalAll();
                        mLock1.unlock();
                    }
                }
            }
        }
    }

    public void connect(final WifiInfo info, final String pwd, WifiConnectListener connectListener) {
        mWifiConnectListener = connectListener;

        if (mLock1 == null) {
            mLock1 = new ReentrantLock();
            mCondition1 = mLock1.newCondition();
        }
        mWifiReceiver.setInfo(info);

        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mWifiConnectListener != null) {
                    mWifiConnectListener.onConnectStart(info);
                }

                //Log.d(TAG, "isWifiConnected(): " + isWifiConnected());
                //Log.d(TAG, "SSID: " + mWifiManager.getConnectionInfo().getSSID());
                Log.d(TAG, "BSSID: " + mWifiManager.getConnectionInfo().getBSSID() + "; Bssid: " + info.getBssid());
                if (isWifiConnected() && mWifiManager.getConnectionInfo().getBSSID().equals(info.getBssid())) {
                    if (mWifiConnectListener != null) {
                        mWifiConnectListener.onConnectSuccess(info, mWifiManager.getConnectionInfo().getIpAddress());
                    }
                    return;
                }

                Log.d(TAG, "connect registerReceiver " + mWifiReceiver.isOrderedBroadcast());
                registerWifiReceiver(new IntentFilter(android.net.wifi.WifiManager.NETWORK_STATE_CHANGED_ACTION));

                if (!connCustomNetWorkBySdk(info.getSsid(), pwd, info.getBssid(), getType(info))) {
                    unregisterWifiReceiver();
                    if (mWifiConnectListener != null) {
                        mWifiConnectListener.onConnectFailed(info, -1);
                    }
                    return;
                }

                mLock1.lock();
                try {
                    mIsWifiConnectCompleted = false;
                    mCondition1.await(WIFI_SEARCH_TIMEOUT, TimeUnit.SECONDS);
                    if( !mIsWifiConnectCompleted && mWifiConnectListener != null) {
                        mWifiConnectListener.onConnectFailed(info, -2);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                mLock1.unlock();

                //删除注册的监听类对象
                Log.d(TAG, "connect unregisterReceiver");
                unregisterWifiReceiver();
            }
        }).start();
    }

    public void search(String reg, SearchWifiListener listener) {
        mSearchWifiListener = listener;
        if (mLock == null) {
            mLock = new ReentrantLock();
            mCondition = mLock.newCondition();
        }

        new Thread(new Runnable() {

            @Override
            public void run() {
                //注册接收WIFI扫描结果的监听类对象
                Log.d(TAG, "search registerReceiver");
                registerWifiReceiver(new IntentFilter(android.net.wifi.WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));

                //开始扫描
                if (mSearchWifiListener != null)
                    mSearchWifiListener.onScanStart();

                mWifiManager.startScan();

                mLock.lock();

                //阻塞等待扫描结果
                try {
                    mIsWifiScanCompleted = false;
                    mCondition.await(WIFI_SEARCH_TIMEOUT, TimeUnit.SECONDS);
                    if( !mIsWifiScanCompleted && mSearchWifiListener != null) {
                        mSearchWifiListener.onScanFailed(ErrorType.SEARCH_WIFI_TIMEOUT);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                mLock.unlock();

                //删除注册的监听类对象
                Log.d(TAG, "search unregisterReceiver");
                unregisterWifiReceiver();
            }
        }).start();
    }
}
