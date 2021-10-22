package com.rockchip.alexa.jacky.socket;

/**
 * TCP Socket服务器端
 * 
 * @author Jacky
 * @since 2017-3-13
 */
public class TcpClient implements SocketListener {

	private	SocketTransceiver transceiver; 
	public TcpClient() {
		transceiver = new SocketTransceiver(this);
	}

	public void connect(String ip, int port) {
		if (transceiver != null) {
			transceiver.connect(ip, port);
		}
	}

	public void send(String msg) {
		if (transceiver != null) {
			transceiver.send(msg);
		}
	}

	@Override
	public void onServerStart() {
		System.out.println("onServerStart");
	}
	
	@Override
	public void onConnect(SocketTransceiver addr) {
		System.out.println("onConnect");
	}
	
	@Override
	public void onConnectFail(String ip, int port) {
		System.out.println("onConnectFail");
	}
	
	@Override
	public void onConnectTimeout(String ip, int port) {
		System.out.println("onConnectTimeout");
	}

	@Override
	public void onReceive(SocketTransceiver addr, String msg) {
		System.out.println("onReceive");
	}
	
	@Override
	public void onDisconnect(SocketTransceiver addr) {
		System.out.println("onDisconnect");
	}
	
	@Override
	public void onServerStop() {
		System.out.println("onServerStop");
	}
}
