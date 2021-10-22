package com.rockchip.smart.rockhome;

import android.os.Environment;

import java.io.File;
import java.io.FileOutputStream;

/**
 * Created by GJK on 2018/12/5.
 */

public class Utils {
    public static void saveFile(String fileName, String str) {
        try {
            File file = new File("/mnt/sdcard", fileName);
            if (file.exists()) {
                file.delete();
            }
            file.createNewFile();
            FileOutputStream outStream = new FileOutputStream(file);
            outStream.write(str.getBytes());
            outStream.flush();
            outStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
