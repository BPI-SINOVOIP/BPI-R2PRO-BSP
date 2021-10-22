package com.rockchip.alexa.jacky.fragment;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.activity.WifiListActivity;
import com.rockchip.alexa.jacky.adapter.WifiListAdapter;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.async.AsyncFactory;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.info.WifiInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.listener.OnSingleItemClickListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import java.util.List;

import static android.media.CamcorderProfile.get;
import static com.rockchip.alexa.jacky.async.AsyncFactory.getAsyncThread;

/**
 * Created by Administrator on 2017/4/28.
 */

public class HotspotFragment extends NoFragment {
    private final String TAG = "RK_Alexa";

    private Button mBtnConfirm;
    private TextView mBtnBack, mBtnSkip;
    private LoadingView mLoadingView;
    private AlertDialog alert;

    private boolean isFromUpdateActivity = false;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.activity_hotspot, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        mBtnConfirm = (Button) view.findViewById(R.id.activity_hotspot_confirm);
        mBtnConfirm.setOnClickListener(onSingleClickListener);
        mBtnBack = (TextView) view.findViewById(R.id.toolbar_back);
        mBtnBack.setOnClickListener(onSingleClickListener);
        mBtnSkip = (TextView) view.findViewById(R.id.toolbar_skip);
        mBtnSkip.setOnClickListener(onSingleClickListener);
        mLoadingView = new LoadingView(getActivity(), getString(R.string.wifi_connecting));
    }

    private void onClickConfirm() {
            if (Env.isWifiEnable(getActivity())) {
                final List<WifiInfo> list = getAsyncThread().scanDeviceHotspot();
                View view1 = LayoutInflater.from(getActivity()).inflate(R.layout.wifi_list, null);

                ListView listView = (ListView) view1.findViewById(R.id.wifi_list);
                listView.setOnItemClickListener(new OnSingleItemClickListener() {
                    @Override
                    protected void onSingleItemClick(AdapterView<?> adapterView, View view, final int position, long id) {
                        String dsn = list.get(position).getSsid();
                        int index = dsn.lastIndexOf("-");
                        if (index > 0 && index < dsn.length()) {
                            BaseApplication.getApplication().setDeviceSsid(dsn);
                            dsn = dsn.substring(index + 1);
                            BaseApplication.getApplication().setDsn(dsn);
                            Log.d("TEST", "DSN:" + dsn);
                            if (alert.isShowing()) {
                                alert.dismiss();
                            }
                            startFragment(AlexaIndexFragment.class, false);
                        }
                    }
                });
                WifiListAdapter wifiListAdapter = new WifiListAdapter(getActivity(), list);
                listView.setAdapter(wifiListAdapter);
                alert = new AlertDialog.Builder(getActivity()).create();
                alert.setTitle(getString(R.string.name_scan_device));
                alert.setView(view1);
                alert.show();
            }
    }

    private void showNoAuthorizedDialog() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder alert=new AlertDialog.Builder(getActivity());
                alert.setTitle(getString(R.string.title_prompt));
                alert.setMessage(getString(R.string.if_continue_setup_wifi));

                alert.setPositiveButton(getString(R.string.confirm), new DialogInterface.OnClickListener(){
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        startActivity(new Intent(getActivity(), WifiListActivity.class));
                    }
                });

                alert.setNegativeButton(getString(R.string.cancel), new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        HotspotFragment.this.finish();
                    }
                });
                alert.create();
                alert.show();
            }
        });
    }

    private OnSingleClickListener onSingleClickListener = new OnSingleClickListener() {
        @Override
        protected void onSingleClick(View view) {
            switch (view.getId()) {
                case R.id.activity_hotspot_confirm:
                    onClickConfirm();
                    break;
                case R.id.toolbar_back:
                    HotspotFragment.this.finish();
                    break;
                case R.id.toolbar_skip:
                    HotspotFragment.this.finish();
                    break;
            }
        }
    };
}
