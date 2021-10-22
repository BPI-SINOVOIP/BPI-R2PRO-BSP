package com.rockchip.alexa.jacky.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import com.amazon.identity.auth.device.api.authorization.AuthorizationManager;
import com.amazon.identity.auth.device.api.authorization.AuthorizeRequest;
import com.amazon.identity.auth.device.api.authorization.ScopeFactory;
import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.activity.AlexaActivity;
import com.rockchip.alexa.jacky.app.AuthConstants;
import com.rockchip.alexa.jacky.app.BaseApplication;
import com.rockchip.alexa.jacky.app.CodeChallengeWorkflow;
import com.rockchip.alexa.jacky.info.DeviceProvisioningInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.UUID;

/**
 * Created by Administrator on 2017/3/21.
 */

public class AlexaIndexFragment extends NoFragment {
    private static final String TAG = "AlexaIndexFragment";
    private Toolbar mToolbar;
    private TextView mBtnBack;
    private TextView mBtnSkip;

    public Button mLoginBtn;
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.frag_alexa_index, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mToolbar = (Toolbar) view.findViewById(R.id.toolbar);
        mBtnBack = (TextView) view.findViewById(R.id.toolbar_back);
        mBtnSkip = (TextView) view.findViewById(R.id.toolbar_skip);
        mLoginBtn = (Button) view.findViewById(R.id.frag_alexa_index_login);
        mLoginBtn.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                DeviceProvisioningInfo deviceProvisioningInfo = null;
                if (AuthConstants.COMMUNICATE_TYPE.equals(AuthConstants.CommunicateType.SOCKET)) {
                    deviceProvisioningInfo = ((AlexaActivity) getActivity()).getDeviceProvisioningInfo();
                } else if (AuthConstants.COMMUNICATE_TYPE.equals(AuthConstants.CommunicateType.HTTPS)) {
                    deviceProvisioningInfo = new DeviceProvisioningInfo(AuthConstants.DeviceProvisioning.PRODUCT_ID, AuthConstants.DeviceProvisioning.DSN, UUID.randomUUID().toString(),
                            CodeChallengeWorkflow.getInstance().getCodeChallenge(), CodeChallengeWorkflow.getInstance().getCodeChallengeMethod(), CodeChallengeWorkflow.getInstance().getCodeVerifier());
                    ((AlexaActivity) getActivity()).setDeviceProvisioningInfo(deviceProvisioningInfo);
                }
                if (deviceProvisioningInfo != null) {
                    JSONObject scopeData = new JSONObject();
                    final JSONObject productInstanceAttributes = new JSONObject();
                    final String codeChallenge = deviceProvisioningInfo.getCodeChallenge();
                    final String codeChallengeMethod = deviceProvisioningInfo.getCodeChallengeMethod();

                    try {
                        productInstanceAttributes.put(AuthConstants.DEVICE_SERIAL_NUMBER, BaseApplication.getApplication().getDsn());
                        scopeData.put(AuthConstants.PRODUCT_INSTANCE_ATTRIBUTES, productInstanceAttributes);
                        scopeData.put("productID", deviceProvisioningInfo.getProductId());

                        AuthorizationManager.authorize(new AuthorizeRequest.Builder(((AlexaActivity) getActivity()).getRequestContext())
                                .addScope(ScopeFactory.scopeNamed(AuthConstants.ALEXA_ALL_SCOPE, scopeData))
                                .forGrantType(AuthorizeRequest.GrantType.AUTHORIZATION_CODE)
                                .withProofKeyParameters(codeChallenge, codeChallengeMethod)
                                .build());
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }
            }
        });

        mBtnBack.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                startFragment(SupportsFragment.class, false);
            }
        });

        mBtnSkip.setOnClickListener(new OnSingleClickListener() {
            @Override
            protected void onSingleClick(View view) {
                startFragment(SupportsFragment.class, false);
            }
        });
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        ((AlexaActivity)getActivity()).setFragment(this);
        setToolbar(mToolbar);

        // Set title for toolbar:
//        setTitle("Alexa");

        // Display close button.
//        displayHomeAsUpEnabled(R.drawable.ic_toolbar_back);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
//        inflater.inflate(R.menu.menu_fragment_main, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }
}
