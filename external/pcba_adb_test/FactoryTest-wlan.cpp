#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

int WLanTest()
{
    int retValue = 0;

    int net_fd;
    net_fd = open("/sys/class/net/wlan0/operstate", O_RDONLY);
    if(net_fd < 0)
    {
        FormatPrint("open wlan0 err\n");
        retValue = 1;
        return retValue;
    }
    FormatPrint("open wlan0 success\n");

    char status[20];
    memset(status, 0, sizeof(status));
    int ret = read(net_fd, status, 10);
    FormatPrint("wlan0 status is %s", status);
    if(NULL != strstr(status, "up"))
    {
        FormatPrint("wlan0 online\n");
        retValue = 0;
    }
    else if(NULL != strstr(status, "down"))
    {
        FormatPrint("wlan0 offline\n");
        retValue = 1;
    }
    else
    {
        FormatPrint("wlan0 nknown err\n");
        retValue = 2;
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

    retValue = WLanTest();
    if(retValue == 0)
    {
        FormatPrint("WLanTest=[OK]\n");
    }
    else
    {
        FormatPrint("WLanTest=[NG]\n");
    }

    return retValue;
}
