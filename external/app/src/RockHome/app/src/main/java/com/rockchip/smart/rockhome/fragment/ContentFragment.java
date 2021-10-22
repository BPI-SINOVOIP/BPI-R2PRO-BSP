package com.rockchip.smart.rockhome.fragment;

import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;

/**
 * Created by Konstantin on 22.12.2014.
 */
public class ContentFragment {
    public static final String DEFAULT = "Default";
    public static final String CLOSE = "Close";
    public static final String BLUE = "Blue";
    public static final String WIFI = "Wifi";

    private static volatile ContentFragment mInstance;
    private AppCompatActivity mAppCompatActivity;
    private int mContentId;
    private String mCurrentFragment = DEFAULT;

    public static ContentFragment getInstance(AppCompatActivity appCompatActivity, int id) {
        if (mInstance == null) {
            synchronized (ContentFragment.class) {
                if (mInstance == null) {
                    mInstance = new ContentFragment(appCompatActivity, id);
                }
            }
        }
        return mInstance;
    }

    private ContentFragment(AppCompatActivity appCompatActivity, int id) {
        this.mAppCompatActivity = appCompatActivity;
        this.mContentId = id;
    }

    private Fragment getDefaultFragment() {
        return getFragment("default");
    }

    private Fragment getFragment(String key) {
        Fragment fragment = null;
        if (key.equals(BLUE)) {
            fragment = BlueFragment.newInstance(this);
        } else if (key.equals(WIFI)) {
            fragment = WifiFragment.newInstance(this);
        } else if (key.equals(CLOSE)) {
            fragment = null;
        } else {
            fragment = DefaultFragment.newInstance(this);
        }
        return fragment;
    }

    public String replaceFragment() {
        return replaceFragment(mCurrentFragment);
    }

    public String replaceFragment(String key) {
        Fragment fragment = getFragment(key);
        if (fragment == null)
            return key;

        mCurrentFragment = key;
        mAppCompatActivity.getSupportFragmentManager().beginTransaction().replace(mContentId, fragment).commit();

        return key;
    }

    public void destroy() {
        mAppCompatActivity = null;
        mInstance = null;
    }

    public boolean verifyWifiPwd(String value) {
        if (TextUtils.isEmpty(value))
            return false;

        String regStr = "^([A-Z]|[a-z]|[0-9]|[`~!@#$%^&*()+=_|{}':;',\\\\[\\\\].<>/?~！@#￥%……&*（）——+|{}【】‘；：”“'。，、？]){8,16}$";
        return value.matches(regStr);
    }
}

