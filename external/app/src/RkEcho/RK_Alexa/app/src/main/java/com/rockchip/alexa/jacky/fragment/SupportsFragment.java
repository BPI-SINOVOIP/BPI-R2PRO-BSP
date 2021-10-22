package com.rockchip.alexa.jacky.fragment;


import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.activity.AlexaActivity;
import com.rockchip.alexa.jacky.activity.DeviceUpdateActivity;
import com.rockchip.alexa.jacky.activity.HotspotActivity;
import com.rockchip.alexa.jacky.app.AuthManager;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.views.LoadingView;


/**
 * Created by Administrator on 2017/3/21.
 */

public class SupportsFragment extends NoFragment {

    private Toolbar mToolbar;
    private TextView mBtnBack;

    private RelativeLayout mAlexa;
    private RelativeLayout mWifiSetup;
    private RelativeLayout mBluSetup;
    private RelativeLayout mDeviceUpdate;

    private LoadingView mLoadingView;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.frag_supportlist, container, false);
    }

    private AlertDialog alert;
    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        mToolbar = (Toolbar) view.findViewById(R.id.toolbar);
        mBtnBack = (TextView) view.findViewById(R.id.toolbar_back);
        mAlexa = (RelativeLayout) view.findViewById(R.id.list_support_alexa);
        mWifiSetup = (RelativeLayout) view.findViewById(R.id.list_support_wifi_setup);
        mBluSetup = (RelativeLayout) view.findViewById(R.id.list_support_blu_setup);
        mDeviceUpdate = (RelativeLayout) view.findViewById(R.id.list_support_update);

        mLoadingView = new LoadingView(getActivity(), getString(R.string.wifi_connecting));

        mAlexa.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                BaseApplication.getApplication().setSetupWifi(false);
//                if (AuthManager.getAuthManager().isAuthorized()) {
//                    ((AlexaActivity) getActivity()).setAuthorized(true);
//                    startFragment(AlexaAuthorSuccessFragment.class, false);
//                } else {
//                    ((AlexaActivity) getActivity()).setAuthorized(false);
                    //startFragment(AlexaIndexFragment.class, false);
                    startFragment(HotspotFragment.class, false);
//                }
            }
        });

        mWifiSetup.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                BaseApplication.getApplication().setSetupWifi(true);
                startActivity(new Intent(getActivity(), HotspotActivity.class));
            }
        });

        mBluSetup.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                BaseApplication.getApplication().setSetupBlu(true);
                startActivity(new Intent(getActivity(), HotspotActivity.class));
            }
        });

        mDeviceUpdate.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                startActivity(new Intent(getActivity(), DeviceUpdateActivity.class));
            }
        });

        mBtnBack.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                getActivity().finish();
            }
        });

    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ((AlexaActivity)getActivity()).setFragment(this);
        setToolbar(mToolbar);

        // Set title for toolbar:
//        setTitle("Partner App");

        // Display close button.
//        displayHomeAsUpEnabled(R.drawable.ic_toolbar_back);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
//        inflater.inflate(R.menu.menu_fragment_main, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }
}
