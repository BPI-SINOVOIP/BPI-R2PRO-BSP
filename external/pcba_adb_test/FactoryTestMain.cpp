#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FACTORYMODE "/tmp/FACTORY_TEST_MODE"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(!strcmp(argv[1], "start"))
    {
        system("touch /tmp/FACTORY_TEST_MODE");
        FILE *fp = fopen(FACTORYMODE, "r");
        if(fp != NULL)
        {
            printf("FACTORY_TEST_MODE=[OK] \n");
            //此处增加执行脚本操作，开启厂测后需要执行的脚本
            //system("xxxxxxxx.sh");
        }
        fclose(fp);
        return 0;
    }
    else if(!strcmp(argv[1], "stop"))
    {
        FILE *fp = fopen(FACTORYMODE, "r");
        if(fp == NULL)
        {
            printf("Not in factory test mode \n");
        }
        else
        {
            fclose(fp);
            system("rm /tmp/FACTORY_TEST_MODE");
            printf("STOP_FACTORY_TEST_MODE=[OK] \n");
            //此处增加执行脚本操作，停止厂测后需要执行的脚本
            //system("xxxxxxxx.sh");
        }
        return 0;
    }

    printf("Unexpected error, Please check the parameter! \n");
    return -1;
}
