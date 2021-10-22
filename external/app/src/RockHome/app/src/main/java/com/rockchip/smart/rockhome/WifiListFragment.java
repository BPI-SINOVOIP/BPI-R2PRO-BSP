/******************************************************************************
 *
 *  Copyright (C) 2013-2014 Cypress Semiconductor
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
package com.rockchip.smart.rockhome;

import android.app.Activity;
import android.app.ListFragment;
import android.content.Context;
import android.graphics.Color;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/**
 * UI component to allow users to scan for Bluetooth LE devices and pick a
 * selected device *
 */
public class WifiListFragment extends ListFragment {
    private static final String TAG = "WifiListFragment";

    /**
     * Interface to listen for results
     */
    public static interface WifiCallback {
        public void onWifiPicked(WifiInfo info);

        public void onWifiPickCancelled();
    }

    /**
     * Helper class used for displaying LE devices in a pick list
     */
    public static class WifiAdapter extends BaseAdapter {

        private final Context mContext;
        private List<WifiInfo> mWifis;
        private final LayoutInflater mInflater;

        public WifiAdapter(Context context) {
            mContext = context;

            mInflater = LayoutInflater.from(context);
            mWifis = new ArrayList<WifiInfo>();
        }

        public void update(List<WifiInfo> wifis) {
            mWifis = wifis;
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mWifis.size();
        }

        @Override
        public Object getItem(int position) {
            return mWifis.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;

            if (convertView == null || convertView.findViewById(R.id.ssid) == null) {
                convertView = mInflater.inflate(R.layout.dialog_wifilist_item, null);
                holder = new ViewHolder();
                holder.ssid = (TextView) convertView.findViewById(R.id.ssid);
                holder.level = (ImageView) convertView.findViewById(R.id.wifi_level);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            WifiInfo rec = mWifis.get(position);
            holder.ssid.setText(rec.getSsid());
            if (rec.isConnecting()) {
                holder.ssid.setTextColor(Color.BLUE);
            }
            int level= WifiManager.calculateSignalLevel(Integer.parseInt(rec.getSignalLevel()), 5);
            if(rec.getFlags() == null || rec.getFlags().contains("WEP") || rec.getFlags().contains("PSK") || rec.getFlags().contains("EAP")){
                holder.level.setImageResource(R.drawable.wifi_signal_lock);
            }else{
                holder.level.setImageResource(R.drawable.wifi_signal_open);
            }
            holder.level.setImageLevel(level);

            return convertView;
        }

        static class ViewHolder {
            TextView ssid;
            ImageView level;
        }
    }

    private WifiAdapter mWifiAdapter;
    private WifiCallback mCallback;
    private boolean mDevicePicked;

    /**
     * Set the callback object to invoke when a device is picked OR if the
     * device picker is cancelled
     *
     * @param cb
     */
    public void setCallback(WifiCallback cb) {
        mCallback = cb;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Activity activity = getActivity();

        // Otherwise populate the list device
        mWifiAdapter = new WifiAdapter(activity);
        setListAdapter(mWifiAdapter);
    }

    @Override
    public void onListItemClick(ListView list, View view, int position, long id) {
        WifiInfo wifi = (WifiInfo) mWifiAdapter.getItem(position);
        if (wifi != null && mCallback != null) {
            try {
                mDevicePicked = true;
                mCallback.onWifiPicked(wifi);
            } catch (Throwable t) {
                Log.w(TAG, "onListItemClick: error calling callback", t);
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d("BlueFragment", "WifiListFragment onResume");
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d("BlueFragment", "WifiListFragment onPause");
        if (!mDevicePicked && mCallback != null)
            mCallback.onWifiPickCancelled();
    }

    public void scan(boolean enable) {
        scan(enable, null);
    }

    /**
     * Start or stop scanning for LE devices
     *
     * @param enable
     */
    public void scan(boolean enable, List<WifiInfo> wifiList) {
        if (enable && wifiList != null) {
            mWifiAdapter.update(wifiList);
        }

        getActivity().invalidateOptionsMenu();
    }
}