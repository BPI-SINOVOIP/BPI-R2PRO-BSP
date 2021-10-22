package com.rockchip.alexa.jacky.socket;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ConnectException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
/**
 * Socket收发器 通过Socket发送数据，并使用新线程监听Socket接收到的数据
 * 
 * @author jacky
 * @since 2017-3-13
 */
public class SocketTransceiver implements Runnable {

	private static final int DEFAULT_TIMEOUT = 5*1000*1000;
	private SocketListener listener;
	private SocketAddress socketAddress;
	private String ip;
	private int port;
	private int timeout;
	protected Socket socket;
	protected InetAddress addr;
	protected DataInputStream in;
	protected DataOutputStream out;
	private boolean runFlag;


	public SocketTransceiver(SocketListener listener, Socket socket) {
		this.listener = listener;
		this.socket = socket;
		addr = socket.getInetAddress();
		try {
			in = new DataInputStream(socket.getInputStream());
			out = new DataOutputStream(socket.getOutputStream());
			start();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	/**
	 * 实例化
	 * 
	 * @param socket
	 *            已经建立连接的socket
	 */
	public SocketTransceiver(SocketListener listener) {
		this.listener = listener;
	}

	public SocketTransceiver(SocketListener listener, String ip, int port, int timeout) {
		this.listener = listener;
		this.ip = ip;
		this.port = port;
		this.timeout = timeout;
		this.socket = new Socket();
		this.socketAddress = new InetSocketAddress(ip, port);
		this.addr = socket.getInetAddress();
	}

	public SocketTransceiver(SocketListener listener, String ip, int port) {
		this(listener, ip, port, DEFAULT_TIMEOUT);
	}

	public void connect(final String ip, final int port) {
		connect(ip, port, DEFAULT_TIMEOUT);
	}

	public void connect(final String ip, final int port, final int timeout) {
		if (ip == null) {
			if (listener != null) {
				System.out.println("ip is null");
				listener.onConnectFail(ip, port);
			}
			return;
		}

		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					socket = new Socket();
					socketAddress = new InetSocketAddress(ip, port);
					socket.connect(socketAddress, timeout);
					//socket.setSoTimeout(timeout);

					addr = socket.getInetAddress();
					try {
						in = new DataInputStream(socket.getInputStream());
						out = new DataOutputStream(socket.getOutputStream());
						start();
					} catch (IOException e) {
						e.printStackTrace();
					}

					if (listener != null) {
						listener.onConnect(SocketTransceiver.this);
					}
				} catch (ConnectException e){
					e.printStackTrace();
				} catch (SocketTimeoutException e) {
					if (listener != null) {
						listener.onConnectTimeout(ip, port);
					}
				} catch (IOException e) {
					e.printStackTrace();
					if (listener != null) {
						System.out.println("connect execption");
						listener.onConnectFail(ip, port);
					}
				}
			}
		}).start();
	}

	/**
	 * 获取连接到的Socket地址
	 * 
	 * @return InetAddress对象
	 */
	public InetAddress getInetAddress() {
		return addr;
	}

	/**
	 * 开启Socket收发
	 * <p>
	 * 如果开启失败，会断开连接并回调{@code onDisconnect()}
	 */
	public void start() {
		runFlag = true;
		new Thread(this).start();
	}

	/**
	 * 断开连接(主动)
	 * <p>
	 * 连接断开后，会回调{@code onDisconnect()}
	 */
	public void stop() {
		runFlag = false;
		try {
			socket.shutdownInput();
			in.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * 发送字符串
	 * 
	 * @param s
	 *            字符串
	 * @return 发送成功返回true
	 */
	public boolean send(String s) {
		if (out != null) {
			try {
				byte[] buffer = s.getBytes("GB2312");
				out.write(buffer);
				out.flush();
				return true;
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 监听Socket接收的数据(新线程中运行)
	 */
	@Override
	public void run() {
		while (runFlag) {
			try {
				byte[] buffer = new byte[10000];
				in.read(buffer);
				final String s = new String(buffer, "GB2312").trim();
				if (listener != null) {
					if (s != null && !s.equals("")) {
						listener.onReceive(SocketTransceiver.this, s);
					}
				}
			} catch (IOException e) {
				// 连接被断开(被动)
				e.printStackTrace();
				runFlag = false;
			}
		}
		// 断开连接
		if (in != null) {
			try {
				in.close();
				in = null;
			} catch (IOException e) {

			}
		}
		if (out != null) {
			try {
				out.close();
				out = null;
			} catch (IOException e) {

			}
		}
		if (socket != null) {
			try {
				socket.close();
				socket = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		if (listener != null) {
			listener.onDisconnect(SocketTransceiver.this);
		}
	}
}
