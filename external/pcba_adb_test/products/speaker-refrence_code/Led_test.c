/*
 *  Led_test.c  --  Led test application
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
#include "common.h"

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>


//error相关头文件
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    //char buf[COMMAND_VALUESIZE] = "wlan_test";
    //char result[COMMAND_VALUESIZE] = RESULT_PASS;
    //test_flag = (int)wlan_test(argv[1]);
    if(argc != 4)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }
    int red, green, blue = 0;
    char cmd[128];
    red = atoi(argv[1]);
    green = atoi(argv[2]);
    blue = atoi(argv[3]);

    if(red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
    {
        printf("he input parameter is incorrect! Should be > 0 and <256 \n");
        return -1;
    }
    sprintf(cmd, "echo %d > /sys/class/leds/redled/brightness", red);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));

    sprintf(cmd, "echo %d > /sys/class/leds/greenled/brightness", green);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));

    sprintf(cmd, "echo %d > /sys/class/leds/blueled/brightness", blue);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));

    return 0;
}
