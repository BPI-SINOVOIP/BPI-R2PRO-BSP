/*
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

//#define READ_DDR_COMMAND "cat /proc/zoneinfo | busybox grep present | busybox awk 'BEGIN{a=0}{a+=$2}END{print a}'"
#define READ_DDR_COMMAND "cat /proc/zoneinfo | busybox grep present | busybox awk 'BEGIN{a=0}{a+=$2}END{print a}'"
#define READ_FLASH_COMMAND "cat /proc/rkflash |  grep Capacity | awk '{print $3}'"
#define CPU_INFO "cat /etc/PLATFORMSOC"

/* for flash  */
int flash_exec(const char *cmd, char *ddrsize_char, unsigned int length)
{
    FILE *pp = popen(cmd, "r");

    //如果文件打开失败，则输出错误信息
    if(!pp)
    {
        printf("errno=%d\n", errno);
        char * mesg = strerror(errno);  //使用strerror（）翻译错误代码
        printf("Mesg:%s\n", mesg);
        return -1;
    }
    if(fgets(ddrsize_char, length, pp) == NULL)
    {
        printf("popen read from %s is NULL!\n", cmd);
        pclose(pp);
        return -1;
    }
    pclose(pp);
    return 0;
}
/* for ddr  */
int ddr_exec(const char *cmd, char *ddrsize_char, unsigned int length)
{
    FILE *pp = popen(cmd, "r");

    //如果文件打开失败，则输出错误信息
    if(!pp)
    {
        printf("errno=%d\n", errno);
        char * mesg = strerror(errno);  //使用strerror（）翻译错误代码
        printf("Mesg:%s\n", mesg);
        return -1;
    }
    if(fgets(ddrsize_char, length, pp) == NULL)
    {
        printf("popen read from %s is NULL!\n", cmd);
        pclose(pp);
        return -1;
    }
    pclose(pp);
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

    int ret = 0;
    char ddrsize_char[20];
    char flashsize_char[20];
    int ddr_size = 0;
    int flash_size = 0;

    memset(ddrsize_char, 0, sizeof(ddrsize_char));
    ret = ddr_exec(READ_DDR_COMMAND, ddrsize_char, sizeof(ddrsize_char));
    if(ret >= 0)
    {
        ddr_size = (int)(atoi(ddrsize_char) * 4 / 1024);
        if(ddr_size > 120 && ddr_size < 150)
        {
            printf("Read_DDRSize=[128]M\n");
        }
        else if(ddr_size > 240 && ddr_size < 300)
        {
            printf("Read_DDRSize=[256]M\n");
        }
        else if(ddr_size > 500 && ddr_size < 550)
        {
            printf("Read_DDRSize=[512]M\n");
        }
        else if(ddr_size > 1000 && ddr_size < 1050)
        {
            printf("Read_DDRSize=[1024]M\n");
        }
        else if(ddr_size > 2000 && ddr_size < 2100)
        {
            printf("Read_DDRSize=[2048]M\n");
        }
        else
        {
            printf("Read DDRSize Error !\n");
        }
    }

    memset(flashsize_char, 0, sizeof(flashsize_char));
    ret = flash_exec(READ_FLASH_COMMAND, flashsize_char, sizeof(flashsize_char));
    if(ret >= 0)
    {
        flash_size = (int)(atoi(flashsize_char));
        printf("Read_FlashSize=[%d]M\n", flash_size);
    }

    FILE *pp;
    pp = popen(CPU_INFO, "r");
    char cpu[64];
    //float sw,hw;
    if(pp == NULL)
    {
        printf("Read CPU_INFO Error !\n");
    }
    else
    {
        fscanf(pp, "%s", cpu);
        printf("Read_CPU=[%s] \n", cpu);
        pclose(pp);
    }

    return 0;
}
