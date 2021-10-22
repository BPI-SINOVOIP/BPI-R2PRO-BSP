package com.rockchip.smart.rockhome.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.rockchip.smart.rockhome.R;

/**
 * Created by GJK on 2018/11/9.
 */

public class DefaultFragment extends Fragment implements View.OnClickListener {
    private static final String TAG = DefaultFragment.class.getSimpleName();
    private static ContentFragment mContentFragment;
    private Button mBtContinue;

    public static DefaultFragment newInstance(ContentFragment contentFragment) {
        mContentFragment = contentFragment;
        DefaultFragment fragment = new DefaultFragment();

        return fragment;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_activity_rockhome_default, container, false);
        mBtContinue = (Button) rootView.findViewById(R.id.fragment_activity_rockhome_default_bt_continue);
        mBtContinue.setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.fragment_activity_rockhome_default_bt_continue:
                mContentFragment.replaceFragment(ContentFragment.BLUE);
                break;
        }
    }
}
