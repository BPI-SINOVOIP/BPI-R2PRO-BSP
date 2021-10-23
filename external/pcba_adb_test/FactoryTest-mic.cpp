/*
 * Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#define MIC_RECORDED_FILE "/tmp/MicRecordedFile.wav"

long GetBootTimeInSec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

int MicTest(int micRecordTime)
{
    FormatPrint("play sound for %d seconds\n",micRecordTime);
    FormatPrint("record sound for %d seconds\n",micRecordTime);

    int retValue = 0;
    if(access(AUDIO_TEST_FILE, F_OK) == -1)
    {
        FormatPrint("/tmp/AudioTestSoundFile.wav not exist,quit audio play test \n");
        retValue = 1;
        return retValue;
    }

    int cmdExecStatus;
    //stop media server
    FormatPrint("kill media server\n");
    cmdExecStatus = system("killall -9 mediaserver");
    if (-1 == cmdExecStatus)
    {
        FormatPrint("mediaserver stop fail return\n");
        retValue = 1;
        return retValue;
    }
    sleep(2);
    FormatPrint("mediaserver stop success\n");

    long startPlayTime = GetBootTimeInSec();
    long currentTime = startPlayTime;
    //play sound
    FormatPrint("start play sound\n");
    //
    cmdExecStatus = system("killall -9 aplay");
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }
    //
    char playCmd[200];
    memset(playCmd,0,sizeof(playCmd));
    sprintf(playCmd,"aplay %s &",AUDIO_TEST_FILE);
    cmdExecStatus = system(playCmd);
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }

    //record sound
    FormatPrint("start play record sound\n");
    //
    cmdExecStatus = system("killall -9 arecord");
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }
    //
    char recordCmd[200];
    memset(recordCmd,0,sizeof(recordCmd));
    sprintf(recordCmd,"arecord -Dhw:0,0 -d %d -f cd -r 48000 -c 2 -t wav %s &",micRecordTime, MIC_RECORDED_FILE);
    cmdExecStatus = system(recordCmd);
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }

    //stop play
    currentTime = GetBootTimeInSec();
    while(currentTime-startPlayTime < micRecordTime)
    {
        usleep(500 * 1000); //500ms
        currentTime = GetBootTimeInSec();
    }
    FormatPrint("play time enough,stop play\n");
    system("killall aplay");

    sleep(1);

    //play recorded sound
    FormatPrint("start play recorded sound\n");
    //
    cmdExecStatus = system("killall -9 aplay");
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }
    //
    startPlayTime = GetBootTimeInSec();

    memset(playCmd,0,sizeof(playCmd));
    sprintf(playCmd,"aplay %s &",MIC_RECORDED_FILE);
    cmdExecStatus = system(playCmd);
    if (-1 == cmdExecStatus)
    {
        retValue = 1;
        return retValue;
    }

    //stop play
    currentTime = GetBootTimeInSec();
    while(currentTime-startPlayTime < micRecordTime)
    {
        usleep(500 * 1000); //500ms
        currentTime = GetBootTimeInSec();
    }
    FormatPrint("play time enough,stop play\n");
    system("killall aplay");

    //rerun RkLunch script
    system("sh /oem/RkLunch.sh &"); //好像无效
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

    int micRecordTime = 10;
    if(argv[1]!=NULL && atoi(argv[1])!=0)
    {
        micRecordTime = atoi(argv[1]);
        if(micRecordTime>10)
        {
            micRecordTime = 10;
        }
    }

    retValue = MicTest(micRecordTime);
    if(retValue == 0)
    {
        FormatPrint("MicTest=[OK]\n");
    }
    else
    {
        FormatPrint("MicTest=[NG]\n");
    }

    return retValue;
}
