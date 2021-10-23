/*
 *  Battery_test.c  --  Battery test application
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
#include "common.h"

typedef		unsigned short	    uint16;
typedef		unsigned int	    uint32;
typedef		unsigned char	    uint8;

#define BATTERY_STATUS  "/sys/class/power_supply/battery/capacity"
#define BATTERY_DEBUG   "echo 1 >  /sys/module/rk816_battery/parameters/dbg_level"
//if vocv > 4500, it means no battery
#define BATTERY_VOCV    "dmesg | grep Vocv | awk -F'[=,]' 'NR<2 {print $9}'"


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

    system(BATTERY_DEBUG);
    sleep(2);
    int vocv;
    FILE *pp = popen(BATTERY_VOCV, "r");
    if(pp == NULL)
    {
        printf("Read BATTERY_VOCV Error !\n");
    }
    else
    {
        fscanf(pp, "%d", &vocv);
        pclose(pp);
    }
    if(vocv > 4500)
    {
        printf("No Battary detect ! \n");
        printf("Battery_Value=[0]\n");
        return -1;
    }
    FILE *fp;
    fp = fopen(BATTERY_STATUS, "r");
    lseek(fp, 0, SEEK_SET);
    float *data = (float *)malloc(sizeof(float));
    if(fp == NULL)
    {
        printf("Read BATTERY_STATUS Error !\n");
    }
    else
    {
        fscanf(fp, "%f", data);
        printf("Battery_Value=[%.0f]\n", *data);
        fclose(fp);
    }
    return 0;
}
