/*
 *  wlan_test.c  --  wlan test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Panzhenzhuan Wang <randy.wang@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//WLAN test program
#include <stdio.h>

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//error相关头文件
#include <errno.h>
#include <string.h>

#define LOG_TAG "wlan_test"
#include "common.h"
#define WLAN_PROC_ERR -45
#define SIGNAL_REF -60

#define STOP_MP_MODE "rtwpriv wlan0 mp_stop"
#define WLAN_START_UP_COMMAND "ifconfig wlan0 up"
#define WLAN_SCAN_COMMAND "wpa_cli scan 'wlan0'"
#define SCAN_RESULT_FILE "/tmp/wlan_scan_result.txt"
#define GET_CFG_SSID "cat /tmp/factory/wpa_supplicant.conf | grep ssid |awk -F'[\"=]' '{print $3}'"
#define GET_CONNECT_STATUS "wpa_cli status | grep wpa_state |awk -F'[=]' '{print $2}'"
#define GET_CONNECT_NAME "wpa_cli status | grep ssid |awk -F'[=]' 'NR>=2{print $2}'"
#define GET_WLAN_TEST_EXIST  "ps -ef | grep 'Wlan_test' | grep -v 'grep' | awk 'NR>1 {print $1}'"

#define AP_SUPPORT_CMD "iw list | busybox grep AP > %s"
#define AP_SUPPORT_FILE "/tmp/wlan_ap_result.txt"
#define SOFTAP_MSG "dmesg | grep SSID | tail -n 6 > %s"
#define MSG_FILE "/tmp/softap_start_msg.txt"

#define USE_WIFICONFIG 0

#if USE_WIFICONFIG
    #define WLAN_SCAN_RESULT  "wpa_cli scan_r -i 'wlan0' | grep %s | busybox awk '{print $5,$3}'"
    #define WLAN_MANAGE_START "wpa_supplicant -B -i wlan0 -c /tmp/factory/wpa_supplicant.conf &"
#else
    #define WLAN_MANAGE_START "wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf &"
    #define WLAN_SCAN_RESULT  "wpa_cli scan_r -i 'wlan0' | busybox awk 'NR>=2{print $5,$3}'"
#endif

//* 1、关闭后台wpa_supplicant进程
static int close_wpa_supplicant(void)
{
    int test_flag = -1;
    char pid_buf[64];
    char cmd[64];
    FILE* pp;
    printf("====================function : %s start =================\n", __func__);
    pp = popen("ps |grep wpa_supplicant|awk 'NR==1 {print $1}'", "r");
    //pp = popen("ps | grep wpa_supplicant | grep -v grep","r");
    //如果文件打开失败，则输出错误信息
    if(!pp)
    {
        printf("%s popen err%s\n", __func__, strerror(errno));
        return -1;
    }
    memset(pid_buf, 0, sizeof(pid_buf));
    fgets(pid_buf, sizeof(pid_buf), pp);
    pclose(pp);
    printf("Get pid_buf is: \t %s\n", pid_buf);
    sprintf(cmd, "kill -9 %d", atoi(pid_buf));
    printf("cmd is: %s\n", cmd);
    system(cmd);
    printf("====================function : %s finish =================\n", __func__);
    return 0;
}

/*
 * RSSI Levels as used by notification icon
 *
 * Level 4  -55 <= RSSI
 * Level 3  -66 <= RSSI < -55
 * Level 2  -77 <= RSSI < -67
 * Level 1  -88 <= RSSI < -78
 * Level 0         RSSI < -88
 */
static int calc_rssi_lvl(int rssi)
{
    if(rssi >= -55)
    {
        return 4;
    }
    else if(rssi >= -66)
    {
        return 3;
    }
    else if(rssi >= -77)
    {
        return 2;
    }
    else if(rssi >= -88)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int get_connect_status(char *buf)
{
    char cfg_ssid[128], connect_ssid[128], connect_status[16];
    int ret = 0;

    printf("=================== function :%s start======================\n\n", __func__);

    //获取CFG SSID NAME
    FILE *pp = popen(GET_CFG_SSID, "r");
    //如果文件打开失败，则输出错误信息
    if(pp == NULL)
    {
        printf("GET_CFG_SSID NAME Error !\n");
    }
    else
    {
        fscanf(pp, "%s", cfg_ssid);
        printf("GET_CFG_SSID is: %s \n", cfg_ssid);
        pclose(pp);
    }

    pp = popen(GET_CONNECT_NAME, "r");
    if(pp == NULL)
    {
        printf("Read GET_CONNECT_NAME Error !\n");
    }
    else
    {
        fscanf(pp, "%s", connect_ssid);
        printf("GET_CONNECT_NAME is: %s \n", connect_ssid);
        pclose(pp);
    }

    pp = popen(GET_CONNECT_STATUS, "r");
    if(pp == NULL)
    {
        printf("GET_CONNECT_STATUS Error !\n");
    }
    else
    {
        fscanf(pp, "%s", connect_status);
        printf("GET_CONNECT_STATUS is: %s \n", connect_status);
        pclose(pp);
    }

    if(!strcmp(connect_status, "COMPLETED"))
    {
        printf("connect_status is: %s \n", connect_status);
        if(!strcmp(cfg_ssid, connect_ssid))
        {
            printf("connect_ssid is: %s \n", connect_ssid);
            sprintf(buf, "%s", connect_ssid);
            return 0;
        }
        else
        {
            printf("connect_ssid is: %s \n", connect_ssid);
            return -1;
        }
    }
    else
    {
        printf("connect_status is: %s \n", connect_status);
        return -1;
    }

}
/* 将扫描到的WiFi信息输出到path对应文件
 * 并显示第一个WiFi名字和信号强度
*/
int wlan_exec(const char *cmd, char *path, char *buf)
{
    char ssid[128];
    int signal_level;
    int ch = 0;

    //printf("=================== function :%s start======================\n\n",__func__);

    //显示第一个WiFi信息
    FILE *pp = popen(cmd, "r");
    //如果文件打开失败，则输出错误信息
    if(!pp)
    {
        printf("%s popen err%s\n", __func__, strerror(errno));
        return -1;
    }

    while(!feof(pp))
    {
        fscanf(pp, "%s", ssid);
        fscanf(pp, "%d", &signal_level);
        printf("%s=[%d]db\n", ssid, signal_level);
        //根据信号强度判断WiFi是否正常工作
        /*
        if(signal_level< -200||signal_level >=0)
        {
            sprintf(buf,"SSID: %s, signal: %d",ssid,signal_level);
            printf("wlan scan result is null\n");
            return -1;
        }else if(signal_level > SIGNAL_REF){
            sprintf(buf,"SSID: %s, signal: %d",ssid,signal_level);
        	printf("wlan test success! \n");
        }else{
            sprintf(buf,"SSID: %s, signal: %d",ssid,signal_level);
        	printf("wlan test failed! \n");
            return -1;
        }
        break;
        */
    }
    pclose(pp);
    //printf("\n=================== function :%s finish======================\n",__func__);
    return 0;
}

void *wlan_test(char *buf)
{
    int test_flag = -1;
    char cmd[128];
    char ssid[128];
    printf("===================Wlan test start======================\n");
    //sprintf(cmd,"aplay %s/wlan_test_start.wav",AUDIO_PATH);
    //system(cmd);
    //system("aplay /data/cfg/test/wlan_test_start.wav");

    //1、先关闭后台wpa_supplicant进程，然后先卸载WLAN驱动固件，然后重新加载驱动固件
    //test_flag = close_wpa_supplicant();
    //system("echo 1 > /sys/class/rkwifi/driver");
    //sleep(1);
    //system(STOP_MP_MODE);

    //2、开启wlan0端口
    //system(WLAN_START_UP_COMMAND);
    //sleep(2);

    //3、启动无线网卡管理程序wpa_supplicant
    //system(WLAN_MANAGE_START);
    //sleep(2);

    //4、扫描无线网络
    system(WLAN_SCAN_COMMAND);
    sleep(4);

#if USE_WIFICONFIG
    //5、获取网络连接信息，获取连接信息
    int ret = get_connect_status(ssid);
#else
    int ret = 0;
#endif
    if(ret == 0)
    {
        //wifi连接成功。
        //6、显示出WiFi信息
#if USE_WIFICONFIG
        sprintf(cmd, WLAN_SCAN_RESULT, ssid);
#else
        sprintf(cmd, WLAN_SCAN_RESULT);
#endif
        test_flag = wlan_exec(cmd, SCAN_RESULT_FILE, buf);
        if(test_flag < 0)
        {
            goto fail;
        }
    }
    else
    {
        //wifi连接失败
        sprintf(buf, "wifi connect failed");
        test_flag = -1;
        goto fail;
    }

//   printf("===================Wlan test success ======================\n");
//   system("ifconfig wlan0 down");
//   system("busybox killall wpa_supplicant");      //关闭无线管理程序wpa_supplicant
    return (void*)test_flag;
fail:
    printf("===================Wlan test failed======================\n");
//       system("ifconfig wlan0 down");
//       system("busybox killall wpa_supplicant");      //关闭无线管理程序wpa_supplicant
    return (void*)test_flag;
}

int isTestRunning()
{
    int test_flag = -1;
    char pid_buf[64];
    char cmd[64];
    FILE* pp;
    pp = popen(GET_WLAN_TEST_EXIST, "r");
    if(!pp)
    {
        printf("%s popen err%s\n", __func__, strerror(errno));
        return -1;
    }
    memset(pid_buf, 0, sizeof(pid_buf));
    fgets(pid_buf, sizeof(pid_buf), pp);
    pclose(pp);
    printf("Get pid_buf is: %s\n", pid_buf);
    if(strcmp(pid_buf, ""))
    {
        printf("====================echo_wlan_test isTestRunning, stop Test =================\n", __func__);
        return -1;
    }
    return 0;
}
int main(int argc, char *argv[])
{
    if(argc != 1)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }

    int test_flag = 0, err_code = 0;
    char buf[COMMAND_VALUESIZE] = {0};
    char tmp[COMMAND_VALUESIZE] = {0};
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    /*
    if(isTestRunning()){
    	return 0;
    }*/
    test_flag = (int)wlan_test(buf);
    if(test_flag < 0)
    {
        strcpy(result, RESULT_FAIL);
        err_code = WLAN_PROC_ERR;
        return -1;
    }
    return 0;
}
