package com.rockchip.smart.rockhome;

public class WifiInfo implements Comparable<WifiInfo> {
    public static final String BSSID = "bssid";
    public static final String FREQUENCY = "frequency";
    public static final String SIGNALLEVEL = "signalLevel";
    public static final String FLAGS = "flags";
    public static final String SSID = "ssid";

    private boolean mIsConnecting;
    private boolean mIsConnected;

    private String bssid;
    private String frequency;
    private String signalLevel;
    private String flags;
    private String ssid;

    public String getBssid() {
        return bssid;
    }
    public void setBssid(String bssid) {
        this.bssid = bssid;
    }
    public String getFrequency() {
        return frequency;
    }
    public void setFrequency(String frequency) {
        this.frequency = frequency;
    }
    public String getSignalLevel() {
        return signalLevel;
    }
    public void setSignalLevel(String signalLevel) {
        this.signalLevel = signalLevel;
    }
    public String getFlags() {
        return flags;
    }
    public void setFlags(String flags) {
        this.flags = flags;
    }
    public String getSsid() {
        return ssid;
    }
    public void setSsid(String ssid) {
        this.ssid = ssid;
    }

    public boolean isConnecting() {
        return mIsConnecting;
    }

    public void setConnecting(boolean isConnecting) {
        this.mIsConnecting = isConnecting;
    }

    public boolean isConnected() {
        return mIsConnected;
    }

    public void setConnected(boolean isConnected) {
        this.mIsConnected = isConnected;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("{\"bssid\":\"").append(bssid).append(",\"frequency\":").append(frequency)
                .append(",\"signalLevel\":").append(signalLevel)
                .append(",\"flags\":").append(flags)
                .append(",\"ssid\":").append(ssid)
                .append("}");
        return builder.toString();
    }

    @Override
    public int compareTo(WifiInfo wifiInfo) {
        try {
            int level = Integer.parseInt(signalLevel);
            int compLevel = Integer.parseInt(wifiInfo.getSignalLevel());
            return level == compLevel ? 0 : (level > compLevel ? -1 : 1);
        } catch (Exception e) {

        }
        return 0;
    }
}
