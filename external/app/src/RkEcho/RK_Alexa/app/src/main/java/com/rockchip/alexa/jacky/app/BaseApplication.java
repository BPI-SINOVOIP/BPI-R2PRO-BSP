package com.rockchip.alexa.jacky.app;

import android.app.Application;
import android.app.Dialog;
import android.os.Handler;

import com.rockchip.alexa.jacky.info.DeviceProvisioningInfo;
import com.rockchip.alexa.jacky.socket.TcpClient;
import com.rockchip.alexa.jacky.socket.TcpServer;

/**
 * Created by Administrator on 2017/3/22.
 */

public class BaseApplication extends Application {

    public static final int SOCKET_SERVER_PORT = 1234;
    public static final int SOCKET_CLIENT_PORT = 1221;
    private static BaseApplication mApplication;
    Handler mMainHandler = new Handler();

    private TcpServer mTcpServer;
    private TcpClient mTcpClient;

    private DeviceProvisioningInfo mDeviceProvisioningInfo;

    private boolean mSetupWifi;

    private boolean mSetupBlu;

    private long mAuthorizeTime;

    private String mDeviceSsid;
    private String mDsn;

    /*---------------------------------------------------------------*/
    /**
     * update handler used in DeviceUpdateActivity
     * {@link com.rockchip.alexa.jacky.activity.DeviceUpdateActivity}
     */
    private Handler updateHandler;

    public static BaseApplication getApplication() {
        return mApplication;
    }

    public static BaseApplication getInstance() {
        return mApplication;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mApplication = this;
        mTcpServer = new TcpServer(SOCKET_SERVER_PORT);
        mTcpClient = new TcpClient();

        CodeChallengeWorkflow.getInstance().generateProofKeyParameters();
    }

    public void runOnUiThread(Runnable runnable) {
        if (runnable != null) {
            mMainHandler.post(runnable);
        }
    }

    public void hideDialog(final Dialog dialog) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (dialog != null && dialog.isShowing()) {
                    dialog.hide();
                }
            }
        });
    }

    public void showDialog(final Dialog dialog) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (dialog != null && !dialog.isShowing()) {
                    dialog.show();
                }
            }
        });
    }

    public void dismissDialog(final Dialog dialog) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (dialog != null && dialog.isShowing()) {
                    dialog.dismiss();
                }
            }
        });
    }

    public TcpServer getSocketServer() {
        return mTcpServer;
    }

    public TcpClient getSocketClient() {
        return mTcpClient;
    }

    public void setSetupWifi(boolean setupWifi) {
        this.mSetupWifi = setupWifi;
    }

    public boolean isSetupWifi() {
        return mSetupWifi;
    }

    public void setSetupBlu(boolean setupBlu) {
        this.mSetupBlu = setupBlu;
    }

    public boolean isSetupBlu() {
        return mSetupBlu;
    }

    // about update
    public Handler getUpdateHandler() {
        return updateHandler;
    }

    public void setUpdateHandler(Handler updateHandler) {
        this.updateHandler = updateHandler;
    }

    public void setAuthorizeTime(long authorizeTime) {
        this.mAuthorizeTime = authorizeTime;
    }

    public long getAuthorizeTime() {
        return mAuthorizeTime;
    }

    public void setDeviceSsid(String deviceSsid) {
        this.mDeviceSsid = deviceSsid;
    }

    public String getDeviceSsid() {
        return mDeviceSsid;
    }

    public void setDsn(String dsn) {
        this.mDsn = dsn;
    }

    public String getDsn () {
        return mDsn == null ? AuthConstants.DeviceProvisioning.DSN : mDsn;
    }
}
