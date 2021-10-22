package com.rockchip.alexa.jacky.views;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.PowerManager;

/**
 * Created by cjs on 2017/5/6.
 */

public class WakeLockProgressDialog extends ProgressDialog {
    PowerManager.WakeLock mWakeLock;

    public WakeLockProgressDialog(Context context) {
        super(context);
        PowerManager powerManager = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(
                PowerManager.FULL_WAKE_LOCK,"MyWakeLock");
    }

    @Override
    public void show() {
        super.show();
        mWakeLock.acquire();
    }

    @Override
    public void dismiss() {
        super.dismiss();
        if(mWakeLock.isHeld()){
            mWakeLock.release();
        }
    }
}
