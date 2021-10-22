package com.rockchip.alexa.jacky.utils;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

/**
 * Created by cjs on 2017/4/13.
 * use httpUrlConnection to connect
 */
public class HttpClientUtil {

    public static void  requestGet(final String urlString,final RequestStateListener stateListener) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    URL requestUrl = new URL(urlString);
                    HttpURLConnection urlConn = (HttpURLConnection) requestUrl.openConnection();
                    // 设置连接超时时间
                    urlConn.setConnectTimeout(5 * 1000);
                    // 设置从主机读取数据超时时间
                    urlConn.setReadTimeout(5 * 1000);
                    // 设置为Get请求
                    urlConn.setRequestMethod("GET");
                    urlConn.connect();
                    // 判断请求是否成功
                    if (urlConn.getResponseCode() == 200) {
                        // 解析返回的数据
                        stateListener.onRequestResultReceived(urlConn.getInputStream());
                    } else {
                        stateListener.onRequestFail("With error code: " + urlConn.getResponseCode());
                    }
                } catch (MalformedURLException e) {
                    stateListener.onRequestFail("Exception: MalformedURLException");
                    e.printStackTrace();
                } catch (IOException e){
                    stateListener.onRequestFail("Exception: IOException");
                    e.printStackTrace();
                }
            }
        }).start();
    }

    /**
     * 将输入流转换成字符串
     *
     * @param is 从网络获取的输入流
     * @return
     */
    public static String streamToString(InputStream is) {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int len ;
            while ((len = is.read(buffer)) != -1) {
                baos.write(buffer, 0, len);
            }
            baos.close();
            is.close();
            byte[] byteArray = baos.toByteArray();
            return new String(byteArray,"UTF-8");
        } catch (Exception e) {
            return null;
        }
    }

    public interface RequestStateListener{
        void onRequestResultReceived(InputStream is);
        void onRequestFail(String failString);
    }


}
