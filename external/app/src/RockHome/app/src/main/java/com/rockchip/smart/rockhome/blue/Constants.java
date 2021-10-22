/******************************************************************************
 *
 *  Copyright (C) 2013-2014 Cypress Semiconductor
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
package com.rockchip.smart.rockhome.blue;

import java.util.UUID;

/**
 * Contains the UUID of services, characteristics, and descriptors
 */
public class Constants {
    public static final String TAG_PREFIX = "Home."; //used for debugging

    /**
     * Transmit packet size
     */
    public static final int SIZE_TRANSMIT_PACKET = 134;

    /**
     * UUID of Wifi Service
     */
    public static final UUID WIFI_SERVICE_UUID = UUID
            .fromString("0000180A-0000-1000-8000-00805F9B34FB");

    /**
     * UUID of Wifi Characteristic
     */
    public static final UUID WIFI_CHARACTERISTIC_UUID = UUID
            .fromString("00009999-0000-1000-8000-00805F9B34FB");

    /**
     * UUID of the client configuration descriptor
     */
    public static final UUID CLIENT_CONFIG_DESCRIPTOR_UUID = UUID
            .fromString("00002902-0000-1000-8000-00805f9b34fb");
}
