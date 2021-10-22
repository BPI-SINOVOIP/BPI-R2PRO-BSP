package com.rockchip.smart.rockhome;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.ListView;

import java.util.List;

/**
 * Created by GJK on 2018/11/12.
 */

public class WifiListDialogFragment extends DialogFragment {

    private ListView mLvWifiList;
    private List<WifiInfo> mWifiList;
    private WifiListFragment mWifiPickerFragment;
    private static WifiListFragment.WifiCallback mWifiCallback;

    public static WifiListDialogFragment createDialog() {
        return createDialog(null);
    }

    public static WifiListDialogFragment createDialog(WifiListFragment.WifiCallback wifiCallback) {
        WifiListDialogFragment dialogFragment = new WifiListDialogFragment();
        dialogFragment.setStyle(DialogFragment.STYLE_NORMAL, R.style.Dialog);

        mWifiCallback = wifiCallback;
        return dialogFragment;
    }

    public void setWifiList(List<WifiInfo> wifiList) {
        this.mWifiList = wifiList;
    }

    public List<WifiInfo> getWifiList() {
        return mWifiList;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        View root = getActivity().getLayoutInflater().inflate(R.layout.dialog_wifilist, null);
        initWifiPickerFragment();

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity(), R.style.Dialog);
        builder.setView(root);

        AlertDialog dialog = builder.create();
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }

    private void initWifiPickerFragment() {
        FragmentManager mgr = getFragmentManager();
        mWifiPickerFragment = (WifiListFragment) mgr.findFragmentById(R.id.wifi_picker_id);
        mWifiPickerFragment.setCallback(mWifiCallback);
    }

    @Override
    public void onResume() {
        super.onResume();
        mWifiPickerFragment.scan(true, mWifiList);
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        if (mWifiPickerFragment != null) {
            mWifiPickerFragment.getFragmentManager().beginTransaction()
                    .remove(mWifiPickerFragment).commit();
        }
    }
}
