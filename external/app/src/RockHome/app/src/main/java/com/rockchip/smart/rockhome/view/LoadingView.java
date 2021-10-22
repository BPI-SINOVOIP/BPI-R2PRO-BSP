package com.rockchip.smart.rockhome.view;

import android.app.Activity;
import android.app.Dialog;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;

import com.rockchip.smart.rockhome.R;

/**
 * 自定义透明的dialog
 */
public class LoadingView extends Dialog {
    private Activity mActivity;
    private TextView mTvContent;
    private String mContent;

    public LoadingView(Activity activity) {
        this(activity, null);
    }

    public LoadingView(Activity activity, String content) {
        super(activity, R.style.LoadingView);
        setContentView(R.layout.loading_view);

        setCancelable(false);

        this.mActivity = activity;
        this.mContent=content;
        initView();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode){
            case KeyEvent.KEYCODE_BACK:
                if(LoadingView.this.isShowing())
                    LoadingView.this.dismiss();
                break;
        }
        return true;
    }

    private void initView(){
        mTvContent = ((TextView) findViewById(R.id.tvcontent));
        if (mContent != null && !mContent.isEmpty()) {
            mTvContent.setVisibility(View.VISIBLE);
            mTvContent.setText(mContent);
        } else {
            mTvContent.setVisibility(View.GONE);
        }
    }

    @Override
    public void show() {
        show(mContent);
    }

    public void show(String content) {
        if (content != null && !content.isEmpty())
            mContent = content;

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mContent != null && !mContent.isEmpty()) {
                    mTvContent.setVisibility(View.VISIBLE);
                    mTvContent.setText(mContent == null ? "" : mContent);
                } else {
                    mTvContent.setVisibility(View.GONE);
                }
                LoadingView.super.show();
            }
        });
    }

    @Override
    public void dismiss() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                LoadingView.super.dismiss();
            }
        });
    }
}