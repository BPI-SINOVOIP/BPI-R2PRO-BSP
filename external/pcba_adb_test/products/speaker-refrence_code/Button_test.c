/*
 *  Button_test.c  --  Button test application
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
//key test program
#include <stdio.h>
#include <linux/input.h>

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include "cJSON/cJSON.h"

//error相关头文件
#include <errno.h>
#include <string.h>
#include "common.h"

/* key code */
#define KEY_VOLUP_CODE      115
#define KEY_VOLDN_CODE      114
#define KEY_MUTE_CODE       248
#define KEY_MODE_CODE       373
#define KEY_PLAYPAUSE_CODE       164

#define KEY_TIMEOUT_DOWN    10
#define KEY_TIMEOUT_UP      5

#define LOG_TAG "key_test"

int  KEY_VALID_NUM = 0;

typedef struct KEY_BOARD_st
{
    char *key_name;
    int key_value;
    int key_press_flag;
} KEY_st;

KEY_st gkey_test[7] = {};
/*
{
    {"VOL+",    KEY_VOLUP_CODE,    0},
    {"VOL-",    KEY_VOLDN_CODE,    0},
    {"MICMUTE", KEY_MUTE_CODE,    0},
	{"PLAY/PAUSE",    KEY_PLAYPAUSE_CODE,    0},
    {"MODECHANGE",   KEY_MODE_CODE,    0},

};
*/

int init_keyset()
{
    FILE *fp = NULL;
    cJSON *json, *array, *objread, *item;
    char buf[1024] = {0};
    char str[10000];
    char file[64];
    char *name;
    int value;

    if(NULL != (fp = fopen("/etc/keytest.json", "r")))
    {
        while(NULL != fgets(buf, sizeof(buf), fp))
        {
            buf[strlen(buf) - 1] = '\0';
            strcat(str, buf);
        }
    }
    else
    {
        printf("parase /etc/keytest.json file error! \n");
    }

    fclose(fp);
    strcat(str, "}");
    //printf("str is %s \n",str);
    json = cJSON_Parse(str);
    array = cJSON_GetObjectItem(json, "keyset");
    KEY_VALID_NUM = cJSON_GetArraySize(array);
    //printf("KEY_VALID_NUM is %d \n", KEY_VALID_NUM);
    for(int i = 0; i < KEY_VALID_NUM; ++i)
    {
        objread = cJSON_GetArrayItem(array, i);
        item = cJSON_GetObjectItem(objread, "name");
        name = item-> valuestring;
        item = cJSON_GetObjectItem(objread, "value");
        value = item-> valueint;
        gkey_test[i].key_name = name;
        gkey_test[i].key_value = value;
        memset(file, 0, sizeof(file));
        sprintf(file, "/tmp/%s_OK", name);
        fp = fopen(file, "r");
        if(fp == NULL)
        {
            printf("%s %d,[NOT TEST]\n", name, value);
            gkey_test[i].key_press_flag = 0;
        }
        else
        {
            printf("%s %d,[OK]\n", name, value);
            gkey_test[i].key_press_flag = 1;
            fclose(fp);
        }
    }
    //printf("====================function : %s finished =================\n", __func__);
    return 0;
}

static int isAllKeyPressed(KEY_st *key_array)
{
    int i = 0;

    if(key_array == NULL)
    {
        return 0;
    }

    for(i = 0; i < KEY_VALID_NUM; i++)
    {
        if(key_array[i].key_press_flag)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if(i == KEY_VALID_NUM)
    {
        //printf("########### All key had pressed!!! ######### \n");
        //dumpKeyPressInfo(key_array);
        return 1;
    }
    else
    {
        return 0;
    }
}

void dumpKeyPressInfo(KEY_st *key_array)
{

    printf("########### key test is over!!! ######### \n");
    int i = 0;

    if(key_array == NULL)
    {
        return;
    }

    for(i = 0; i < KEY_VALID_NUM; i++)
    {
        if(key_array[i].key_press_flag)
        {
            printf("%s=[OK]\n", key_array[i].key_name);
        }
        else
        {
            printf("%s=[NG]\n", key_array[i].key_name);
        }
    }
}

int key_event_read(int fd, struct input_event *buf)
{
    int read_len = 0;

    read_len = read(fd, buf, sizeof(*buf));
    if(read_len < 0)
    {
        if((errno != EINTR) && (errno != EAGAIN))
        {
            return 0;
        }

        return -1;
    }
    //printf("<key.type=%d>\n<key.code=%d>\n<key.value=%d>\n",buf->type,buf->code,buf->value);
    if(buf->type)
    {
        return 1;
    }

    return 0;
}

#define MAX_INPUT_COUNT (4)
int main(int argc, char **argv)
{
    if(argc != 1 && argc != 2)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }

    int time = 0;

    if(argc == 2)
    {
        time = atoi(argv[1]);
    }
    else
    {
        time = KEY_TIMEOUT_DOWN;
    }

    int fd;
    int ret = 0;
    int err_code = 0;
    fd_set rdfds;
    struct input_event key_event;
    int idx = 0;
    int fds[MAX_INPUT_COUNT] = {0};
    char path[64];
    int fdcount = 0;
    int max_fd = 0;
    int i, k;
    struct timeval sel_timeout_tv;
    char cmd[64];

    printf("key test process start...\n");

    init_keyset();

    if(isAllKeyPressed(gkey_test))
    {
        goto EXIT;
    }

    for(i = 0; i < MAX_INPUT_COUNT; i++)
    {
        sprintf(path, "/dev/input/event%d", i);
        fd = open(path, O_RDONLY | O_NOCTTY);

        if(fd < 0)
        {
            printf("open fail:%s\n", strerror(errno));
            continue;
        }

        fds[fdcount++] = fd;
    }

    if(fdcount < 1)
    {
        //err_code = KEY_OPEN_FAIL;
        goto EXIT;
    }

    for(i = 0 ; i < fdcount; i++)
        if(max_fd < fds[i])
        {
            max_fd = fds[i];
        }

    while(1)
    {
        FD_ZERO(&rdfds);
        sel_timeout_tv.tv_sec = time;
        sel_timeout_tv.tv_usec = 0;

        for(i = 0 ; i < fdcount; i++)
        {
            FD_SET(fds[i], &rdfds);
        }

        if(isAllKeyPressed(gkey_test))
        {
            break;
        }

        ret = select(max_fd + 1, &rdfds, NULL, NULL, &sel_timeout_tv);
        if(ret > 0)
        {
            k = 0;

            while(k < fdcount)
            {
                int fd = fds[k++];

                if(FD_ISSET(fd, &rdfds))
                {
                    ret = key_event_read(fd, &key_event);

                    if(ret > 0)
                    {
                        if(key_event.value)
                        {
                            //printf("key(%d) is down\n", key_event.code);
                            time = KEY_TIMEOUT_UP;
                        }
                        else
                        {
                            //printf("key(%d) is up\n", key_event.code);
                            if(argc == 2)
                            {
                                time = atoi(argv[1]);
                            }
                            else
                            {
                                time = KEY_TIMEOUT_DOWN;
                            }
                            for(idx = 0; idx < KEY_VALID_NUM; idx ++)
                            {
                                if(key_event.code == gkey_test[idx].key_value
                                        && gkey_test[idx].key_press_flag == 0)
                                {
                                    gkey_test[idx].key_press_flag = 1;
                                    printf("%s is pressed \n", gkey_test[idx].key_name);
                                    memset(cmd, 0, sizeof(cmd));
                                    sprintf(cmd, "touch /tmp/%s_OK", gkey_test[idx].key_name);
                                    system(cmd);
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            if(isAllKeyPressed(gkey_test))
                            {
                                goto EXIT;
                            }
                        }
                    }
                }
            }
        }
        else if(ret == 0)
        {
            //printf("wait key event fail, errno=%d\n", errno);
            //if (idx != KEY_VALID_NUM) {

            /*
            int i = 0;
            for (i  = 0; i < KEY_VALID_NUM; ++i)  {
                if (gkey_test[i].key_press_flag == 0) {
            		printf("%s =[NG]",gkey_test[idx].key_name);
                }
            }
             */
            //}
            goto EXIT;
        }
    }

    //snprintf(buf, sizeof(buf), "key_code:%d", key_event.code);
EXIT:
    //sleep(1);
    //memset(buf, 0 , sizeof(buf));
    //if (!err_code)
    //strcpy(result, RESULT_VERIFY);
    //send_msg_to_server(buf, result, err_code);
    dumpKeyPressInfo(gkey_test);
    return err_code;
}
