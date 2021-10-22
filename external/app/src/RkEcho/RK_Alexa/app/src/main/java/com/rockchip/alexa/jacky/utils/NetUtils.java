package com.rockchip.alexa.jacky.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.provider.Settings;

import com.rockchip.alexa.jacky.R;


/**
 * Created by cjs on 2017/4/17.
 */

public class NetUtils {

    /**
     * 对当前的网络状态进行判断
     * @param context
     * @return
     */
    public static boolean isOpenNetwork(Context context) {
        ConnectivityManager connManager = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connManager.getActiveNetworkInfo() != null) {
            return connManager.getActiveNetworkInfo().isAvailable();
        }
        return false;
    }


    /**
     * 打开网络设置
     * @param context
     */
    public static void setNetworkMethod(final Context context){
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(context.getString(R.string.dialog_network_set_title))
                .setMessage(context.getString(R.string.dialog_network_set_message))
                .setPositiveButton(context.getString(R.string.dialog_network_set_positive_button),
                        new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent;
                if(android.os.Build.VERSION.SDK_INT > 10){
                    intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
                }else{
                    intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
                }
                context.startActivity(intent);
            }
        }).setNegativeButton(context.getString(R.string.dialog_network_set_negative_button),new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        }).show();
    }
}
