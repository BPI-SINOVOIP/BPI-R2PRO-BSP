package com.rockchip.alexa.jacky.socket;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.NetworkInterface;
import java.net.SocketException;

/**
 * LocalNet类
 * @author Jacky.ge
 *
 */
public class LocalNet {

    /**
     * IceWee 2017.03.11
     * 获取本地IP列表（针对多网卡情况）
     *
     * @return
     */
    private static List<String> getLocalIPList() {
        List<String> ipList = new ArrayList<String>();
        try {
            Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
            NetworkInterface networkInterface;
            Enumeration<InetAddress> inetAddresses;
            InetAddress inetAddress;
            String ip;
            while (networkInterfaces.hasMoreElements()) {
                networkInterface = networkInterfaces.nextElement();
                inetAddresses = networkInterface.getInetAddresses();
                while (inetAddresses.hasMoreElements()) {
                    inetAddress = inetAddresses.nextElement();
                    if (inetAddress != null && inetAddress instanceof Inet4Address) { // IPV4
                        ip = inetAddress.getHostAddress();
                        ipList.add(ip);
                    }
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        }
        return ipList;
    }

    /** 
     * 获取本机的IP 
     * @return Ip地址 
     */
    public static String getLocalIP() {
        List<String> ipList = getLocalIPList();
        if (ipList == null || ipList.size() == 0)
            return "127.0.0.1";

        String ip;
        int size = ipList.size();
        for (int i=0; i<size; i++) {
            ip = ipList.get(i);
            if (ip.length() > 0 && !ip.trim().equals("127.0.0.1")) {
                return ip;
            }
        }
        return "127.0.0.1";
    }
}
