package com.rockchip.smart.rockhome.softap;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.rockchip.smart.rockhome.R;
import com.rockchip.smart.rockhome.WifiInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by GJK on 2018/11/9.
 */

public class SoftapDeviceAdapter extends BaseAdapter {

    private Context mContext;
    private List<WifiInfo> mDevices;
    private SoftapDeviceAdapterCallback mSoftapDeviceAdapter;

    public interface SoftapDeviceAdapterCallback {
        void onItemClick(WifiInfo info);
    }

    public SoftapDeviceAdapter(Context context, SoftapDeviceAdapterCallback softapDeviceAdapter) {
        mContext = context;
        mDevices = new ArrayList<WifiInfo>();
        mSoftapDeviceAdapter = softapDeviceAdapter;
    }

    public SoftapDeviceAdapter(Context context) {
        this(context, null);
    }

    public List<WifiInfo> getDevices() {
        return mDevices;
    }

    public void setDevices(List<WifiInfo> devices) {
        this.mDevices = devices;
    }

    @Override
    public int getCount() {
        return mDevices.size();
    }

    @Override
    public Object getItem(int position) {
        return mDevices.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }


    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder;
        if (convertView == null) {
            convertView = LayoutInflater.from(mContext).inflate(R.layout.item_blue, null);
            convertView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
//                    Log.d("WifiFragment", "Click " + mDevices.get(position).getSsid());
//                    WifiManager manager = WifiManager.getInstance(mContext);
//                    if (manager.isWifiConnected()) {
//                        manager.disconnectNetWork();
//                    }
//                    manager.connCustomNetWork("TP-LINK_C734BC", "12345678", WifiManager.Data.WIFI_CIPHER_WPA2);
                    if (mSoftapDeviceAdapter != null)
                        mSoftapDeviceAdapter.onItemClick(mDevices.get(position));
                }
            });
            viewHolder = new ViewHolder();
            viewHolder.deviceName = (TextView) convertView.findViewById(R.id.tv_blue);

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        WifiInfo device = mDevices.get(position);
        viewHolder.deviceName.setText(device.getSsid());

        return convertView;
    }

    static class ViewHolder {
        TextView deviceName;
    }
}
