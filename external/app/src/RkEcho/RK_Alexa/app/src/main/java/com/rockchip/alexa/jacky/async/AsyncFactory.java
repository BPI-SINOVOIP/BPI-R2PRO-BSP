package com.rockchip.alexa.jacky.async;

/**
 * Created by Administrator on 2017/4/27.
 */

public class AsyncFactory {

    private static AsyncThread mAsyncThread;

    public static synchronized AsyncThread getAsyncThread() {
        if (mAsyncThread == null)
            mAsyncThread = new AsyncThread();
        return mAsyncThread;
    }
}
