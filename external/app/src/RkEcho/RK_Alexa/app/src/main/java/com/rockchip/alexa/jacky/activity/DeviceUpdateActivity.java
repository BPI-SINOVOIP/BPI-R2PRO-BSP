package com.rockchip.alexa.jacky.activity;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.widget.EditText;
import android.widget.Toast;

import com.coolerfall.download.DownloadCallback;
import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.utils.FileUtil;
import com.rockchip.alexa.jacky.utils.HttpClientUtil;
import com.rockchip.alexa.jacky.utils.HttpDownloader;
import com.rockchip.alexa.jacky.utils.NetUtils;
import com.rockchip.alexa.jacky.utils.SharedPreference;
import com.rockchip.alexa.jacky.utils.UpdateManager;
import com.rockchip.alexa.jacky.utils.updater_client;
import com.rockchip.alexa.jacky.views.CustomDialog;
import com.rockchip.alexa.jacky.views.WakeLockProgressDialog;

import java.io.IOException;
import java.io.InputStream;

/**
 * Created by cjs on 2017/5/8.
 */

public class DeviceUpdateActivity extends BasePreferenceActivity {
    private static final String TAG = "RK_Alexa";

    private static boolean firstComeIn = true;

    public static final int MESSAGE_UPDATE_PREFERENCE      = 2;
    public static final int MESSAGE_REQUEST_FAILED         = 3;

    public static final int MESSAGE_DEVICE_DATA_PREPARED   = 7;
    // device update success and failed
    public static final int MESSAGE_DEVICE_UPDATE_SUCCESS  = 8;
    public static final int MESSAGE_DEVICE_UPDATE_FAILED   = 9;
    public static final int MESSAGE_DEVICE_REBOOTING       = 10;
    public static final int MESSAGE_DEVICE_UPDATING        = 11;
    public static final int MESSAGE_DEVICE_VERSION_LATEST  = 12;


    private static UpdateManager mUpdateManager;
    /**
     * httpDownload tool used for download firmware.
     */
    private HttpDownloader mDownloader;

    /*
     * progress dialog is for firmware download progress
     */
    private WakeLockProgressDialog mLoadingDialog;
    private WakeLockProgressDialog mProgressDialog;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MESSAGE_UPDATE_PREFERENCE:
                    mLoadingDialog.dismiss();
                    refreshPreference(mUpdateManager.isUpdatable());
                    break;
                case MESSAGE_REQUEST_FAILED:
                    if(!NetUtils.isOpenNetwork(mContext)){
                        showToastOnUiThread(getString(R.string.request_error_for_network),Toast.LENGTH_SHORT);
                    }else{
                        showToastOnUiThread(getString(R.string.request_error_for_server),Toast.LENGTH_SHORT);
                    }
                    mLoadingDialog.dismiss();
                    break;
                case MESSAGE_DEVICE_DATA_PREPARED:
                    mLoadingDialog.dismiss();
                    /* new firmware so produce a new update id */
                    mUpdateManager.resetUpdateId();
                    sendDeviceData();
                    break;
                case MESSAGE_DEVICE_UPDATE_SUCCESS:
                    showToastOnUiThread(getString(R.string.device_update_success),Toast.LENGTH_SHORT);
                    mLoadingDialog.dismiss();
                    // saved sharePreference value
                    SharedPreference.putString(mContext,UpdateManager.KEY_DEVICE_VERSION_STR,UpdateManager.getUpdateVersionStr());
                    SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
                    refreshPreference(mUpdateManager.isUpdatable());
                    break;
                case MESSAGE_DEVICE_UPDATE_FAILED:
                    showToastOnUiThread(getString(R.string.device_update_failed),Toast.LENGTH_SHORT);
                    mLoadingDialog.dismiss();
                    break;
                case MESSAGE_DEVICE_REBOOTING:
                    onDeviceRebooting();
                    break;
                case MESSAGE_DEVICE_UPDATING:
                    onDeviceUpdating();
                    break;
                case MESSAGE_DEVICE_VERSION_LATEST:
                    onDeviceVersionLatest();
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "DeviceUpdateActivity onCreate.");
        addPreferencesFromResource(R.xml.pref_screen_device_update);
        initData();
        initUi();
    }

    private void initData(){
        BaseApplication.getApplication().setUpdateHandler(mHandler);
        if(mUpdateManager == null){
            mUpdateManager = new UpdateManager(this);
        }
        mDownloader = new HttpDownloader(this);
    }

    private void initUi(){
        mLoadingDialog = new WakeLockProgressDialog(this);
        mLoadingDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mLoadingDialog.setIndeterminate(false);
        mLoadingDialog.setCancelable(false);

        mProgressDialog = new WakeLockProgressDialog(this);
        mProgressDialog.setMessage(getString(R.string.firmware_downloading));
        mProgressDialog.setCancelable(false);
        mProgressDialog.setProgressNumberFormat("%1d MB/%2d MB");
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        mProgressDialog.setButton(DialogInterface.BUTTON_POSITIVE,getString(R.string.firmware_download_backstage),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mProgressDialog.dismiss();
                    }
                });
        mProgressDialog.setButton(DialogInterface.BUTTON_NEGATIVE, getString(R.string.firmware_download_cancel),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
                        mDownloader.cancelTask();
                    }
                });

        if(firstComeIn){
            refreshPreference(false);
            onCheckVersionClick();
        }else{
            refreshPreference(mUpdateManager.isUpdatable());
        }
    }

    /**
     * refresh preference state by the param below
     * 1.whether is updatable 2.the firmware download State
     * @param isUpdatable
     */
    private void refreshPreference(boolean isUpdatable){
        PreferenceCategory netUpdateCategory = (PreferenceCategory)getPreferenceScreen().findPreference(getString(R.string.pref_category_net_update));
        if(prefDownload == null){
            prefCheckVersion = netUpdateCategory.findPreference(getString(R.string.pref_update_item_check_version));
            prefDownload = netUpdateCategory.findPreference(getString(R.string.pref_update_item_download_update));
        }
        if(isUpdatable){
            netUpdateCategory.addPreference(prefDownload);
            prefCheckVersion.setSummary(R.string.message_new_version_on);
            int downloadState = SharedPreference.getInt(this,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
            Log.d(TAG, "refreshPreference: downloadState: " + downloadState);
            if(downloadState == UpdateManager.DOWNLOAD_STATE_DOWNLOADED
                    || downloadState == UpdateManager.DOWNLOAD_STATE_DATA_PREPARED){
                prefDownload.setSummary(getString(R.string.message_new_version_has_downloaded));
            }else{
                prefDownload.setSummary("");
            }
        }else{
            netUpdateCategory.removePreference(prefDownload);
            prefCheckVersion.setSummary("");
        }
    }


    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference){
        if(preference.getKey().equals(getString(R.string.pref_update_item_check_version))){
            onCheckVersionClick();
            return true;
        }else if(preference.getKey().equals(getString(R.string.pref_update_item_download_update))){
            int downloadState = SharedPreference.getInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
            switch (downloadState){
                case UpdateManager.DOWNLOAD_STATE_DOWNLOADING:
                    mProgressDialog.show();
                    break;
                case UpdateManager.DOWNLOAD_STATE_DOWNLOADED:
                    prepareDeviceData();
                    break;
                case UpdateManager.DOWNLOAD_STATE_DATA_PREPARED:
                    sendDeviceData();
                    break;
                case UpdateManager.DOWNLOAD_STATE_NONE:
                    /*=====================================================*/
                    if(mUpdateManager.isFirmwareReady()){
                        Log.d("System.out", "onPreferenceTreeClick: FirmwareReady and reset updateId");
                        mUpdateManager.resetUpdateId();
                        SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DOWNLOADED);
                        refreshPreference(mUpdateManager.isUpdatable());
//                        prepareDeviceData();
                        return true;
                    }
                    /*=====================================================*/
                    CustomDialog.Builder builder = new CustomDialog.Builder(DeviceUpdateActivity.this);
                    builder.setMessage(mUpdateManager.getServerVersionUpdateInfo());
                    builder.setTitle(getString(R.string.str_version_upgrade_info));
                    builder.setPositiveButton(getResources().getString(R.string.dialog_button_confirm), new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
//                            RxPermissions.getInstance(DeviceUpdateActivity.this)
//                                    .request(Manifest.permission.WRITE_EXTERNAL_STORAGE);
                            mUpdateManager.initPathFileBeforeDownload();
                            mDownloader.startDownload(Context.HTTP_IP_ADDRESS + Context.HTTP_REQUEST_FIRMWARE
                                    ,UpdateManager.FIRMWARE_PATH,UpdateManager.FIRMWARE_NAME,new DownloadListener());
                            dialog.dismiss();
                            mLoadingDialog.setMessage(getString(R.string.connecting_to_server));
                            mLoadingDialog.show();
                        }
                    });

                    builder.setNegativeButton(getResources().getString(R.string.dialog_button_cancel),
                            new android.content.DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.dismiss();
                                }
                            });
                    builder.create().show();
                    break;
            }
            return true;
        }else if(preference.getKey().equals(getString(R.string.pref_update_item_set_ip))){
            final EditText inputServer = new EditText(this);
            inputServer.setText("http://");
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(getString(R.string.server_ip_set)).setIcon(android.R.drawable.ic_dialog_info).setView(inputServer)
                    .setNegativeButton(getString(R.string.dialog_button_cancel), null);
            builder.setPositiveButton(getString(R.string.dialog_button_confirm), new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface dialog, int which) {
                    Context.HTTP_IP_ADDRESS = inputServer.getText().toString();
                    Toast.makeText(DeviceUpdateActivity.this,getString(R.string.server_ip_set_success) + Context.HTTP_IP_ADDRESS,Toast.LENGTH_SHORT).show();
                }
            });
            builder.show();
            return true;
        }
        return false;
    }

    public void onCheckVersionClick(){
        mLoadingDialog.setMessage(getString(R.string.message_get_new_version_ing));
        mLoadingDialog.show();
        HttpClientUtil.requestGet(Context.HTTP_IP_ADDRESS + Context.HTTP_REQUEST_NEW_VERSION
                , new HttpClientUtil.RequestStateListener() {
                    @Override
                    public void onRequestResultReceived(InputStream is) {
                        String serverVersionInfo = HttpClientUtil.streamToString(is);
                        mUpdateManager.handlerServerString(serverVersionInfo);
                        mUpdateManager.reInitDownloadStateByCurrent();
                        if(!firstComeIn) {
                            showToastOnUiThread(mUpdateManager.isUpdatable() ? getString(R.string.message_new_version_on2) :
                                    getString(R.string.message_newest_version_on), Toast.LENGTH_SHORT);
                        }
                        firstComeIn = false;
                        mHandler.sendEmptyMessage(MESSAGE_UPDATE_PREFERENCE);
                    }

                    @Override
                    public void onRequestFail(String failString) {
                        Log.d(TAG, "Request failed with failString: "+ failString);
                        mHandler.sendEmptyMessage(MESSAGE_REQUEST_FAILED);
                    }
                });
    }

    /**
     * prepare the data that send to device
     * when firmware version download complete
     */
    private void prepareDeviceData(){
        mLoadingDialog.setMessage(getString(R.string.firmware_data_handle_ing));
        mLoadingDialog.show();
        new Thread(new Runnable() {
            @Override
            public void run() {
                if(mUpdateManager.isFirmwareReady()) {
                    try {
                        FileUtil.unZipFile(UpdateManager.FIRMWARE_PATH + UpdateManager.FIRMWARE_NAME, UpdateManager.UNZIP_FIRMWARE_PATH);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DATA_PREPARED);
                    mHandler.sendEmptyMessage(MESSAGE_DEVICE_DATA_PREPARED);
                }else{
                    showToastOnUiThread(getString(R.string.firmware_data_error),Toast.LENGTH_SHORT);
                    SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
                    mHandler.sendEmptyMessage(MESSAGE_UPDATE_PREFERENCE);
                }
            }
        }).start();
    }

    /**
     * tcp connect with device and send needed image to device for update
     * it used jni for detail realize
     */
    private void sendDeviceData(){
        if(!UpdateManager.isImgDataReady()){
            showToastOnUiThread(getString(R.string.firmware_data_error),Toast.LENGTH_SHORT);
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DOWNLOADED);
            refreshPreference(mUpdateManager.isUpdatable());
            return;
        }
        updater_client.setImageDirectory(UpdateManager.UNZIP_FIRMWARE_PATH);
        Log.d(TAG, "sendDeviceData: isHost_PREFIX " + Env.getConnectingSSID(mContext).startsWith(Context.HOTSPOT_PREFIX));
        Log.d(TAG, "sendDeviceData: isConnected " + Env.isWifiConnected(mContext));
        Log.d(TAG, "sendDeviceData: isWifiEnable " + Env.isWifiEnable(mContext));

        if (Env.isWifiEnable(mContext) && Env.isWifiConnected(mContext)
                && Env.getConnectingSSID(mContext).startsWith(Context.HOTSPOT_PREFIX)){
            doUpdate();
            return;
        }
        CustomDialog.Builder builder = new CustomDialog.Builder(mContext);
        builder.setMessage(getString(R.string.firmware_device_data_send_warn));
        builder.setTitle(getString(R.string.dialog_warm));
        builder.setPositiveButton(getResources().getString(R.string.dialog_button_confirm), new DialogInterface.OnClickListener() {
            public void onClick(final DialogInterface dialog, int which) {
                dialog.dismiss();
                Intent intent = new Intent(mContext,HotspotActivity.class);
                Bundle bundle = new Bundle();
                bundle.putBoolean("fromUpdateActivity",true);
                intent.putExtras(bundle);
                startActivityForResult(intent,1);
            }
        });

        builder.setNegativeButton(getResources().getString(R.string.dialog_button_cancel),
                new android.content.DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        builder.create().show();
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent){
        if(requestCode == 1 && resultCode == RESULT_OK){
            doUpdate();
        }
    }

    private void doUpdate(){
        mLoadingDialog.setMessage(getString(R.string.firmware_device_upgrading));
        mLoadingDialog.show();
        new Thread(new Runnable() {
            @Override
            public void run() {
                updater_client.doUpdater();
                mLoadingDialog.dismiss();
            }
        }).start();
    }

    private void onDeviceRebooting(){
        mLoadingDialog.setMessage(getString(R.string.firmware_device_rebooting));
        mLoadingDialog.show();
    }

    private void onDeviceUpdating(){
        mLoadingDialog.setMessage(getString(R.string.firmware_device_upgrading));
        mLoadingDialog.show();
    }

    private void onDeviceVersionLatest(){
        showToastOnUiThread(getString(R.string.device_version_latest),Toast.LENGTH_SHORT);
        // saved sharePreference value
        SharedPreference.putString(mContext,UpdateManager.KEY_DEVICE_VERSION_STR,UpdateManager.getUpdateVersionStr());
        SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
        refreshPreference(mUpdateManager.isUpdatable());
    }

    private class DownloadListener extends DownloadCallback {
        @Override
        public void onStart(int downloadId, long totalBytes) {
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DOWNLOADING);
            /* save download server info to sharePreference in case of quit when downloading */
            mLoadingDialog.dismiss();
            SharedPreference.putString(mContext,UpdateManager.KEY_DOWNLOADING_VERSION_STR,mUpdateManager.getServerVersionNumberStr());
            mProgressDialog.show();
        }

        @Override
        public void onRetry(int downloadId) {
        }

        @Override
        public void onProgress(int downloadId, long bytesWritten, long totalBytes) {
            mProgressDialog.setMax(FileUtil.convertSizeFromBToMB(totalBytes));
            mProgressDialog.setProgress(FileUtil.convertSizeFromBToMB(bytesWritten));
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_FIRMWARE_LENGTH,(int)totalBytes);
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DOWNLOADING);
        }

        @Override
        public void onSuccess(int downloadId, String filePath) {
            mProgressDialog.dismiss();
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_DOWNLOADED);
            refreshPreference(mUpdateManager.isUpdatable());
            prepareDeviceData();
        }

        @Override
        public void onFailure(int downloadId, int statusCode, String errMsg) {
            Log.d(TAG, "download failed with errMsg: " + errMsg);
            SharedPreference.putInt(mContext,UpdateManager.KEY_DOWNLOAD_STATE,UpdateManager.DOWNLOAD_STATE_NONE);
            showToastOnUiThread(getString(R.string.request_error_for_server),Toast.LENGTH_LONG);
            mLoadingDialog.dismiss();
            mProgressDialog.dismiss();
        }
    }

}
