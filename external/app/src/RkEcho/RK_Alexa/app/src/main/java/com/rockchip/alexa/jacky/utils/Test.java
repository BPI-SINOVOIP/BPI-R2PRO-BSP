package com.rockchip.alexa.jacky.utils;

import java.io.IOException;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by Administrator on 2017/5/11.
 */

public class Test
{
    public enum TEST {
        START,
        STOP
    };

    private static boolean running = true;
    public static void main(String[] args) {

    }

    public void test(Thread thread)  {
        System.out.println("it is running");
        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("it is end");
    }

    private  Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            try {
                Thread.sleep(3000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    });
//    public static void main(String[] args) {
//        List<String> list = new ArrayList<String>();
//        list.add("Selected interface 'wlan0'");
//        list.add("bssid=1c:fa:68:83:76:9e");
//        list.add("freq=2437");
//        list.add("ssid=hxw@rk");
//        list.add("id=0");
//        list.add("mode=station");
//        list.add("pairwise_cipher=CCMP");
//        list.add("group_cipher=TKIP");
//        list.add("key_mgmt=WPA2-PSK");
//        list.add("wpa_state=COMPLETED");
//        list.add("ip_address=172.16.22.128");
//        list.add("address=b0:f1:ec:2d:9c:3c");
//
//        for (String str : list) {
//            if (str.startsWith("wpa_state")) {
//                String te = str.substring(str.indexOf("=") + 1);
//                System.out.println("test:" + te);
//            }
//            if (str.startsWith("ip_address")) {
//                String te = str.substring(str.indexOf("=") + 1);
//            }
//        }
//
//
//        Map<TEST, Integer> map = new HashMap<>();
//
//        map.put(TEST.START, 1);
//        System.out.println("TEST.START:" + TEST.START);
//        System.out.println(map.get(TEST.START));
//        SimpleDateFormat format = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS");
//        System.out.println(format.format(new Date(1494652391353L)));
//        System.out.println(format.format(new Date(System.currentTimeMillis())));
//        System.out.println(Context.isAuthorized);
//        try {
//            throw new IOException("TEST");
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//    }
}
