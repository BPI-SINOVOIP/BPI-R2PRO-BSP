package com.rockchip.alexa.jacky.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.alexa.jacky.R;
import com.rockchip.alexa.jacky.adapter.BluListAdapter;
import com.rockchip.alexa.jacky.app.Context;
import com.rockchip.alexa.jacky.info.BluInfo;
import com.rockchip.alexa.jacky.info.ProvisioningClient;
import com.rockchip.alexa.jacky.listener.OnSingleClickListener;
import com.rockchip.alexa.jacky.listener.OnSingleDialogListener;
import com.rockchip.alexa.jacky.listener.OnSingleItemClickListener;
import com.rockchip.alexa.jacky.utils.Env;
import com.rockchip.alexa.jacky.views.LoadingView;

import org.json.JSONException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class BluListActivity extends Activity {
	public static final String TAG = "BluListActivity";
	private ListView mListView;
	private TextView mBtnBack;
	private LoadingView mLoadingView;
	private List<BluInfo> mBluList;

	private BluListAdapter mListAdapter;

	private long mInitialTime;
	private boolean mCompleted;

	private boolean mActivityStoped;

	private String mConnected;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_blu_list);

		initViews();
		initGlobals();
		initViewListeners();
	}

	private void initViews() {
		mListView = (ListView) findViewById(R.id.blu_list);
		mBtnBack = (TextView) findViewById(R.id.toolbar_back);

		mLoadingView = new LoadingView(this, getResources().getString(R.string.dialog_loading));
	}

	private void initGlobals() {

	}

	private void initViewListeners() {
		mListView.setOnItemClickListener(new OnSingleItemClickListener() {
			@Override
			protected void onSingleItemClick(final AdapterView<?> adapterView, View view, final int position, long id) {
				new Thread(new Runnable() {
					@Override
					public void run() {
						final boolean result = ProvisioningClient.getProvisioningClient(BluListActivity.this).postBluSetupInfo(mBluList.get(position).getName(), mBluList.get(position).getAddr(), mConnected);
						runOnUiThread(new Runnable() {
							@Override
							public void run() {
								Toast.makeText(BluListActivity.this, result ? getResources().getString(R.string.request_send_success) : getResources().getString(R.string.request_send_fail), Toast.LENGTH_SHORT).show();
							}
						});
					}
				}).start();
			}
		});

		mBtnBack.setOnClickListener(new OnSingleClickListener() {
			@Override
			protected void onSingleClick(View v) {
				BluListActivity.this.finish();
			}
		});
	}

	@Override
	protected void onStart() {
		super.onStart();
		mActivityStoped = false;
		if (!Env.isWifiEnable(this) || !Env.isWifiConnected(this) || !Env.getConnectingSSID(this).startsWith(Context.HOTSPOT_PREFIX)) {
			AlertDialog.Builder alert=new AlertDialog.Builder(BluListActivity.this);
			alert.setTitle(getResources().getString(R.string.title_prompt));
			alert.setMessage(getResources().getString(R.string.device_not_connected));

			alert.setPositiveButton(getResources().getString(R.string.confirm), new OnSingleDialogListener() {
				@Override
				protected void onSingleClick(View v) {
					BluListActivity.this.finish();
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
					getBluList();
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

	public void getBluList() {
		try {
			mBluList = ProvisioningClient.getProvisioningClient(this).getBluListInfo();
			List<BluInfo> paired = new ArrayList<>();
			List<BluInfo> scan = new ArrayList<>();

			BluInfo bluConnected = null;
			for (int i=0; i<mBluList.size(); i++) {
				if (mBluList.get(i).isConnected()) {
					mConnected = mBluList.get(i).getAddr();
					bluConnected = mBluList.get(i);
					continue;
				}
				if (mBluList.get(i).isPaired()) {
					paired.add(mBluList.get(i));
				} else {
					scan.add(mBluList.get(i));
				}
			}
			mBluList.clear();
			if (bluConnected != null) {
				BluInfo bluConn = new BluInfo();
				bluConn.setTag(getResources().getString(R.string.blu_tag_connected));
				mBluList.add(bluConn);
				mBluList.add(bluConnected);
			}

			BluInfo bluPaired = new BluInfo();
			bluPaired.setTag(getResources().getString(R.string.blu_tag_paired));
			mBluList.add(bluPaired);
			mBluList.addAll(paired);

			BluInfo bluScan = new BluInfo();
			bluScan.setTag(getResources().getString(R.string.blu_tag_scaned));
			mBluList.add(bluScan);
			mBluList.addAll(scan);


			mListAdapter = new BluListAdapter(this, mBluList);

			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					mCompleted = true;

					if (mLoadingView != null && mLoadingView.isShowing()) {
						mLoadingView.dismiss();
					}

					if (mBluList.size() > 0) {
						mListView.setAdapter(mListAdapter);
					}
				}
			});
		} catch (JSONException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
