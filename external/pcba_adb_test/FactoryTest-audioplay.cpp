/*
 *  audio_test.c  --  audio test application
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
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string>
#include "common.h"
using namespace std;

#define AUDIO_TEST_FILE "/tmp/AudioTestSoundFile.wav"

long GetBootTimeInSec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

int AudioPlayTest(int playTime)
{
    int retValue = 0;
    if(access(AUDIO_TEST_FILE, F_OK) == -1)
    {
        FormatPrint("/tmp/AudioTestSoundFile.wav not exist,quit audio play test \n");
        retValue = 1;
        return retValue;
    }

    long startPlayTime = GetBootTimeInSec();
    long currentTime = startPlayTime;

    char playCmd[200];
    memset(playCmd,0,sizeof(playCmd));
    sprintf(playCmd,"aplay %s &",AUDIO_TEST_FILE);
    FormatPrint("play time: %d, play command: %s\n",playTime, playCmd);
    int cmdExecStatus = system(playCmd);
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }

    if(playTime!=-1)
    {
        currentTime = GetBootTimeInSec();
        while(currentTime-startPlayTime < playTime)
        {
            usleep(500 * 1000); //500ms
            currentTime = GetBootTimeInSec();
        }
        FormatPrint("play time enough,stop play\n");
        system("killall aplay");
    }

    return retValue;
}

int main(int argc, char *argv[])
{
    int retValue = 0;
    if(access(FACTORYMODE, F_OK) == -1)
    {
        FormatPrint("Not in factory test mode \n");
        retValue = 1;
        return retValue;
    }

    int playTime = -1;
    if(argv[1]!=NULL && atoi(argv[1])!=0)
    {
        playTime = atoi(argv[1]);
    }

    retValue = AudioPlayTest(playTime);
    if(retValue == 0)
    {
        FormatPrint("AudioPlayTest=[OK]\n");
    }
    else
    {
        FormatPrint("AudioPlayTest=[NG]\n");
    }

    return retValue;
}
