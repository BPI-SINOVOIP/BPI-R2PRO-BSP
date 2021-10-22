package com.rockchip.alexa.jacky.utils;

import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

/**
 * Created by cjs on 2017/4/20.
 */
public class FileUtil {

    private static final String TAG = "FileUtil";

    /**
     *
     * @param zipFile
     * @param folderPath
     * @throws IOException
     */
    public static boolean unZipFile(String zipFile, String folderPath)
            throws IOException {
        File rootDirectory = new File(folderPath);
        if(!rootDirectory.exists()){
            Log.d(TAG, "unzip mkdir: " + folderPath);
            rootDirectory.mkdir();
        }
        ZipFile zFile = new ZipFile(zipFile);
        Enumeration zList = zFile.entries();
        ZipEntry ze;
        byte[] buf = new byte[1024];
        while (zList.hasMoreElements()) {
            ze = (ZipEntry) zList.nextElement();
            if (ze.isDirectory()) {
                Log.d(TAG, "unZipFile: no suitable zipFile..");
                return false;
            }

            File tempFile = new File(folderPath + ze.getName());
            if(!tempFile.exists()){
                tempFile.createNewFile();
            }
            Log.d("upZipFile", "ze.getName() = " + ze.getName());
            OutputStream os = new BufferedOutputStream(new FileOutputStream(
                    folderPath + ze.getName()));
            InputStream is = new BufferedInputStream(zFile.getInputStream(ze));
            int readLen;
            while ((readLen = is.read(buf, 0, 1024)) != -1) {
                os.write(buf, 0, readLen);
            }
            is.close();
            os.close();
        }
        zFile.close();
        return true;
    }


//    /**
//     * unzip a zip file
//     * @param zipFileString zip文件路径
//     * @param outPathString 解压输出路径
//     */
//    public static void unZipFile(String zipFileString,String outPathString) throws IOException{
//        ZipInputStream inZip = null;
//        try {
//            inZip = new ZipInputStream(new FileInputStream(zipFileString));
//            ZipEntry zipEntry;
//            String szName ;
//            File outputDir = new File(outPathString);
//            if(!outputDir.exists()){
//                outputDir.mkdirs();
//            }
//            while((zipEntry = inZip.getNextEntry()) != null) {
//                szName = zipEntry.getName();
//                if(zipEntry.isDirectory()){
//                    // get the folder name of the widget    
//                    szName = szName.substring(0, szName.length() - 1);
//                    File folder = new File(outPathString + File.separator + szName);
//                    folder.mkdirs();
//                }else{
//                    File file = new File(outPathString + File.separator + szName);
//                    file.createNewFile();
//                    // get the output stream of the file    
//                    FileOutputStream out = new FileOutputStream(file);
//                    int len;
//                    byte[] buffer = new byte[1024];
//                    // read (len) bytes into buffer    
//                    while((len = inZip.read(buffer)) != -1){
//                        // write (len) byte from buffer at the position 0    
//                        out.write(buffer,0,len);
//                        out.flush();
//                    }
//                    out.close();
//                }
//            }
//        } finally {
//            try {
//                if(inZip != null){
//                    inZip.close();
//                }
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
//        }
//    }

    /**
     * compress files contains in resFileStrList
     * @param resFileStrList
     * @param zipFilePath
     */
    public static void zipMultiFiles(Collection<String> resFileStrList,String zipFilePath){
        try {
            File zipFile = new File(zipFilePath);
            if(!zipFile.exists()){
                zipFile.createNewFile();
            }
            InputStream input;
            ZipOutputStream zipOut = new ZipOutputStream(new FileOutputStream(zipFile));

            for(String resFileStr:resFileStrList){
                File resFile = new File(resFileStr);
                if(resFile.exists()){
                    input = new FileInputStream(resFile);
                    zipOut.putNextEntry(new ZipEntry(resFile.getName()));
                    int length;
                    byte[] buffer = new byte[4096];
                    while((length = input.read(buffer))!=-1){
                        zipOut.write(buffer,0,length);
                    }
                    input.close();
                }
            }
            zipOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static int convertSizeFromBToMB(long size){
        // 如果字节数少于1024，则直接以B为单位，否则先除以1024
        if(size < 1024){
            return 0;
        }else{
            size = size / 1024;
        }
        // 如果原字节数少于1024，则直接以KB为单位，否则除以1024
        if(size < 1024){
            return 0;
        }else{
            size = size / 1024;
        }
        // 如果原字节数少于1024，则直接以MB为单位
        return (int)size;
    }

    public static int getFileSize(String filePath){
        File file = new File(filePath);
        if(file.isFile() && file.exists()){
            FileInputStream fis = null;
            try {
                fis = new FileInputStream(file);
                return fis.available();
            }catch (IOException e){

            }finally {
                try {
                    fis.close();
                }catch (IOException e){
                    e.printStackTrace();
                }
            }
        }
        return 0;
    }

    public static boolean isFileExit(String filePath){
        return new File(filePath).exists();
    }

}
