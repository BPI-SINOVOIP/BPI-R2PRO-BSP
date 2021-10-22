package com.rockchip.smart.rockhome;

import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;

import com.rockchip.smart.rockhome.fragment.ContentFragment;
import com.rockchip.smart.rockhome.slidemenu.interfaces.Resourceble;
import com.rockchip.smart.rockhome.slidemenu.interfaces.ScreenShotable;
import com.rockchip.smart.rockhome.slidemenu.model.SlideMenuItem;
import com.rockchip.smart.rockhome.slidemenu.util.ViewAnimator;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by GJK on 2018/11/8.
 */

public class RockHomeActivity extends AppCompatActivity {
    private static final String TAG = "RockHomeActivity";

    private ContentFragment mContentFragment;
    private DrawerLayout mDrawerLayout;
    private ActionBarDrawerToggle mActionBarDrawerToggle;
    private List<SlideMenuItem> mSlideMenuList;

    private ViewAnimator mViewAnimator;
    private LinearLayout mLinearLayout;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState, @Nullable PersistableBundle persistentState) {
        super.onCreate(savedInstanceState, persistentState);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_rockhome);
        Log.d(TAG, "onCreate");

        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerLayout.setScrimColor(Color.TRANSPARENT);
        mLinearLayout = (LinearLayout) findViewById(R.id.left_drawer);
        mLinearLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDrawerLayout.closeDrawers();
            }
        });

        setActionBar();
        createMenuList();
        mViewAnimator = new ViewAnimator<>(this,
                mSlideMenuList,
                null,
                mDrawerLayout,
                new ViewAnimator.ViewAnimatorListener() {
            @Override
            public ScreenShotable onSwitch(Resourceble slideMenuItem, ScreenShotable screenShotable, int position) {
                Log.d(TAG, "position:" + slideMenuItem.getName());
                mContentFragment.replaceFragment(slideMenuItem.getName());
                return screenShotable;
            }

            @Override
            public void disableHomeButton() {
            }

            @Override
            public void enableHomeButton() {
                mDrawerLayout.closeDrawers();
            }

            @Override
            public void addViewToContainer(View view) {
                mLinearLayout.addView(view);
            }
        });

        mContentFragment = ContentFragment.getInstance(this, R.id.content_frame);
        mContentFragment.replaceFragment();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        mContentFragment.destroy();
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        mActionBarDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mActionBarDrawerToggle.onConfigurationChanged(newConfig);
    }

    private void setActionBar() {
        mActionBarDrawerToggle = new ActionBarDrawerToggle(
                this,
                mDrawerLayout,
                R.string.drawer_open,
                R.string.drawer_close) {
            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
            }

            @Override
            public void onDrawerSlide(View drawerView, float slideOffset) {
                super.onDrawerSlide(drawerView, slideOffset);
                if (slideOffset > 0.6 && mLinearLayout.getChildCount() == 0)
                    mViewAnimator.showMenuContent();
            }

            @Override
            public void onDrawerClosed(View drawerView) {
                super.onDrawerClosed(drawerView);
                mLinearLayout.removeAllViews();
                mLinearLayout.invalidate();
            }
        };
        mDrawerLayout.setDrawerListener(mActionBarDrawerToggle);
    }

    private List<SlideMenuItem> createMenuList() {
        if (mSlideMenuList == null)
            mSlideMenuList = new ArrayList<>();
        mSlideMenuList.clear();

        SlideMenuItem menuItem0 = new SlideMenuItem(ContentFragment.CLOSE, R.drawable.icn_close);
        mSlideMenuList.add(menuItem0);
        SlideMenuItem menuItem = new SlideMenuItem(ContentFragment.BLUE, R.drawable.icn_blue);
        mSlideMenuList.add(menuItem);
        SlideMenuItem menuItem2 = new SlideMenuItem(ContentFragment.WIFI, R.drawable.icn_wifi);
        mSlideMenuList.add(menuItem2);

        return mSlideMenuList;
    }
}
