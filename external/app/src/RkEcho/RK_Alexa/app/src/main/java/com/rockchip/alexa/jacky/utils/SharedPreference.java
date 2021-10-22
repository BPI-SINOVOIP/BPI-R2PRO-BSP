package com.rockchip.alexa.jacky.utils;

import android.content.Context;
import android.content.SharedPreferences;

import com.rockchip.alexa.jacky.app.BaseApplication;


/**
 * Created by Administrator on 2016/8/4.
 */
public class SharedPreference {

    private static final String PREF_NAME = "rk_echo";
    /**
     * 向SharedPreferences中写入int类型数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param value 值
     */
    public static void putInt(Context context, String name, String key, int value) {
        SharedPreferences.Editor sp = getEditor(context, name);
        sp.putInt(key, value);
        sp.commit();
    }
    
    public static void putInt(Context context, String key, int value) {
        putInt(context, PREF_NAME, key, value);
    }

    public static void putInt(String key, int value) {
        putInt(BaseApplication.getInstance().getApplicationContext(), key, value);
    }

    /**
     * 向SharedPreferences中写入boolean类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param value 值
     */
    public static void putBoolean(Context context, String name, String key, boolean value) {
        SharedPreferences.Editor sp = getEditor(context, name);
        sp.putBoolean(key, value);
        sp.commit();
    }

    public static void putBoolean(Context context, String key, boolean value) {
        putBoolean(context, PREF_NAME, key, value);
    }

    public static void putBoolean(String key, boolean value) {
        putBoolean(BaseApplication.getInstance().getApplicationContext(), key, value);
    }

    /**
     * 向SharedPreferences中写入String类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param value 值
     */
    public static void putString(Context context, String name, String key, String value) {
        SharedPreferences.Editor sp = getEditor(context, name);
        sp.putString(key, value);
        sp.commit();
    }

    public static void putString(Context context, String key, String value) {
        putString(context, PREF_NAME, key, value);
    }

    public static void putString(String key, String value) {
        putString(BaseApplication.getInstance().getApplicationContext(), key, value);
    }

    /**
     * 向SharedPreferences中写入float类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param value 值
     */
    public static void putFloat(Context context, String name, String key, float value) {
        SharedPreferences.Editor sp = getEditor(context, name);
        sp.putFloat(key, value);
        sp.commit();
    }

    public static void putFloat(Context context, String key, float value) {
        putFloat(context, PREF_NAME, key, value);
    }

    public static void putFloat(String key, float value) {
        putFloat(BaseApplication.getInstance().getApplicationContext(), key, value);
    }

    /**
     * 向SharedPreferences中写入long类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param value 值
     */
    public static void putLong(Context context, String name, String key, long value) {
        SharedPreferences.Editor sp = getEditor(context, name);
        sp.putLong(key, value);
        sp.commit();
    }

    public static void putLong(Context context, String key, long value) {
        putLong(context, PREF_NAME, key, value);
    }

    public static void putLong(String key, long value) {
        putLong(BaseApplication.getInstance().getApplicationContext(), key, value);
    }

    /**
     * 从SharedPreferences中读取int类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param defValue 如果读取不成功则使用默认值
     * @return 返回读取的值
     */
    public static int getInt(Context context, String name, String key, int defValue) {
        SharedPreferences sp = getPreferences(context, name);
        int value = sp.getInt(key, defValue);
        return value;
    }

    public static int getInt(Context context, String key, int defalue) {
        return getInt(context, PREF_NAME, key, defalue);
    }

    public static int getInt(String key, int defValue) {
        return getInt(BaseApplication.getInstance().getApplicationContext(), key, defValue);
    }

    /**
     * 从SharedPreferences中读取boolean类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param defValue 如果读取不成功则使用默认值
     * @return 返回读取的值
     */
    public static boolean getBoolean(Context context, String name, String key, boolean defValue) {
        SharedPreferences sp = getPreferences(context, name);
        boolean value = sp.getBoolean(key, defValue);
        return value;
    }

    public static boolean getBoolean(Context context, String key, boolean defalue) {
        return getBoolean(context, PREF_NAME, key, defalue);
    }

    public static boolean getBoolean(String key, boolean defValue) {
        return getBoolean(BaseApplication.getInstance().getApplicationContext(), key, defValue);
    }

    /**
     * 从SharedPreferences中读取String类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param defValue 如果读取不成功则使用默认值
     * @return 返回读取的值
     */
    public static String getString(Context context, String name, String key, String defValue) {
        SharedPreferences sp = getPreferences(context, name);
        String value = sp.getString(key, defValue);
        return value;
    }

    public static String getString(Context context, String key, String defalue) {
        return getString(context, PREF_NAME, key, defalue);
    }

    public static String getString(String key, String defValue) {
        return getString(BaseApplication.getInstance().getApplicationContext(), key, defValue);
    }

    /**
     * 从SharedPreferences中读取float类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param defValue 如果读取不成功则使用默认值
     * @return 返回读取的值
     */
    public static float getFloat(Context context, String name, String key, float defValue) {
        SharedPreferences sp = getPreferences(context, name);
        float value = sp.getFloat(key, defValue);
        return value;
    }

    public static float getFloat(Context context, String key, float defalue) {
        return getFloat(context, PREF_NAME, key, defalue);
    }

    public static float getFloat(String key, float defValue) {
        return getFloat(BaseApplication.getInstance().getApplicationContext(), key, defValue);
    }

    /**
     * 从SharedPreferences中读取long类型的数据
     *
     * @param context 上下文环境
     * @param name 对应的xml文件名称
     * @param key 键
     * @param defValue 如果读取不成功则使用默认值
     * @return 返回读取的值
     */
    public static long getLong(Context context, String name, String key, long defValue) {
        SharedPreferences sp = getPreferences(context, name);
        long value = sp.getLong(key, defValue);
        return value;
    }

    public static long getLong(Context context, String key, long defalue) {
        return getLong(context, PREF_NAME, key, defalue);
    }

    public static long getLong(String key, long defValue) {
        return getLong(BaseApplication.getInstance().getApplicationContext(), key, defValue);
    }

    private static SharedPreferences.Editor getEditor(Context context, String name) {
        return getPreferences(context, name).edit();
    }

    public static SharedPreferences.Editor getEditor(Context context) {
        return getEditor(context, PREF_NAME);
    }

    private static SharedPreferences getPreferences(Context context, String name) {
        return context.getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    private static SharedPreferences getPreferences(Context context) {
        return getPreferences(context, PREF_NAME);
    }
}
