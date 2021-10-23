/*
 *  wlan_test.c  --  wlan test application
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

//error相关头文件
#include <errno.h>
#include <string.h>
#include "common.h"

#define WATCH_SERVICE "ps -ef | grep -iE 'watch_sai_service' | grep -v 'grep' | awk '{print $1}' | xargs kill -9"
#define VOIP_CLIENT "ps -ef | grep -iE 'cmcc_voip' | grep -v 'grep' | awk '{print $1}' | xargs kill -9"
#define QPLAY_CLIENT "ps -ef | grep -iE 'qplay' | grep -v 'grep' | awk '{print $1}' | xargs kill -9"
#define SAI_CLIENT "ps -ef | grep -iE 'sai_client' | grep -v 'grep' | awk '{print $1}' | xargs kill -9"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    FILE *fp;

    if(!strcmp(argv[1], "stop"))
    {
        fp = fopen(FACTORYMODE, "r");
        if(fp == NULL)
        {
            printf("Not in testMode \n");
        }
        else
        {
            fclose(fp);
            system("rm /tmp/testMode");
            system("recoverySystem");
        }
        return 0;
    }

    if(!strcmp(argv[1], "start"))
    {
        system("touch /tmp/testMode");
        fp = fopen(FACTORYMODE, "r");
        if(fp != NULL)
        {
            printf("testMode=[OK] \n");
        }
        fclose(fp);
        system("/oem/sayinfoos.sh stop");
//        system(WATCH_SERVICE);
//        system(VOIP_CLIENT);
//        system(QPLAY_CLIENT);
//        system(SAI_CLIENT);
//        system("Aging_test &");
        return 0;
    }
    printf("Unexpected error, Please check the parameter! \n");
    return -1;
}
