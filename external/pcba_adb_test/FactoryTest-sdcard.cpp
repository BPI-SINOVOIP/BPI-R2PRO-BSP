#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

int SDCardTest()
{
    int ret = 0;
    int status = 0;
    status = system("/usr/bin/FactoryTest-sdcard.sh");
    if(status == -1)
    {
        FormatPrint("system cmd run error...\n");
        ret = -1;
    }
    else
    {
        FormatPrint("exit status value = [0x%x]\n", status);
        if(WIFEXITED(status))
        {
            if(0 == WEXITSTATUS(status))
            {
                FormatPrint("run shell script successfully.\n");
                ret = 0;
            }
            else
            {
                if(1 == WEXITSTATUS(status))
                {
                    FormatPrint("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                    ret = 1;
                }
                else if(2 == WEXITSTATUS(status))
                {
                    FormatPrint("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                    ret = 2;
                }
            }
        }
        else
        {
            FormatPrint("exit status = [%d]\n", WEXITSTATUS(status));
            ret = -1;
        }
    }

    if(ret != 0)
    {
        FormatPrint("sdcard test failed. ret = %d\n", ret);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int retValue = 0, tmpRes = 0;

    FILE *fp = fopen(FACTORYMODE, "r");
    if(fp == NULL)
    {
        FormatPrint("Not in factory test mode \n");
        retValue = 1;
        return retValue;
    }

    FormatPrint("SDCard test process start...\n");

    tmpRes = SDCardTest();
    if(tmpRes == 0)
    {
        FormatPrint("SDCard test Pass\n");
    }
    else if(tmpRes == 1)
    {
        retValue = 1;
        FormatPrint("SDCard test MountErr...\n");
    }
    else if(tmpRes == 2)
    {
        retValue = 2;
        FormatPrint("SDCard test Timgout...\n");
    }

    if(retValue == 0)
    {
        FormatPrint("SDCardTest=[OK]\n");
    }
    else
    {
        FormatPrint("SDCardTest=[NG]\n");
    }
    return retValue;
}

