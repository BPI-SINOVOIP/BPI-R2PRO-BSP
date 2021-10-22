package com.rockchip.alexa.jacky.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.info.SupportEntity;

import java.util.List;

/**
 * Created by Administrator on 2017/3/22.
 */

public class SupportAdapter extends BaseAdapter {

    private Context context;
    private LayoutInflater inflater;
    private List<SupportEntity> list;

    public SupportAdapter(Context context, List<SupportEntity> list) {
        this.context = context;
        this.list = list;
    }
    @Override
    public int getCount() {
        return list.size();
    }

    @Override
    public Object getItem(int i) {
        return list.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @Override
    public View getView(int i, View convertView, ViewGroup parent) {
        View view=inflater.inflate(R.layout.wifi_listitem, null);
        SupportEntity support = list.get(i);

        ImageView logo = (ImageView) view.findViewById(R.id.item_partner_logo);
        TextView name = (TextView) view.findViewById(R.id.item_partner_name);
        TextView desc = (TextView) view.findViewById(R.id.item_partner_desc);
        ImageView rightImg = (ImageView) view.findViewById(R.id.item_partner_auxiv);
        ToggleButton toggle = (ToggleButton) view.findViewById(R.id.item_partner_tob);

        if (support.getLogoId() > 0) {
            logo.setVisibility(View.VISIBLE);
            logo.setImageResource(support.getLogoId());
        } else {
            logo.setVisibility(View.GONE);
        }
        name.setText(support.getName());

        if (!support.getDesc().isEmpty()) {
            desc.setVisibility(View.VISIBLE);
            rightImg.setVisibility(View.GONE);
            toggle.setVisibility(View.GONE);
            desc.setText(support.getDesc());
        } else if (support.isToggle()){
            toggle.setVisibility(View.VISIBLE);
            desc.setVisibility(View.GONE);
            rightImg.setVisibility(View.GONE);
        } else if (support.getRightImg() > 0) {
            rightImg.setVisibility(View.VISIBLE);
            toggle.setVisibility(View.GONE);
            desc.setVisibility(View.GONE);
            rightImg.setImageResource(support.getRightImg());
        } else {
            rightImg.setVisibility(View.VISIBLE);
            toggle.setVisibility(View.GONE);
            desc.setVisibility(View.GONE);
            rightImg.setImageResource(R.drawable.ic_close_white);
        }
        return view;
    }
}
