/**
 * Copyright 2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * You may not use this file except in compliance with the License. A copy of the License is located the "LICENSE.txt"
 * file accompanying this source. This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing permissions and limitations
 * under the License.
 */
package com.rockchip.alexa.jacky.info;

import com.rockchip.alexa.jacky.app.AuthConstants;

import org.json.JSONException;
import org.json.JSONObject;

public class CompanionProvisioningInfo {
    private String sessionId;
    private String clientId;
    private String redirectUri;
    private String authCode;
    private String codeVerifier;

    public CompanionProvisioningInfo(String sessionId, String clientId, String redirectUri, String authCode, String codeVerifier) {
        this.sessionId = sessionId;
        this.clientId = clientId;
        this.redirectUri = redirectUri;
        this.authCode = authCode;
        this.codeVerifier = codeVerifier;
    }

    public String getSessionId() {
        return sessionId;
    }

    public void setSessionId(String sessionId) {
        this.sessionId = sessionId;
    }

    public String getClientId() {
        return clientId;
    }

    public void setClientId(String clientId) {
        this.clientId = clientId;
    }

    public String getRedirectUri() {
        return redirectUri;
    }

    public void setRedirectUri(String redirectUri) {
        this.redirectUri = redirectUri;
    }

    public String getAuthCode() {
        return authCode;
    }

    public void setAuthCode(String authCode) {
        this.authCode = authCode;
    }

    public String getCodeVerifier() {
        return codeVerifier;
    }

    public void setCodeVerifier(String codeVerifier) {
        this.codeVerifier = codeVerifier;
    }

    public JSONObject toJson() {
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put(AuthConstants.AUTH_CODE, authCode);
            jsonObject.put(AuthConstants.CLIENT_ID, clientId);
            jsonObject.put(AuthConstants.REDIRECT_URI, redirectUri);
            jsonObject.put(AuthConstants.SESSION_ID, sessionId);
            jsonObject.put(AuthConstants.CODE_VERIFIER, codeVerifier);
            return jsonObject;
        } catch (JSONException e) {
            return null;
        }
    }
}
