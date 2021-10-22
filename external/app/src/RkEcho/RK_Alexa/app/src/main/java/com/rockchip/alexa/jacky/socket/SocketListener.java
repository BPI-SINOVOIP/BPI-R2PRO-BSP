package com.rockchip.alexa.jacky.socket;

/**
 * Created by Administrator on 2017/3/20.
 */

public interface SocketListener {
	public void onServerStart();
    public void onConnect(SocketTransceiver addr);
    public void onConnectFail(String ip, int port);
    public void onConnectTimeout(String ip, int port);

    public void onReceive(SocketTransceiver addr, String msg);
    public void onDisconnect(SocketTransceiver addr);
	public void onServerStop();
}
