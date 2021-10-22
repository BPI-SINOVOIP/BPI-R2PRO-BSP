package com.rockchip.alexa.jacky.socket.entity;

import java.io.Serializable;

import com.alibaba.fastjson.JSON;

public class TransEntity implements Serializable {
    private String ip;
    private String port;
    private String crt;

    public void setIp(String ip) {
        this.ip = ip;
    }

    public String getIp() {
        return ip;
    }

    public void setPort(String port) {
        this.port = port;
    }

    public String getPort() {
        return port;
    }

    public void setCrt(String crt) {
        this.crt = crt;
    }

    public String getCrt() {
        return crt;
    }

    @Override
    public String toString() {
        String json = "";
        try {
            json = JSON.toJSONString(this);
        } catch (Exception e) {

        }
        return json;
    }
}
