package com.rockchip.alexa.jacky.info;

public class BluInfo {
    public static final String NAME = "name";
    public static final String ADDR = "addr";
    public static final String PAIRED = "paired";
    public static final String CONNECTED = "connected";

    private String tag;

    private String name;
    private String addr;
    private boolean isPaired;
    private boolean isConnected;

    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getAddr() {
        return addr;
    }

    public void setAddr(String addr) {
        this.addr = addr;
    }

    public boolean isPaired() {
        return isPaired;
    }

    public void setPaired(boolean isPaired) {
        this.isPaired = isPaired;
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("{\"name\":\"").append(name).append("\",\"addr\":\"").append(addr).append("\",\"paired\":\"").append(isPaired ? 1 : 0).append("\"}");
        return builder.toString();
    }
}
