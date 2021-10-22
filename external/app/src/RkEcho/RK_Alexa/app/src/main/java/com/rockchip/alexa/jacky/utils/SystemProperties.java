package com.rockchip.alexa.jacky.utils;

import java.lang.reflect.Method;

public class SystemProperties {
    private static Method setMethod = null;
    private static Method getMethod = null;
    private static Method getIntMethod = null;
    private static Method getLongMethod = null;
    private static Method getBooleanMethod = null;

    /**
     * Set the value for the given key.
     * 
     * @param key
     *            the key to setup
     * @param val
     *            a value to set
     * @return
     */
    public static void set(final String key, final String val) {
        try {
            if (setMethod == null) {
                setMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("set", String.class, String.class);
            }
            setMethod.invoke(null, key, val);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Get the value for the given key
     * 
     * @param key
     *            the key to lookup
     * @param def
     *            a default value to return
     * @return the key parsed as an integer, or def if the key isn't found or
     *         cannot be parsed
     */
    public static String get(final String key, final String def) {
        try {
            if (getMethod == null) {
                getMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("get", String.class, String.class);
            }
            return (String) getMethod.invoke(null, key, def);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return def;
    }

    /**
     * Get the value for the given key
     * 
     * @param key
     *            the key to lookup
     * @param def
     *            a default value to return
     * @return the key parsed as an integer, or def if the key isn't found or
     *         cannot be parsed
     */
    public static int getInt(final String key, final int def) {
        try {
            if (getIntMethod == null) {
                getIntMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("getInt", String.class, Integer.class);
            }
            return (Integer) getIntMethod.invoke(null, key, def);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return def;
    }

    /**
     * Get the value for the given key
     * 
     * @param key
     *            the key to lookup
     * @param def
     *            a default value to return
     * @return the key parsed as an long, or def if the key isn't found or
     *         cannot be parsed
     */
    public static long getLong(final String key, final long def) {
        try {
            if (getLongMethod == null) {
                getLongMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("getLong", String.class, long.class);
            }
            return ((Long) getLongMethod.invoke(null, key, def)).longValue();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return def;
    }

    /**
     * Get the value for the given key
     * 
     * @param key
     *            the key to lookup
     * @param def
     *            a default value to return
     * @return the key parsed as an boolean, or def if the key isn't found or
     *         cannot be parsed
     */
    public static boolean getBoolean(final String key, final boolean def) {
        try {
            if (getBooleanMethod == null) {
                getBooleanMethod = Class.forName("android.os.SystemProperties")
                        .getMethod("getBoolean", String.class, boolean.class);
            }
            return (Boolean) getBooleanMethod.invoke(null, key, def);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return def;
    }
}
