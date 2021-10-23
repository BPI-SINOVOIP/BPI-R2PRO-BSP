#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

int IRCutTest()
{
    int retValue = 0;
    system("v4l2-ctl -d /dev/v4l-subdev5 --set-ctrl 'band_stop_filter=0'");
    sleep(5);
    system("v4l2-ctl -d /dev/v4l-subdev5 --set-ctrl 'band_stop_filter=3'");
    sleep(5);
    system("v4l2-ctl -d /dev/v4l-subdev5 --set-ctrl 'band_stop_filter=0'");
    sleep(5);
    system("v4l2-ctl -d /dev/v4l-subdev5 --set-ctrl 'band_stop_filter=3'");
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

    retValue = IRCutTest();
    if(retValue == 0)
    {
        FormatPrint("IRCutTest=[OK]\n");
    }
    else
    {
        FormatPrint("IRCutTest=[NG]\n");
    }

    return retValue;
}
