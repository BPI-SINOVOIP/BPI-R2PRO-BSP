package com.rockchip.alexa.jacky.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.adapter.WifiListAdapter;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.control.WifiControl;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.info.WifiInfo;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.listener.OnSingleDialogListener;
import com.rockchip.alexa.jacky.listener.OnSingleItemClickListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import org.json.JSONException;

import java.io.IOException;
import java.util.List;

public class WifiListActivity extends Activity {
	public static final String TAG = "WifiListActivity";
	private ListView mlistView;
	private TextView mBtnBack;
	private LoadingView mLoadingView;
	protected WifiControl mWifiControl;
	private List<WifiInfo> mWifiList;
	protected String ssid;

	private WifiListAdapter mWifiListAdapter;

	private long mInitialTime;
	private boolean mCompleted;

	private boolean mActivityStoped;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_wifi_list);

		initViews();
		initGlobals();
		initViewListeners();
	}

	private void initViews() {
		mlistView = (ListView) findViewById(R.id.wifi_list);
		mBtnBack = (TextView) findViewById(R.id.toolbar_back);

		mLoadingView = new LoadingView(this, getResources().getString(R.string.dialog_loading));
	}

	private void initGlobals() {
		mWifiControl = new WifiControl(WifiListActivity.this);
	}

	private void initViewListeners() {
		mlistView.setOnItemClickListener(new OnSingleItemClickListener() {
			@Override
			protected void onSingleItemClick(AdapterView<?> adapterView, View view, int position, long id) {
				Intent intent = new Intent(WifiListActivity.this, WifiInputActivity.class);
				intent.putExtra("ssid", mWifiList.get(position).getSsid());
				startActivity(intent);
				WifiListActivity.this.finish();
			}
		});

		mBtnBack.setOnClickListener(new OnSingleClickListener() {
			@Override
			protected void onSingleClick(View v) {
				WifiListActivity.this.finish();
			}
		});
	}

	@Override
	protected void onStart() {
		super.onStart();
		mActivityStoped = false;
		if (!Env.isWifiEnable(this) || !Env.isWifiConnected(this) || !Env.getConnectingSSID(this).startsWith(Context.HOTSPOT_PREFIX)) {
			AlertDialog.Builder alert=new AlertDialog.Builder(WifiListActivity.this);
			alert.setTitle(getResources().getString(R.string.title_prompt));
			alert.setMessage(getResources().getString(R.string.device_not_connected));

			alert.setPositiveButton(getResources().getString(R.string.confirm), new OnSingleDialogListener() {
				@Override
				protected void onSingleClick(View v) {
					WifiListActivity.this.finish();
				}
			});
			alert.create();
			alert.show();
		} else {
			new Thread(new Runnable() {
				@Override
				public void run() {
					mLoadingView.show();
					mInitialTime = SystemClock.uptimeMillis();
					scanWifi();
					while (!mActivityStoped){
						if (mCompleted) {
							return;
						} else if (!mCompleted && (SystemClock.uptimeMillis() - mInitialTime > 3000)) {

						}
						try {
							Thread.sleep(100);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}).start();
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onStop() {
		super.onStop();
		mActivityStoped = true;
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	public void scanWifi() {
//		mWifiControl.startScan(WifiListActivity.this);
		while (!mActivityStoped){
			try {
				Log.d(TAG, "========getWifiListInfo start========");
				mWifiList = ProvisioningClient.getProvisioningClient(this).getWifiListInfo();
				Log.d(TAG, "========getWifiListInfo end " + mWifiList.size() + "========");
			} catch (JSONException e) {
				Log.d(TAG, "GetWifiListInfo JSONException. ", e);
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						Toast.makeText(WifiListActivity.this, "GetWifiListInfo JSONException.", Toast.LENGTH_SHORT).show();
					}
				});
			} catch (IOException e) {
				Log.d(TAG, "GetWifiListInfo IOException. ", e);
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						Toast.makeText(WifiListActivity.this, "GetWifiListInfo IOException.", Toast.LENGTH_SHORT).show();
					}
				});
			}
			if (mWifiList != null) {
				if (mWifiList.size() > 1) {
					mWifiListAdapter = new WifiListAdapter(this, mWifiList);
					runOnUiThread(new Runnable() {
						@Override
						public void run() {
							mCompleted = true;
							mlistView.setAdapter(mWifiListAdapter);
							if (mLoadingView != null && mLoadingView.isShowing()) {
								mLoadingView.dismiss();
							}
						}
					});
					return;
				}
			}
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
			}
		}
	}
}
