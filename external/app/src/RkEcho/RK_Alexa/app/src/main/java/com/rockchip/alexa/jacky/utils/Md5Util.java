package com.rockchip.alexa.jacky.utils;

import android.util.Log;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;


public class Md5Util {

//    public static String getFileMD5String(String filePath) {
//        String value = null;
//        FileInputStream in = null;
//        File file = new File(filePath);
//        if (!file.exists()) {
//            return null;
//        }
//        try {
//            in = new FileInputStream(file);
//            MappedByteBuffer byteBuffer = in.getChannel().map(FileChannel.MapMode.READ_ONLY, 0, file.length());
//            MessageDigest md5 = MessageDigest.getInstance("MD5");
//            md5.update(byteBuffer);
//            BigInteger bi = new BigInteger(1, md5.digest());
//            value = bi.toString(16);
//        } catch (Exception e) {
//            e.printStackTrace();
//        } finally {
//            if (null != in) {
//                try {
//                    in.close();
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }
//            }
//        }
//        return value.toUpperCase();
//    }

    private static MessageDigest mMessageDigest;
    private static char mHexDigits[] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    static {
        try {
            mMessageDigest = MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException e) {
        }
    }

    public static byte[] getFileMD5String(String filePath){
        InputStream fis = null;
        try {
            fis = new FileInputStream(filePath);
            byte[] buf = new byte[1024];
            int numRead;
            while ((numRead = fis.read(buf)) != -1) {
                mMessageDigest.update(buf, 0, numRead);
            }
        }catch (IOException e){
            e.printStackTrace();
        }finally {
            try {
                fis.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return mMessageDigest.digest();
    }

    private static String bufferToHex(byte bytes[]) {
        return bufferToHex(bytes, 0, bytes.length);
    }

    private static String bufferToHex(byte bytes[], int m, int n) {
        StringBuffer stringbuffer = new StringBuffer(2 * n);
        int k = m + n;
        for (int l = m; l < k; l++) {
            appendHexPair(bytes[l], stringbuffer);
        }
        Log.d("System.out", "bufferToHex: " +stringbuffer.toString().length());
        return stringbuffer.toString();
    }

    private static void appendHexPair(byte bt, StringBuffer stringBuffer) {
        char c0 = mHexDigits[(bt & 0xf0) >> 4]; // 取字节中高 4 位的数字转换
        char c1 = mHexDigits[bt & 0xf]; // 取字节中低 4 位的数字转换
        stringBuffer.append(c0);
        stringBuffer.append(c1);
    }
}
