#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

int main(int argc, char *argv[])
{
    int retValue = 0;  //0:OK  other:NG

    FILE *fp = fopen(FACTORYMODE, "r");
    if(fp == NULL)
    {
        FormatPrint("Not in factory test mode \n");
        retValue = 1;
        return retValue;
    }
    else
    {
        fclose(fp);
        //此处开始实际测试
        //xxxxx
        //xxxxx
    }

    if(retValue == 0)
    {
        FormatPrint("NewTest=[OK]\n");
    }
    else
    {
        FormatPrint("NewTest=[NG]\n");
    }
    return retValue;
}
