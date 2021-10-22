package com.rockchip.alexa.jacky.utils;

import android.content.Context;

import com.coolerfall.download.DownloadCallback;
import com.coolerfall.download.DownloadManager;
import com.coolerfall.download.DownloadRequest;
import com.coolerfall.download.OkHttpDownloader;
import com.coolerfall.download.Priority;

import java.util.concurrent.TimeUnit;

/**
 * Created by cjs on 2017/5/5.
 */

public class HttpDownloader {
    private Context mContext;
    private int mDownloadId;
    private DownloadManager mDownloadManager;

    public HttpDownloader(Context context){
        mContext = context;
        mDownloadManager = new DownloadManager.Builder().context(mContext)
                .downloader(OkHttpDownloader.create())
                .threadPoolSize(2)
                .build();
    }

    public int startDownload(String downloadUrlStr,String destPath,String fileName,DownloadCallback downloadCallback){
        // prepare download
        DownloadRequest request = new DownloadRequest.Builder()
                .url(downloadUrlStr)
                .retryTime(3)
                .retryInterval(2, TimeUnit.SECONDS)
                .progressInterval(1, TimeUnit.SECONDS)
                .priority(Priority.HIGH)
                .allowedNetworkTypes(DownloadRequest.NETWORK_WIFI)
                .destinationFilePath(destPath + fileName)
                .downloadCallback(downloadCallback)
                .build();
        mDownloadId = mDownloadManager.add(request);
        return mDownloadId;
    }

    public void cancelTask(){
        if(mDownloadId != 0){
            mDownloadManager.cancel(mDownloadId);
            mDownloadId = 0;
        }
    }
}
