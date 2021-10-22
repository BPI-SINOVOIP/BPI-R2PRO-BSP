package com.rockchip.alexa.jacky.listener;

import android.os.SystemClock;
import android.view.View;

/**
 * Created by Administrator on 2017/5/4.
 */

public abstract class OnSingleClickListener implements View.OnClickListener {

    public static final int MIN_CLICK_DELAY_TIME = 2000;
    private long lastClickTime = 0;

    @Override
    public synchronized void onClick(View view) {
        long currentTime = SystemClock.uptimeMillis();
        if (currentTime - lastClickTime > MIN_CLICK_DELAY_TIME) {
            lastClickTime = currentTime;
            onSingleClick(view);
        }
    }

    protected abstract void onSingleClick(View view);
}
