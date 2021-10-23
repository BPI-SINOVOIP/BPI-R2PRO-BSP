#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

int UsbHostAutoTest()
{
    int ret = 0;
    int status = 0;
    status = system("/usr/bin/FactoryTest-usbhost.sh");
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
        FormatPrint("usbhost test failed. ret = %d\n", ret);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int retValue = 0, tmpRes = 0;

    if(access(FACTORYMODE, F_OK) == -1)
    {
        FormatPrint("Not in factory test mode \n");
        retValue = 1;
        return retValue;
    }

    FormatPrint("usbhost test process start...\n");

    tmpRes = UsbHostAutoTest();
    if(tmpRes == 0)
    {
        FormatPrint("usbhost test Pass\n");
    }
    else if(tmpRes == 1)
    {
        retValue = 1;
        FormatPrint("usbhost test MountErr...\n");
    }
    else if(tmpRes == 2)
    {
        retValue = 2;
        FormatPrint("usbhost test Timgout...\n");
    }

    if(retValue == 0)
    {
        FormatPrint("USBHostTest=[OK]\n");
    }
    else
    {
        FormatPrint("USBHostTest=[NG]\n");
    }
    return retValue;
}

