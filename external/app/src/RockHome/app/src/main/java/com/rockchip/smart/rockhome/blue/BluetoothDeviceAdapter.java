package com.rockchip.smart.rockhome.blue;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Build;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.rockchip.smart.rockhome.R;

import java.util.ArrayList;

/**
 * Created by GJK on 2018/11/9.
 */

public class BluetoothDeviceAdapter extends BaseAdapter {

    private Context mContext;
    private ArrayList<Device> mDevices;
    private BluetoothDeviceAdapterCallback mCallback;

    public BluetoothDeviceAdapter(Context context) {
        this(context, null);
    }

    public BluetoothDeviceAdapter(Context context, BluetoothDeviceAdapterCallback callback) {
        mContext = context;
        mDevices = new ArrayList<Device>();
        mCallback = callback;
    }

    public ArrayList<Device> getDevices() {
        return mDevices;
    }

    public void setDevices(ArrayList<Device> devices) {
        this.mDevices = devices;
    }

    public BluetoothDeviceAdapterCallback getCallback() {
        return mCallback;
    }

    public void setCallback(BluetoothDeviceAdapterCallback callback) {
        this.mCallback = callback;
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

    public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
        for (Device blue : mDevices) {
            if (blue.getDevice().getAddress().equals(device.getAddress())) {
                blue.setDevice(device);
                blue.setRssi(rssi);

                return;
            }
        }

        if (device != null && device.getName() != null && (device.getName().startsWith("RockChip") ||
                device.getName().startsWith("Rockchip") || device.getName().startsWith("ROCKCHIP"))) {
            Device blueDevice = new Device(device, rssi);
            mDevices.add(blueDevice);
            if (mCallback != null)
                mCallback.onLeScan(blueDevice, mDevices.size() - 1);

            notifyDataSetChanged();
        }
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder;
        if (convertView == null) {
            convertView = LayoutInflater.from(mContext).inflate(R.layout.item_blue, null);
            viewHolder = new ViewHolder();
            viewHolder.rootView = (LinearLayout) convertView.findViewById(R.id.item_blue_root);
            viewHolder.deviceName = (TextView) convertView.findViewById(R.id.tv_blue);
            viewHolder.deviceAddr = (TextView) convertView.findViewById(R.id.tv_blue_addr);

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        Device device = mDevices.get(position);
        viewHolder.deviceName.setText(device.getDevice().getName());
        viewHolder.deviceAddr.setText(device.getDevice().getAddress());

        viewHolder.rootView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mCallback != null) {
                    mCallback.onBluetoothDeviceClick(mDevices.get(position), position);
                }
            }
        });

        return convertView;
    }

    static class ViewHolder {
        LinearLayout rootView;
        TextView deviceName;
        TextView deviceAddr;
    }

    public interface BluetoothDeviceAdapterCallback {
        void onLeScanStart();
        void onLeScan(Device device, int position);
        void onLeScanStop(int size);

        void onBluetoothDeviceClick(Device device, int position);
    }

    private abstract class OnConvertViewClickListener implements View.OnClickListener{
        private View convertView;
        private int[] positionIds;
        public OnConvertViewClickListener(View convertView, int... positionIds) {
            this.convertView = convertView;
            this.positionIds = positionIds;
        }

        @TargetApi(Build.VERSION_CODES.DONUT)
        @Override
        public void onClick(View v) {
            int len = positionIds.length;
            int[] positions = new int[len];
            for(int i = 0; i < len; i++){
                positions[i] = (int) convertView.getTag(positionIds[i]);
            }
            onClickCallBack(v, positions);
        }

        public abstract void onClickCallBack(View registedView, int... positionIds);
    };
}
