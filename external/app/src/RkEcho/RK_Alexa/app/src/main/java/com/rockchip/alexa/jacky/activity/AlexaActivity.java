package com.rockchip.alexa.jacky.activity;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;

import com.amazon.identity.auth.device.AuthError;
import com.amazon.identity.auth.device.api.authorization.AuthCancellation;
import com.amazon.identity.auth.device.api.authorization.AuthorizeListener;
import com.amazon.identity.auth.device.api.authorization.AuthorizeResult;
import com.amazon.identity.auth.device.api.workflow.RequestContext;
import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.app.AuthConstants;
import com.rockchip.alexa.jacky.app.AuthManager;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.fragment.AlexaAuthorSuccessFragment;
import com.rockchip.alexa.jacky.fragment.CompatActivity;
import com.rockchip.alexa.jacky.fragment.NoFragment;
import com.rockchip.alexa.jacky.fragment.SupportsFragment;
import com.rockchip.alexa.jacky.info.CompanionProvisioningInfo;
import com.rockchip.alexa.jacky.info.DeviceProvisioningInfo;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.socket.TcpClient;

/**
 * Created by Administrator on 2017/3/21.
 */

public class AlexaActivity extends CompatActivity {
    private static final String TAG = "AlexaActivity";
    private RequestContext mRequestContext;
    private ProvisioningClient mProvisioningClient;
    private DeviceProvisioningInfo mDeviceProvisioningInfo;
    private String deviceIp;
    private NoFragment mFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mRequestContext = RequestContext.create(this);
        mRequestContext.registerListener(new AuthorizeListenerImpl());

        try {
            mProvisioningClient = ProvisioningClient.getProvisioningClient(this);
        } catch (Exception e) {
            Log.e(TAG, "Unable to use Provisioning Client. CA Certificate is incorrect or does not exist.", e);
        }

        startFragment(SupportsFragment.class);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mRequestContext.onResume();
    }

    @Override
    protected int fragmentLayoutId() {
        return R.id.fragment_root;
    }

    public void setFragment(NoFragment fragment) {
        this.mFragment = fragment;
    }

    public String getDeviceIp() {
        return deviceIp;
    }

    public void setDeviceIp(String deviceIp) {
        this.deviceIp = deviceIp;
    }

    public DeviceProvisioningInfo getDeviceProvisioningInfo() {
        return mDeviceProvisioningInfo;
    }

    public RequestContext getRequestContext() {
        return mRequestContext;
    }

    public void setDeviceProvisioningInfo(DeviceProvisioningInfo mDeviceProvisioningInfo) {
        this.mDeviceProvisioningInfo = mDeviceProvisioningInfo;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyDown(keyCode, event);
    }

    private CompanionProvisioningInfo companionProvisioningInfo;

    private boolean mIsAuthorized;
    public boolean isAuthorized() {
        return mIsAuthorized;
    }

    public void setAuthorized(boolean isAuthorized) {
        this.mIsAuthorized = isAuthorized;
    }

    private class AuthorizeListenerImpl extends AuthorizeListener {
        @Override
        public void onSuccess(final AuthorizeResult authorizeResult) {
            BaseApplication.getApplication().setAuthorizeTime(SystemClock.uptimeMillis());
            final String authorizationCode = authorizeResult.getAuthorizationCode();
            final String redirectUri = authorizeResult.getRedirectURI();
            final String clientId = authorizeResult.getClientId();
            final String sessionId = getDeviceProvisioningInfo().getSessionId();

            companionProvisioningInfo = new CompanionProvisioningInfo(sessionId, clientId, redirectUri, authorizationCode, getDeviceProvisioningInfo().getCodeVerifier());
            AuthManager.getAuthManager().setCompanionProvisioningInfo(companionProvisioningInfo);

            new AsyncTask<Void, Void, Void>() {
                @Override
                protected void onPreExecute() {
                    super.onPreExecute();
                }

                @Override
                protected Void doInBackground(Void... voids) {
                    try {
                        Log.d(TAG, "result:" + companionProvisioningInfo.toJson().toString());
                        if (AuthConstants.COMMUNICATE_TYPE.equals(AuthConstants.CommunicateType.SOCKET)) {
                            TcpClient client = new TcpClient();
                            client.connect(getDeviceIp(), 1221);
                            Thread.sleep(1000);
                            client.send(companionProvisioningInfo.toJson().toString());
                            if (mFragment != null) {
                                mFragment.startFragment(AlexaAuthorSuccessFragment.class);
                            }
                        } else if (AuthConstants.COMMUNICATE_TYPE.equals(AuthConstants.CommunicateType.HTTPS)) {
                            if (mFragment != null) {
                                mFragment.startFragment(AlexaAuthorSuccessFragment.class);
                            }
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "Post CompanionProvisioningInfo fail", e);
                    }
                    return null;
                }

                @Override
                protected void onPostExecute(Void result) {
                    super.onPostExecute(result);
                }
            }.execute();
        }

        @Override
        public void onError(final AuthError authError) {
            Log.e(TAG, "AuthError during authorization", authError);
        }

        @Override
        public void onCancel(final AuthCancellation authCancellation) {
            Log.e(TAG, "User cancelled authorization");
        }
    }
}
