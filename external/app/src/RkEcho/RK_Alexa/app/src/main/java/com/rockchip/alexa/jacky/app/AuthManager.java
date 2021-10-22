package com.rockchip.alexa.jacky.app;

import com.rockchip.alexa.jacky.info.CompanionProvisioningInfo;

/**
 * Created by Administrator on 2017/4/7.
 */

public class AuthManager {

    private static AuthManager mAuthManager;
    private CompanionProvisioningInfo mCompanionProvisioningInfo;

    private long mAuthTime;

    public static AuthManager getAuthManager() {
        if (mAuthManager == null) {
            mAuthManager = new AuthManager();
        }
        return mAuthManager;
    }

    private AuthManager() {

    }

    public CompanionProvisioningInfo getCompanionProvisioningInfo() {
        return mCompanionProvisioningInfo;
    }

    public void setCompanionProvisioningInfo(CompanionProvisioningInfo companionProvisioningInfo) {
        this.mCompanionProvisioningInfo = companionProvisioningInfo;
        this.mAuthTime = System.currentTimeMillis();
    }

    public boolean isAuthorized() {
        return (System.currentTimeMillis() - mAuthTime < 8 * 1000 * 60 && mCompanionProvisioningInfo != null);
    }
}
