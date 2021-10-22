package com.rockchip.alexa.jacky.socket;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

/**
 * TCP Socket服务器端
 * 
 * @author Jacky
 * @since 2017-3-13
 */
public class TcpServer implements Runnable, SocketListener {

	private int port;
	private boolean runFlag;
	private List<SocketTransceiver> clients = new ArrayList<SocketTransceiver>();
	private SocketListener mSocketListener;

	/**
	 * 实例化
	 * 
	 * @param port
	 *            监听的端口
	 */
	public TcpServer(int port) {
		this.port = port;
	}

	public void setSocketListener(SocketListener socketListener) {
		this.mSocketListener = socketListener;
	}

	/**
	 * 启动服务器
	 * <p>
	 * 如果启动失败，会回调{@code onServerStop()}
	 */
	public void start() {
		runFlag = true;
		new Thread(this).start();
	}

	/**
	 * 停止服务器
	 * <p>
	 * 服务器停止后，会回调{@code onServerStop()}
	 */
	public void stop() {
		runFlag = false;
	}

	/**
	 * 监听端口，接受客户端连接(新线程中运行)
	 */
	@Override
	public void run() {
		try {
			if (mSocketListener != null) {
				mSocketListener.onServerStart();
			} else {
				onServerStart();
			}
			final ServerSocket server = new ServerSocket(port);
			while (runFlag) {
				try {
					final Socket socket = server.accept();
					startClient(socket);
				} catch (IOException e) {
					// 接受客户端连接出错
					e.printStackTrace();
					this.onConnectFail("127.0.0.1", port);
				}
			}
			// 停止服务器，断开与每个客户端的连接
			try {
				for (SocketTransceiver client : clients) {
					client.stop();
				}
				clients.clear();
				server.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (IOException e) {
			// ServerSocket对象创建出错，服务器启动失败
			e.printStackTrace();
		}
		if (mSocketListener != null) {
			mSocketListener.onServerStop();
		} else {
			onServerStop();
		}
	}

	/**
	 * 启动客户端收发
	 * 
	 * @param socket
	 */
	private void startClient(final Socket socket) {
		SocketTransceiver client = new SocketTransceiver(mSocketListener != null ? mSocketListener : this, socket);
		client.start();
		clients.add(client);
		if (mSocketListener != null) {
			mSocketListener.onConnect(client);
		} else {
			onConnect(client);
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
		System.out.println("onReceive. msg:" + msg);
		addr.send(msg);
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
