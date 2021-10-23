/*
 *  key_test.c  --  key test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Bin Yang <yangbin@rock-chips.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <set>
#include <linux/input.h>
#include "common.h"

#define KEY_SRC "/dev/input/event1"
#define test_bit(bit) (mask[(bit)/8] & (1 << ((bit)%8)))

struct input_event event;

long GetBootTimeInSec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

int main(int argc, char **argv)
{
    int retValue = 0;

    char buf[256] = { 0, };
    unsigned char mask[EV_MAX / 8 + 1];
    int version;
    int fd = 0;
    int rc;
    int i, j;
    char *tmp;

    int targetKeyNumber = atoi(argv[1]);
    set<int> inputKeySet;
    long startRunTime = GetBootTimeInSec();
    long currentTime = 0;
    FormatPrint("currentTime: %ld \n", currentTime);

    if((fd = open(KEY_SRC, O_RDWR, 0)) >= 0)
    {
        while((rc = read(fd, &event, sizeof(event))) > 0)
        {
            currentTime = GetBootTimeInSec();
            if(currentTime - startRunTime >= 10)
            {
                FormatPrint("Key test 10 sec time up.\n");
                retValue = 2;
                break;
            }

            //判断按键总数
            int testedKeyNumber = inputKeySet.size();
            FormatPrint("Target key number:%d | Total tested key number:%d \n", atoi(argv[1]), testedKeyNumber);
            if(testedKeyNumber >= targetKeyNumber)
            {
                retValue = 0;
                break;
            }

            switch(event.type)
            {
                case EV_KEY:
                    int keyCode = event.code & 0xff;
                    int keyValue = event.value;
                    FormatPrint("Key %d %s", keyCode, keyValue ? "press" : "release");
                    inputKeySet.insert(keyCode);
                    break;
            }
            FormatPrint("\n");
        }

        //判断按键总数
        int testedKeyNumber = inputKeySet.size();
        FormatPrint("Target key number:%d | Total tested key number:%d \n", atoi(argv[1]), testedKeyNumber);
        if(testedKeyNumber >= targetKeyNumber)
        {
            retValue = 0;
        }
        else
        {
            retValue = 1;
        }

        FormatPrint("rc = %d, (%s)\n", rc, strerror(errno));
        close(fd);
    }


    if(retValue == 0)
    {
        FormatPrint("KeyTest=[OK]\n");
    }
    else
    {
        FormatPrint("KeyTest=[NG]\n");
    }
    return retValue;
}
