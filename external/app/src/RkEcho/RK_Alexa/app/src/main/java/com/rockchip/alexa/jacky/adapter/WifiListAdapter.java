package com.rockchip.alexa.jacky.adapter;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.net.wifi.WifiManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.info.WifiInfo;

import java.util.List;

/**
 * Created by Administrator on 2017/3/14.
 */

public class WifiListAdapter extends BaseAdapter {
    private LayoutInflater inflater;
    private List<WifiInfo> list;
    private int level;

    public WifiListAdapter(Context context, List<WifiInfo> list){
        this.inflater=LayoutInflater.from(context);
        this.list=list;
    }

    public List<WifiInfo> getList() {
        return list;
    }

    public void setList(List<WifiInfo> scanResult) {
        this.list = scanResult;
    }

    @Override
    public int getCount() {
        return list.size();
    }
    @Override
    public Object getItem(int position) {
        return position;
    }
    @Override
    public long getItemId(int position) {
        return position;
    }

    @SuppressLint({ "ViewHolder", "InflateParams" })
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view=inflater.inflate(R.layout.wifi_listitem, null);
        WifiInfo scanResult = list.get(position);
        TextView wifi_ssid=(TextView) view.findViewById(R.id.ssid);
        ImageView wifi_level=(ImageView) view.findViewById(R.id.wifi_level);
        wifi_ssid.setText(scanResult.getSsid());
        if (list.get(position).isConnecting()) {
            wifi_ssid.setTextColor(Color.BLUE);
        }
        level= WifiManager.calculateSignalLevel(Integer.parseInt(scanResult.getSignalLevel()), 5);
        if(scanResult.getFlags().contains("WEP")||scanResult.getFlags().contains("PSK")|| scanResult.getFlags().contains("EAP")){
            wifi_level.setImageResource(R.drawable.wifi_signal_lock);
        }else{
            wifi_level.setImageResource(R.drawable.wifi_signal_open);
        }
        wifi_level.setImageLevel(level);
        return view;
    }
}
