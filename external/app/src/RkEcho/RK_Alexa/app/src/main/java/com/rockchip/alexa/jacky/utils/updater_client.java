package com.rockchip.alexa.jacky.utils;

import android.util.Log;

import com.rockchip.alexa.jacky.activity.DeviceUpdateActivity;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.async.AsyncFactory;
import com.rockchip.alexa.jacky.control.WifiControl;
import com.rockchip.alexa.jacky.info.WifiInfo;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import static com.rockchip.alexa.jacky.async.AsyncFactory.getAsyncThread;

/**
 * Created by cjs on 2017/5/9.
 */

public class updater_client {
    private static final String TAG = "RK_Alexa";

    static{
        System.loadLibrary("updater_client");
    }

    public static native String getDeviceVersion();

    public static native int doUpdater();

    public static native void setImageDirectory(String imageDirectory);

    /*-------------------below java method is called by jni-------------------------*/

    public static short getUpdateImageBit(String versionNumberStr){
        Log.d(TAG, "Current device versionNumberStr: " + versionNumberStr);
        return UpdateManager.getUpdateImageBit(versionNumberStr);
    }

    /**
     * get update version string and put back to jni.
     * this method also used as a call-back when device update success
     * @return
     */
    public static String getUpdateVersionStr(){
        return UpdateManager.getUpdateVersionStr();
    }

    public static void onUpdateSuccess(){
         /* do some ui and data change when upgrade finished */
        BaseApplication.getApplication().getUpdateHandler().sendEmptyMessage(DeviceUpdateActivity.MESSAGE_DEVICE_UPDATE_SUCCESS);
    }

    public static void onUpdateFailed(){
        BaseApplication.getApplication().getUpdateHandler().sendEmptyMessage(DeviceUpdateActivity.MESSAGE_DEVICE_UPDATE_FAILED);
    }

    public static long getNewUpdateId(){
        Log.d("System.out", "getNewUpdateId: oldUpdateId: " + getOldUpdateId());
        // produce a new updateId for progress
        long updateId = Long.valueOf(new SimpleDateFormat("yyMMddhhmmssSSS").format(new Date()).toString()) * 10000;
        SharedPreference.putLong(UpdateManager.KEY_DOWNLOAD_UPDATE_ID,updateId);
        SharedPreference.putLong(UpdateManager.KEY_DOWNLOAD_UPDATE_ID_TIME,System.currentTimeMillis());
        return updateId;
    }

    public static long getOldUpdateId(){
        long currentTime = System.currentTimeMillis();
        long lastRecordTime = SharedPreference.getLong(UpdateManager.KEY_DOWNLOAD_UPDATE_ID_TIME,0);
        long timeSpan = currentTime - lastRecordTime;
        /*  if timeSpan > 6 hours,get a new update id */
        if(lastRecordTime != 0 && timeSpan > 21600000){
            return getNewUpdateId();
        }else{
            return SharedPreference.getLong(UpdateManager.KEY_DOWNLOAD_UPDATE_ID,0);
        }
    }

    private static String savedSsid = null;


    private static int retryTime = 0;

    /* ==================================== */
    private static int rebootState = 0;

    private final static int STATE_NULL = 0;
    private final static int STATE_WIFI_SAVED = 1;
    private final static int STATE_PREPARED_FOR_RECONNECT = 3;
    private final static int STATE_RECONNECTED = 4;

    private static int MAX_TRY_TIME = 100;


    /**
     * it will wait util device reboot to set into recovery mode.
     * it return 0 when mobile connect to the previous softAp provided by device.
     * and return retryTime when not success.
     */
    public static int waitDeviceReboot(){
        if(retryTime == 100){
            savedSsid = null;
            rebootState = STATE_NULL;
            retryTime = 0;
        }

        if(!Env.isWifiEnable(BaseApplication.getApplication().getApplicationContext())){
            return 0;
        }

        String connectedSsid = new WifiControl(BaseApplication.getApplication()).getConnectWifiSsid();
        if(connectedSsid.equals(savedSsid)
                && (rebootState == STATE_RECONNECTED||rebootState == STATE_PREPARED_FOR_RECONNECT)){
            savedSsid = null;
            rebootState = STATE_NULL;
            retryTime = 0;
            Log.d(TAG, "waitDeviceReboot: success.");
            BaseApplication.getApplication().getUpdateHandler().sendEmptyMessage(DeviceUpdateActivity.MESSAGE_DEVICE_UPDATING);
            return 0;
        }

        BaseApplication.getApplication().getUpdateHandler().sendEmptyMessage(DeviceUpdateActivity.MESSAGE_DEVICE_REBOOTING);

        switch (rebootState){
            case STATE_NULL:
                // it has no wifi connected.so it is error.
                if(connectedSsid.equals("")){
                    return MAX_TRY_TIME;
                }else if(savedSsid == null){
                    savedSsid = connectedSsid;
                    rebootState = STATE_WIFI_SAVED;
                    Log.d("System.out","STATE_WIFI_SAVED: " + savedSsid);
                }
                break;
            case STATE_WIFI_SAVED:
                if(connectedSsid.equals("")||!connectedSsid.equals(savedSsid)) {
                    Log.d("System.out", "STATE_PREPARED_FOR_RECONNECT.");
                    rebootState = STATE_PREPARED_FOR_RECONNECT ;
                }
                break;
        }


        if(rebootState == STATE_PREPARED_FOR_RECONNECT) {
            final List<WifiInfo> wifiList = getAsyncThread().scanDeviceHotspot();
            Log.d("System.out", "wifiList size: " + wifiList.size());
            for (WifiInfo info : wifiList) {
                Log.d("System.out", "wifi Name: " + info.getSsid());
                if (info.getSsid().equals(savedSsid)) {
                    int result = AsyncFactory.getAsyncThread().connect(savedSsid);
                    if(result == 1) {
                        Log.d("System.out", "STATE_RECONNECTED.");
                        rebootState = STATE_RECONNECTED;
                    } else{
                        Log.d("System.out", "STATE_RECONNECTED failed.");
                    }
                    break;
                }
            }
        }
        return (++retryTime);
    }

    public static void onVersionLatest(){
         /* do some ui and data change when version latest. */
        BaseApplication.getApplication().getUpdateHandler().sendEmptyMessage(DeviceUpdateActivity.MESSAGE_DEVICE_VERSION_LATEST);
    }

}
