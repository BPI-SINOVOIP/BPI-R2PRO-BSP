package com.rockchip.alexa.jacky.fragment;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.Toolbar;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.broadcom.cooee.Cooee;
import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.activity.AlexaActivity;
import com.rockchip.alexa.jacky.app.AuthConstants;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.DesUtils;
import com.rockchip.alexa.jacky.info.DeviceProvisioningInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.socket.SocketListener;
import com.rockchip.alexa.jacky.socket.SocketTransceiver;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Administrator on 2017/3/22.
 */

public class WifiInputFragment extends NoFragment implements SocketListener {
    private static final String TAG = "WifiInputFragment";
    private LoadingView mLoadingView;
    private Toolbar mToolbar;
    private TextView mWifiSSID, mConfirm;
    private ImageView mWifiPwdLock;
    private boolean mShowPwd;
    private EditText mWifiPwd;

    private boolean mSmartConfiged;
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.frag_wifi_input, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        mToolbar = (Toolbar) view.findViewById(R.id.toolbar);
        mWifiSSID = (TextView) view.findViewById(R.id.frag_wifi_input_ssid);
        mWifiSSID.setText(Env.getConnectingSSID(BaseApplication.getApplication()));
        mWifiPwdLock = (ImageView) view.findViewById(R.id.frag_wifi_input_lock);
        mWifiPwd = (EditText)view.findViewById(R.id.frag_wifi_input_pwd);
        mConfirm = (TextView) view.findViewById(R.id.frag_wifi_input_confirm);
        mConfirm.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                String pwd = mWifiPwd.getText().toString();
                if (pwd == null || pwd.length() < 8) {
                    Toast.makeText(BaseApplication.getApplication(), getString(R.string.error_pwd), Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(BaseApplication.getApplication(), "text:" + pwd, Toast.LENGTH_SHORT).show();
                    if (mLoadingView != null && !mLoadingView.isShowing()) {
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                WifiManager wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
                                final WifiInfo info = wifiManager.getConnectionInfo();
                                Toast.makeText(getActivity(), "IP:" + info.getIpAddress(), Toast.LENGTH_SHORT).show();
                                BaseApplication.getApplication().getSocketServer().setSocketListener(WifiInputFragment.this);
                                BaseApplication.getApplication().getSocketServer().start();
                                mLoadingView.show();
                                mSmartConfiged = false;
                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        while (!mSmartConfiged) {
                                            Cooee.send(mWifiSSID.getText().toString(), mWifiPwd.getText().toString(), info.getIpAddress());
                                            try {
                                                Thread.sleep(1000);
                                            } catch (InterruptedException e) {
                                                e.printStackTrace();
                                            }
                                        }
                                    }
                                }).start();
                            }
                        });
                    }
                }
            }
        });

        mWifiPwdLock.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                mShowPwd = !mShowPwd;
                if (mShowPwd) {
                    mWifiPwdLock.setImageResource(R.drawable.password_input_02);
                    mWifiPwd.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                } else {
                    mWifiPwdLock.setImageResource(R.drawable.password_input_01);
                    mWifiPwd.setTransformationMethod(PasswordTransformationMethod.getInstance());
                }
            }
        });

        mLoadingView = new LoadingView(getActivity(), getString(R.string.wifi_setting_up));
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ((AlexaActivity)getActivity()).setFragment(this);
        setToolbar(mToolbar);

        // Set title for toolbar:
        setTitle("WIFI Settings");

        // Display close button.
        displayHomeAsUpEnabled(R.drawable.ic_toolbar_back);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.menu_fragment_main, menu);

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onServerStart() {
        Log.d(TAG, "Socket onServerStart!");
    }

    @Override
    public void onConnect(SocketTransceiver addr) {
        Log.d(TAG, "Socket onConnect!");
    }

    @Override
    public void onConnectFail(String ip, int port) {
        Log.d(TAG, "Socket onConnectFail!");
    }

    @Override
    public void onConnectTimeout(String ip, int port) {
        Log.d(TAG, "Socket onConnectTimeout!");
    }

    @Override
    public void onReceive(final SocketTransceiver addr, String msg) {
        Log.d(TAG, "Socket onReceive!" + msg);
        mSmartConfiged = true;
        try {
            JSONObject resp = new JSONObject(msg);
            String type = resp.getString("type");
            JSONObject response = resp.getJSONObject("content");

            if (type.equals("DeviceProvisioning")) {
                List<String> missingParameters = new ArrayList<String>();
                if (!response.has(AuthConstants.PRODUCT_ID)) {
                    missingParameters.add(AuthConstants.PRODUCT_ID);
                }

                if (!response.has(AuthConstants.DSN)) {
                    missingParameters.add(AuthConstants.DSN);
                }

                if (!response.has(AuthConstants.SESSION_ID)) {
                    missingParameters.add(AuthConstants.SESSION_ID);
                }

                if (!response.has(AuthConstants.CODE_CHALLENGE)) {
                    missingParameters.add(AuthConstants.CODE_CHALLENGE);
                }

                if (!response.has(AuthConstants.CODE_CHALLENGE_METHOD)) {
                    missingParameters.add(AuthConstants.CODE_CHALLENGE_METHOD);
                }

                if (missingParameters.size() != 0) {
                    throw new DeviceProvisioningInfo.MissingParametersException(missingParameters);
                }

                String productId = response.getString(AuthConstants.PRODUCT_ID).trim();
                if (productId.startsWith("RK_")) {
                    Log.d(TAG, "producted is encrypted");
                    productId = productId.substring(3);
                    try {
                        DesUtils des = new DesUtils();
                        productId = des.decrypt(productId);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                Log.d(TAG, "productId is " + productId);
                final String dsn = response.getString(AuthConstants.DSN);
                final String sessionId = response.getString(AuthConstants.SESSION_ID);
                final String codeChallenge = response.getString(AuthConstants.CODE_CHALLENGE);
                final String codeChallengeMethod = response.getString(AuthConstants.CODE_CHALLENGE_METHOD);
                final String codeVerifier = response.getString(AuthConstants.CODE_VERIFIER);

                final DeviceProvisioningInfo deviceProvisioningInfo = new DeviceProvisioningInfo(productId, dsn, sessionId, codeChallenge, codeChallengeMethod, codeVerifier);
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mLoadingView != null && mLoadingView.isShowing()) {
                            mLoadingView.dismiss();
                        }
                        AlexaActivity activity = ((AlexaActivity) getActivity());
                        activity.setDeviceProvisioningInfo(deviceProvisioningInfo);
                        activity.setDeviceIp(addr.getInetAddress().getHostAddress());
                        startFragment(AlexaIndexFragment.class);
                    }
                });
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onDisconnect(SocketTransceiver addr) {
        Log.d(TAG, "Socket onDisconnect!");
    }

    @Override
    public void onServerStop() {
        Log.d(TAG, "Socket server stop!");
    }
}
