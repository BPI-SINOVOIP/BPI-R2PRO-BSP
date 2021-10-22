package com.rockchip.alexa.jacky.utils;

/**
 * Created by Administrator on 2017/3/24.
 */

public class IPAddress {

    public static int ipToInt(String strIp) {
        int[] ip = new int[4];
        int position1 = strIp.indexOf(".");
        int position2 = strIp.indexOf(".", position1 + 1);
        int position3 = strIp.indexOf(".", position2 + 1);

        ip[0] = Integer.parseInt(strIp.substring(0, position1));
        ip[1] = Integer.parseInt(strIp.substring(position1 + 1, position2));
        ip[2] = Integer.parseInt(strIp.substring(position2 + 1, position3));
        ip[3] = Integer.parseInt(strIp.substring(position3 + 1));
        return (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3];
    }

    public static String intToIP(int intIp) {
        StringBuffer sb = new StringBuffer("");
        sb.append(String.valueOf((intIp >>> 24)));
        sb.append(".");
        sb.append(String.valueOf((intIp & 0x00FFFFFF) >>> 16));
        sb.append(".");
        sb.append(String.valueOf((intIp & 0x0000FFFF) >>> 8));
        sb.append(".");
        sb.append(String.valueOf((intIp & 0x000000FF)));
        return sb.toString();
    }
}
