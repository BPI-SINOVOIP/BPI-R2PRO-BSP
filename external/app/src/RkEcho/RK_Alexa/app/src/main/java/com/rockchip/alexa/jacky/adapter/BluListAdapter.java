package com.rockchip.alexa.jacky.adapter;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.info.BluInfo;

import java.util.List;

/**
 * Created by Administrator on 2017/3/14.
 */

public class BluListAdapter extends BaseAdapter {
    private LayoutInflater inflater;
    private List<BluInfo> list;

    public BluListAdapter(Context context, List<BluInfo> list){
        this.inflater=LayoutInflater.from(context);
        this.list=list;
    }

    public List<BluInfo> getList() {
        return list;
    }

    public void setList(List<BluInfo> scanResult) {
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
        if (list.get(position).getTag() != null && !list.get(position).getTag().isEmpty()) {
            View view = inflater.inflate(R.layout.blu_listitem_tag, null);

            BluInfo blu = list.get(position);
            TextView name = (TextView) view.findViewById(R.id.name);

            Log.d("Test", "blu.getTag() i:" + position + "; " + blu.getTag());
            name.setText(blu.getTag());
            return view;
        } else {
            View view = inflater.inflate(R.layout.blu_listitem, null);

            BluInfo blu = list.get(position);
            TextView name = (TextView) view.findViewById(R.id.name);
            TextView addr = (TextView) view.findViewById(R.id.addr);

            name.setText(blu.getName());
            addr.setText(blu.getAddr());

            return view;
        }
    }
}
