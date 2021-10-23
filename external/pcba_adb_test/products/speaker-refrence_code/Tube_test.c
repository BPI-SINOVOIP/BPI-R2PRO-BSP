/*
 *  Tube_test.c  --  tube test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: linqihao <kevein.lin@rock-chips.com>
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

#include <stdio.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdlib.h>

//error相关头文件
#include <errno.h>
#include <string.h>
#include "common.h"

/* io control */
#define RK_ECHO_SYSTEM_BOOTING       _IO('I', 0x01)
#define RK_ECHO_SYSTEM_BOOTC         _IO('I', 0x02)
#define RK_ECHO_NET_CONNECTING       _IO('I', 0x03)
#define RK_ECHO_NET_CONNECT_SUCCESS  _IO('I', 0x04)
#define RK_ECHO_NET_CONNECT_FAIL     _IO('I', 0x05)
#define RK_ECHO_NET_CONFIG           _IO('I', 0x06)
#define RK_ECHO_MIC_MUTE             _IO('I', 0x07)
#define RK_ECHO_UPGRADING            _IO('I', 0x08)
#define RK_ECHO_UPGRAD_SUCC          _IO('I', 0x09)
#define RK_ECHO_UPGRADPFAIL          _IO('I', 0x0A)
#define RK_ECHO_CHARGER              _IO('I', 0x0B)
#define RK_ECHO_PHONING              _IO('I', 0x0C)
#define RK_ECHO_PHONE                _IO('I', 0x0D)
#define RK_ECHO_VOLUME_UP            _IO('I', 0x0E)
#define RK_ECHO_VOLUME_DOWN          _IO('I', 0x0F)
#define RK_ECHO_WAKEUP_WAITTING      _IO('I', 0x10)
#define RK_ECHO_TTS_THINKING         _IO('I', 0x11)
#define RK_ECHO_TTS_PLAYING          _IO('I', 0x12)
#define RK_ECHO_SHUTDOWN             _IO('I', 0x13)
#define RK_ECHO_CONTACT              _IO('I', 0x14)
#define RK_ECHO_TIME                _IOW('I', 0x15, struct tube_time)
#define RK_ECHO_ENABLE_TIME          _IO('I', 0x16)
#define RK_ECHO_BT_OPEN              _IO('I', 0x17)
#define RK_ECHO_BT_SUCC              _IO('I', 0x18)
#define RK_ECHO_BT_FAIL              _IO('I', 0x19)
#define RK_ECHO_BT_CLOSE             _IO('I', 0x1A)
#define RK_ECHO_ALARM                _IO('I', 0x1B)
#define RK_ECHO_MIC_UNMUTE           _IO('I', 0x1C)
#define RK_ECHO_LED_OFF              _IO('I', 0x1D)

#define LOG_TAG "tube_test"

#define LEDS_SIMPLE_CTRL_FILE "/dev/tube"


struct tube_time
{
    int tm_hour;
    int tm_min;
};

int multi_ctrl_fd = -1;
struct tube_time tt;
pthread_t ledtime_id;

int set_led_status(int status)
{
    if(RK_ECHO_TIME == status)
    {
        if(ioctl(multi_ctrl_fd, RK_ECHO_TIME, &tt) == 0)
        {
            printf("%s, fd %d set status %d success\n", __func__, multi_ctrl_fd, status);
        }
        else
        {
            printf("%s,fd %d set fail %d\n", __func__, multi_ctrl_fd, status);
        }
    }
    else
    {
        if(ioctl(multi_ctrl_fd, status, NULL) == 0)
        {
            printf("%s, fd %d set status %d success\n", __func__, multi_ctrl_fd, status);
        }
        else
        {
            printf("%s,fd %d set fail %d\n", __func__, multi_ctrl_fd, status);
        }
    }
    return 0;
}

int set_led_time()
{
    printf("set_led_time start \n");
    tt.tm_hour = 88;
    tt.tm_min = 88;
    //set_led_status(RK_ECHO_UPGRADING);
    set_led_status(RK_ECHO_TTS_THINKING);
    return 0;
}

int leds_multi_init()
{
    multi_ctrl_fd = open(LEDS_SIMPLE_CTRL_FILE, O_RDONLY);
    if(multi_ctrl_fd < 0)
    {
        printf("%s,can't open file %s\n", __func__, LEDS_SIMPLE_CTRL_FILE);
        return -1;
    }

    printf("%s,init success %d\n", __func__, multi_ctrl_fd);

    set_led_time();

    return multi_ctrl_fd;
}

int main(int argc, char **argv)
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

    leds_multi_init();

    return 0;
}
