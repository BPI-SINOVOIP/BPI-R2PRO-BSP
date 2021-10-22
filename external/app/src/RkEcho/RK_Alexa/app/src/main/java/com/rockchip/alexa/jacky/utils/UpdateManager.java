package com.rockchip.alexa.jacky.utils;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.util.SparseArray;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;


/**
 * Created by cjs on 2017/5/8.
 */

public class UpdateManager{
    private static final String TAG = "RK_Alexa";

    public static String FIRMWARE_PATH;
    public static String UNZIP_FIRMWARE_PATH;
    public static String FIRMWARE_NAME = "firmware.zip";

    /**
     * the key of sharePreference about firmware download
     */
    public static final String KEY_DOWNLOAD_STATE           = "download_state";
    public static final String KEY_DOWNLOADING_VERSION_STR  = "download_version_str";
    public static final String KEY_DEVICE_VERSION_STR       = "device_version_str";
    public static final String KEY_DOWNLOAD_FIRMWARE_LENGTH = "download_firmware_length";
    public static final String KEY_DOWNLOAD_UPDATE_ID       = "download_update_id";
    public static final String KEY_DOWNLOAD_UPDATE_ID_TIME  = "download_update_id_time";

    /**
     * download state that will saved in sharePreference
     */
    public static final int DOWNLOAD_STATE_NONE            = 0;
    public static final int DOWNLOAD_STATE_DOWNLOADING     = 1;
    public static final int DOWNLOAD_STATE_DOWNLOADED      = 2;
    public static final int DOWNLOAD_STATE_DATA_PREPARED   = 3;

    /**
     * the key of five images
     */
    public static final int KEY_IMG_ROOTFS   = 0;
    public static final int KEY_IMG_DATA     = 1;
    public static final int KEY_IMG_UBOOT    = 2;
    public static final int KEY_IMG_KERNEL   = 3;
    public static final int KEY_IMG_RESOURCE = 4;

    /**
     * Image name of firmware
     */
    public static final String IMG_NAME_UBOOT    = "uboot.img";
    public static final String IMG_NAME_RESOURCE = "resource.img";
    public static final String IMG_NAME_KERNEL   = "kernel.img";
    public static final String IMG_NAME_ROOTFS   = "rootfs.img";
    public static final String IMG_NAME_DATA     = "data.img";

    private Context mContext;

    private String serverVersionNumber;
    private String serverUpdateInfo;

    private String deviceVersionNumber;

    private static SparseArray<Double> serverImgVersionNumbers;
    private static SparseArray<Double> deviceImgVersionNumbers;

    public UpdateManager(Context context){
        mContext = context;
        Log.d(TAG, "UpdateManager:  setExternalStorageDirectory");
        FIRMWARE_PATH = Environment.getExternalStorageDirectory() + File.separator + "RK_Alexa/";
        UNZIP_FIRMWARE_PATH = Environment.getExternalStorageDirectory() + File.separator +"RK_Alexa/firmware/";

        serverImgVersionNumbers = new SparseArray<>();
        deviceImgVersionNumbers = new SparseArray<>();
        deviceVersionNumber = SharedPreference.getString(context,KEY_DEVICE_VERSION_STR,null);
        if(deviceVersionNumber != null){
            parseDeviceVersionNumber(deviceVersionNumber);
            Log.d(TAG, "get device number version info: " + deviceVersionNumber);
        }else{
            Log.d(TAG, "device number version info is not available.");
        }
    }

    /**
     * when the versionInfo result back form httpClient.
     * 1. save the needed params.
     * 2. compare the info between server and device
     * @param versionInfo
     */
    public void handlerServerString(String versionInfo){
         /* the back result from server contains version number and update information like(next line):
         uboot.img:1.0,resource.img:1.0,kernel.img:1.0,rootfs.img:1.0,data.img:0.13 & updateInfo
         ---divided by '&' between version number and update info
         and in version info it divided by ',' between image version info */
        String versionNumber =  versionInfo.split("&")[0];
        String updateInfo = versionInfo.split("&")[1];
        Log.d(TAG, "get server versionNumber info: " + versionNumber);

        this.serverVersionNumber = versionNumber;
        this.serverUpdateInfo = updateInfo;
        parseServerVersionNumber(serverVersionNumber);
    }

    /**
     * compare five images version number between server firmware versionNumber
     * and device firmware versionNumber.
     * @return isUpgradeable
     */
    public boolean isUpdatable(){
        if(deviceImgVersionNumbers.size() == 0){
            Log.d(TAG, "Device version string don't set,update");
            return true;
        }
        if(serverImgVersionNumbers.size() < 5 || deviceImgVersionNumbers.size() < 5){
            Log.d(TAG, "global application has wrong number of image version saved");
            return true;
        }
        boolean result = false;
        if(serverImgVersionNumbers.get(KEY_IMG_ROOTFS).compareTo(deviceImgVersionNumbers.get(KEY_IMG_ROOTFS)) != 0){
            result = true;
        }
        if(serverImgVersionNumbers.get(KEY_IMG_DATA).compareTo(deviceImgVersionNumbers.get(KEY_IMG_DATA)) != 0){
            result = true;
        }
        if(serverImgVersionNumbers.get(KEY_IMG_UBOOT).compareTo(deviceImgVersionNumbers.get(KEY_IMG_UBOOT)) != 0) {
            result = true;
        }
        if(serverImgVersionNumbers.get(KEY_IMG_KERNEL).compareTo(deviceImgVersionNumbers.get(KEY_IMG_KERNEL)) != 0){
            result = true;
        }
        if(serverImgVersionNumbers.get(KEY_IMG_RESOURCE).compareTo(deviceImgVersionNumbers.get(KEY_IMG_RESOURCE)) != 0){
            result = true;
        }
        return result;
    }

    /**
     * save server versionNumber string into ArrayList info
     * @param versionNumber
     */
    public static void parseServerVersionNumber(String versionNumber){
        /* the backed string get 5 images version number like:
           uboot.img:1.0,resource.img:1.0,kernel.img:1.0,rootfs.img:1.0,data.img:0.13 */
        String[] imgVersionNumbers = versionNumber.split(",");

        for(int i = 0;i < imgVersionNumbers.length; i++){
            String imgName = imgVersionNumbers[i].split(":")[0];
            double imgVersionNumber;
            if(imgName.contains(IMG_NAME_UBOOT)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "server uboot: " + imgVersionNumber);
                serverImgVersionNumbers.put(KEY_IMG_UBOOT,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_RESOURCE)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "server resource: " + imgVersionNumber);
                serverImgVersionNumbers.put(KEY_IMG_RESOURCE,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_KERNEL)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "server kernel: " + imgVersionNumber);
                serverImgVersionNumbers.put(KEY_IMG_KERNEL,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_ROOTFS)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "server rootfs: " + imgVersionNumber);
                serverImgVersionNumbers.put(KEY_IMG_ROOTFS,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_DATA)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "server data: " + imgVersionNumber);
                serverImgVersionNumbers.put(KEY_IMG_DATA,imgVersionNumber);
            }
        }
    }

    /**
     * save device versionNumber string into ArrayList info
     * @param versionNumber
     */
    public static void parseDeviceVersionNumber(String versionNumber){
        /* the backed string get 5 images version number like:
           uboot.img:1.0,resource.img:1.0,kernel.img:1.0,rootfs.img:1.0,data.img:0.13 */
        String[] imgVersionNumbers = versionNumber.split(",");

        for(int i = 0;i < imgVersionNumbers.length; i++){
            String imgName = imgVersionNumbers[i].split(":")[0];
            double imgVersionNumber;
            if(imgName.contains(IMG_NAME_UBOOT)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "device uboot: " + imgVersionNumber + "   server uboot" + serverImgVersionNumbers.get(KEY_IMG_UBOOT));
                deviceImgVersionNumbers.put(KEY_IMG_UBOOT,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_RESOURCE)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "device resource: " + imgVersionNumber + "   server RESOURCE" + serverImgVersionNumbers.get(KEY_IMG_RESOURCE));
                deviceImgVersionNumbers.put(KEY_IMG_RESOURCE,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_KERNEL)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "device kernel: " + imgVersionNumber + "   server kernel" + serverImgVersionNumbers.get(KEY_IMG_KERNEL));
                deviceImgVersionNumbers.put(KEY_IMG_KERNEL,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_ROOTFS)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "device rootfs: " + imgVersionNumber + "   server rootfs" + serverImgVersionNumbers.get(KEY_IMG_ROOTFS));
                deviceImgVersionNumbers.put(KEY_IMG_ROOTFS,imgVersionNumber);
            }else if(imgName.contains(IMG_NAME_DATA)){
                imgVersionNumber = Double.parseDouble(imgVersionNumbers[i].split(":")[1]);
                Log.d(TAG, "device data: " + imgVersionNumber + "   server data" + serverImgVersionNumbers.get(KEY_IMG_DATA));
                deviceImgVersionNumbers.put(KEY_IMG_DATA,imgVersionNumber);
            }
        }
    }

    public static short getUpdateImageBit(String versionNumberStr){
        // save versionNumber string to SharePreference
        Log.d(TAG, "getUpdateImageBit: " + versionNumberStr);
        SharedPreference.putString(UpdateManager.KEY_DEVICE_VERSION_STR,versionNumberStr);

        UpdateManager.parseDeviceVersionNumber(versionNumberStr);

        int imgBit = 0x0000;
        int img_uboot_on = 0x0001,img_resource_on = 0x0002,img_kernel_on = 0x0004;
        int img_rootfs_on = 0x0008,img_data_on = 0x0010;

        if(deviceImgVersionNumbers.get(KEY_IMG_UBOOT).compareTo(serverImgVersionNumbers.get(KEY_IMG_UBOOT)) != 0) {
            imgBit = imgBit | img_uboot_on;
        }
        if(deviceImgVersionNumbers.get(KEY_IMG_RESOURCE).compareTo(serverImgVersionNumbers.get(KEY_IMG_RESOURCE)) != 0){
            imgBit = imgBit | img_resource_on;
        }
        if(deviceImgVersionNumbers.get(KEY_IMG_KERNEL).compareTo(serverImgVersionNumbers.get(KEY_IMG_KERNEL)) != 0){
            imgBit = imgBit | img_kernel_on;
        }
        if(deviceImgVersionNumbers.get(KEY_IMG_ROOTFS).compareTo(serverImgVersionNumbers.get(KEY_IMG_ROOTFS)) != 0){
            imgBit = imgBit | img_rootfs_on;
        }
        if(deviceImgVersionNumbers.get(KEY_IMG_DATA).compareTo(serverImgVersionNumbers.get(KEY_IMG_DATA)) != 0){
            imgBit = imgBit | img_data_on;
        }
        return (short)imgBit;
    }

    /**
     * get update version string and put back to jni.
     * this method also used as a call-back when device update success
     * @return
     */
    public static String getUpdateVersionStr(){
        deviceImgVersionNumbers.put(KEY_IMG_ROOTFS,serverImgVersionNumbers.get(KEY_IMG_ROOTFS));
        deviceImgVersionNumbers.put(KEY_IMG_DATA,serverImgVersionNumbers.get(KEY_IMG_DATA));
        deviceImgVersionNumbers.put(KEY_IMG_UBOOT,serverImgVersionNumbers.get(KEY_IMG_UBOOT));
        deviceImgVersionNumbers.put(KEY_IMG_KERNEL,serverImgVersionNumbers.get(KEY_IMG_KERNEL));
        deviceImgVersionNumbers.put(KEY_IMG_RESOURCE,serverImgVersionNumbers.get(KEY_IMG_RESOURCE));

        StringBuffer sb = new StringBuffer();
        sb.append("uboot.img:").append(deviceImgVersionNumbers.get(KEY_IMG_UBOOT)).append(",");
        sb.append("resource.img:").append(deviceImgVersionNumbers.get(KEY_IMG_RESOURCE)).append(",");
        sb.append("kernel.img:").append(deviceImgVersionNumbers.get(KEY_IMG_KERNEL)).append(",");
        sb.append("rootfs.img:").append(deviceImgVersionNumbers.get(KEY_IMG_ROOTFS)).append(",");
        sb.append("data.img:").append(deviceImgVersionNumbers.get(KEY_IMG_DATA)).append(",");
        Log.d(TAG, "new Updated versionNumber string: " + sb.toString());
        return sb.toString();
    }

    /**
     *  it is used to check download state error
     *  when app quit while downloading or delete fle
     */
    public void reInitDownloadStateByCurrent(){
        int currentDownloadState = SharedPreference.getInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_NONE);
        String downloadVersionStr = SharedPreference.getString(KEY_DOWNLOADING_VERSION_STR,null);
        int hasDownloadLength = FileUtil.getFileSize(FIRMWARE_PATH + FIRMWARE_NAME);
        switch (currentDownloadState){
            case DOWNLOAD_STATE_DOWNLOADED:
                if(downloadVersionStr == null || !downloadVersionStr.equals(serverVersionNumber)){
                    SharedPreference.putInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_NONE);
                }
                break;
            case DOWNLOAD_STATE_DOWNLOADING:
                /* it means that firmware has download finished */
                if(hasDownloadLength == SharedPreference.getInt(mContext,KEY_DOWNLOAD_FIRMWARE_LENGTH,-1)){
                    SharedPreference.putInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_DOWNLOADED);
                }
                break;
            case DOWNLOAD_STATE_NONE:
                break;
            case DOWNLOAD_STATE_DATA_PREPARED:
                if(downloadVersionStr == null || !downloadVersionStr.equals(serverVersionNumber)){
                    SharedPreference.putInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_NONE);
                }else if(!isImgDataReady()){
                    SharedPreference.putInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_DOWNLOADED);
                }
                break;
            default:
                break;
        }

        if(hasDownloadLength == 0 && !isImgDataReady()){
            SharedPreference.putInt(mContext,KEY_DOWNLOAD_STATE,DOWNLOAD_STATE_NONE);
        }
    }

    public static boolean isImgDataReady(){
        ArrayList<Integer> imgNeedUpdate = new ArrayList<>();
        imgNeedUpdate.add(KEY_IMG_DATA);
        imgNeedUpdate.add(KEY_IMG_KERNEL);
        imgNeedUpdate.add(KEY_IMG_RESOURCE);
        imgNeedUpdate.add(KEY_IMG_ROOTFS);
        imgNeedUpdate.add(KEY_IMG_UBOOT);
        for(int i = 0; i < imgNeedUpdate.size(); i++ ){
            File tempFile;
            switch (imgNeedUpdate.get(i)){
                case KEY_IMG_UBOOT:
                    tempFile = new File(UNZIP_FIRMWARE_PATH + IMG_NAME_UBOOT);
                    if(!tempFile.exists()){
                        return false;
                    }
                    break;
                case KEY_IMG_RESOURCE:
                    tempFile = new File(UNZIP_FIRMWARE_PATH + IMG_NAME_RESOURCE);
                    if(!tempFile.exists()){
                        return false;
                    }
                    break;
                case KEY_IMG_KERNEL:
                    tempFile = new File(UNZIP_FIRMWARE_PATH + IMG_NAME_KERNEL);
                    if(!tempFile.exists()){
                        return false;
                    }
                    break;
                case KEY_IMG_ROOTFS:
                    tempFile = new File(UNZIP_FIRMWARE_PATH + IMG_NAME_ROOTFS);
                    if(!tempFile.exists()){
                        return false;
                    }
                    break;
                case KEY_IMG_DATA:
                    tempFile = new File(UNZIP_FIRMWARE_PATH + IMG_NAME_DATA);
                    if(!tempFile.exists()){
                        return false;
                    }
                    break;
            }
        }
        return true;
    }

    public boolean isFirmwareReady(){
        return FileUtil.isFileExit(FIRMWARE_PATH + FIRMWARE_NAME);
    }

    /**
     * this method produce updateId used for tcp connection with device.
     * if the updateId is not same with device,reConnection and transfer.
     * @return updateId
     */
    public long resetUpdateId(){
        /* produce a time correlation long value for update id*/
        return updater_client.getNewUpdateId();
    }

    public String getServerVersionUpdateInfo(){
        return serverUpdateInfo;
    }

    public String getServerVersionNumberStr(){
        return serverVersionNumber;
    }

    public void initPathFileBeforeDownload(){
        File tempFile = new File(FIRMWARE_PATH);
        if(!tempFile.exists()){
            tempFile.mkdir();
        }
        File tempFile2 = new File(FIRMWARE_PATH + FIRMWARE_NAME);
        if(tempFile2.isFile() && tempFile2.exists()){
            tempFile2.delete();
        }
    }

}
