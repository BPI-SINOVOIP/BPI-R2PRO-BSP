package com.rockchip.alexa.jacky.listener;

import android.os.SystemClock;
import android.view.View;
import android.widget.AdapterView;

/**
 * Created by Administrator on 2017/5/4.
 */

public abstract class OnSingleItemClickListener implements AdapterView.OnItemClickListener {

    public static final int MIN_CLICK_DELAY_TIME = 2000;
    private long lastClickTime = 0;

    @Override
    public synchronized void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
        long currentTime = SystemClock.uptimeMillis();
        if (currentTime - lastClickTime > MIN_CLICK_DELAY_TIME) {
            lastClickTime = currentTime;
            onSingleItemClick(adapterView, view, position, id);
        }
    }

    protected abstract void onSingleItemClick(AdapterView<?> adapterView, View view, int position, long id);
}
