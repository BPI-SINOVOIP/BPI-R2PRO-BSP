/** 
 * Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use this file 
 * except in compliance with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */
package com.rockchip.smart.rockhome.softap.http;

/**
 * A container for the necessary provisioning information about this device.
 */
public class DeviceContextInfo {
    private final String ssid;
    private final String ip;
    private final boolean authorized;

    /**
     * Creates a {@link DeviceContextInfo} object.
     *
     * @param ssid The ssid what connected of this device.
     * @param ip The ip of this device.
     * @param authorized The sessionId associated with this information..
     */
    public DeviceContextInfo(String ssid, String ip, boolean authorized) {
        this.ssid = ssid;
        this.ip = ip;
        this.authorized = authorized;
    }

    /**
     * @return ssid.
     */
    public String getSsid() {
        return ssid;
    }

    /**
     * @return ip.
     */
    public String getIp() {
        return ip;
    }

    /**
     * @return authorized.
     */
    public boolean isAuthorized() {
        return authorized;
    }
}
