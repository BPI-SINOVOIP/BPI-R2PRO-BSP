package com.rockchip.alexa.jacky.fragment;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.amazon.identity.auth.device.AuthError;
import com.amazon.identity.auth.device.api.Listener;
import com.amazon.identity.auth.device.api.authorization.AuthorizationManager;
import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.activity.AlexaActivity;
import com.rockchip.alexa.jacky.activity.HotspotActivity;
import com.rockchip.alexa.jacky.activity.WifiListActivity;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.async.AsyncFactory;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import static android.R.id.list;

/**
 * Created by Administrator on 2017/3/22.
 */

public class AlexaAuthorSuccessFragment extends NoFragment {

    private static final String TAG = "AlexaIndexFragment";
    private Toolbar mToolbar;
    private TextView mBtnBack;
    private TextView mTvHint;
    private  TextView mBtnLoginOut;

    private LinearLayout mLayoutSuccessHint;
    private LinearLayout mLayoutSuccessInfo;
    private TextView mLayoutSuccessHintTitle;

    private Button mBtnDeviceSetup;

    private boolean mShowInfo;

    private LoadingView mLoadingView;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.frag_alexa_author_success, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mLoadingView = new LoadingView(getActivity(), getString(R.string.wifi_connecting));

        mToolbar = (Toolbar) view.findViewById(R.id.toolbar);
        mBtnBack = (TextView) view.findViewById(R.id.toolbar_back);
        mTvHint = (TextView) view.findViewById(R.id.frag_alexa_auth_success_hint_title);
        mBtnLoginOut = (TextView) view.findViewById(R.id.toolbar_loginout);

        mLayoutSuccessHint = (LinearLayout) view.findViewById(R.id.frag_alexa_auth_success_hint);
        mLayoutSuccessInfo = (LinearLayout) view.findViewById(R.id.frag_alexa_auth_success_info);
        mLayoutSuccessHintTitle = (TextView) view.findViewById(R.id.frag_alexa_auth_success_hint_title);
        mBtnDeviceSetup = (Button) view.findViewById(R.id.frag_alexa_auth_success_info_device_setup);
        updateView();
        if (!((AlexaActivity)getActivity()).isAuthorized()) {
            Log.d(TAG, "It's authorized befor");
            new Thread(new Runnable() {
                @Override
                public void run() {
                    int time = 5;
                    final String txt = getString(R.string.auth_success_time_hint);
                    for (int i = 0; i < time; time--) {
                        final int tmp = time;
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mLayoutSuccessHintTitle.setText(String.format(txt, tmp));
                            }
                        });
                        try {
                            Thread.sleep(1000);
                        } catch (Exception e) {

                        }
                    }
                    mShowInfo = true;
                    updateView();
                }
            }).start();
        } else {
            Log.d(TAG, "It's not authorized befor");
            mShowInfo = true;
            updateView();
        }

        mBtnBack.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                startFragment(SupportsFragment.class, false);
            }
        });

        mBtnLoginOut.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                AuthorizationManager.signOut(getActivity(), new Listener<Void, AuthError>() {
                    @Override
                    public void onSuccess(Void aVoid) {
                        Log.d(TAG, "SignOut Success");
                    }

                    @Override
                    public void onError(AuthError authError) {
                        Log.d(TAG, "SignOut Error");
                    }
                });
            }
        });

        mBtnDeviceSetup.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
//                startActivity(new Intent(getActivity(), HotspotActivity.class));
                if (Env.isWifiEnable(getActivity())) {
                    if (Env.isWifiConnected(getActivity()) && Env.getConnectingSSID(getActivity()).startsWith(Context.HOTSPOT_PREFIX)) {
                        startActivity(new Intent(getActivity(), WifiListActivity.class));
                        return;
                    }
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            mLoadingView.show();
                            int res = 0;
                            for (int i = 0; i < 5; i++) {
                                res = AsyncFactory.getAsyncThread().connect(BaseApplication.getApplication().getDeviceSsid());
                                if (res == 1) {
                                    break;
                                }
                            }

                            if (mLoadingView != null && mLoadingView.isShowing()) {
                                mLoadingView.dismiss();
                            }
                            final int result = res;
                            if (result == 1) {
                                startActivity(new Intent(getActivity(), WifiListActivity.class));
                            } else {
                                getActivity().runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast.makeText(getActivity(), (result == -1) ? getString(R.string.wifi_connected_fail) : (result == 0) ? getString(R.string.wifi_connected_timeout) : "", Toast.LENGTH_LONG).show();
                                    }
                                });
                            }
                        }
                    }).start();
                }
            }
        });
    }

    private void updateView() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mShowInfo) {
                    mLayoutSuccessInfo.setVisibility(View.VISIBLE);
                    mLayoutSuccessHint.setVisibility(View.GONE);
                } else {
                    mLayoutSuccessInfo.setVisibility(View.GONE);
                    mLayoutSuccessHint.setVisibility(View.VISIBLE);
                }
            }
        });
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ((AlexaActivity)getActivity()).setFragment(this);
        setToolbar(mToolbar);

        // Set title for toolbar:
//        setTitle("Author Success");

        // Display close button.
//        displayHomeAsUpEnabled(R.drawable.ic_toolbar_back);
    }
}
