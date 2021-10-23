/*
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
#include <stdio.h>

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>


//error相关头文件
#include <errno.h>
#include <string.h>
#include "common.h"

typedef		unsigned short	    uint16;
typedef		unsigned int	    uint32;
typedef		unsigned char	    uint8;

#define VENDOR_REQ_TAG		0x56524551
#define VENDOR_READ_IO		_IOW('v', 0x01, unsigned int)
#define VENDOR_WRITE_IO		_IOW('v', 0x02, unsigned int)

#define VENDOR_SN_ID		1
#define VENDOR_WIFI_MAC_ID	2
#define VENDOR_LAN_MAC_ID	3
#define VENDOR_BT_MAC_ID	4
#define VENDOR_IMEI_ID		5

struct rk_vendor_req
{
    uint32 tag;
    uint16 id;
    uint16 len;
    uint8 data[1024];
};

int vendor_storage_write(int cmd, char *num)
{
    uint32 i;
    int ret ;
    uint8 p_buf[100]; /* malloc req buffer or used extern buffer */
    struct rk_vendor_req *req;

    req = (struct rk_vendor_req *)p_buf;
    int sys_fd = open("/dev/vendor_storage", O_RDWR, 0);
    if(sys_fd < 0)
    {
        printf("vendor_storage open fail\n");
        return -1;
    }

    req->tag = VENDOR_REQ_TAG;
    req->id = cmd;

    if(cmd != VENDOR_SN_ID && cmd != VENDOR_IMEI_ID)
    {
        req->len = 6;
    }
    else
    {
        req->len = strlen(num);
    }
    memcpy(req->data, num, req->len);

    ret = ioctl(sys_fd, VENDOR_WRITE_IO, req);
    if(ret)
    {
        printf("vendor write error\n");
        return -1;
    }

    //rknand_print_hex_data("vendor write:", req, req->len);
    return 0;
}

void vendor_storage_write_cmd_parse(char *cmd)
{
    char space = ' ';
    int i, cnt, tmp, id = 0xff;
    unsigned char vendor_id[20];
    unsigned char vendor_num[20];
    unsigned char vendor_hex[10];

    cnt = strlen(cmd);

    memset(vendor_num, 0, sizeof(vendor_num));
    memcpy(vendor_num, cmd, cnt);

    //set write category
    id = VENDOR_BT_MAC_ID;

    memset(vendor_hex, 0, sizeof(vendor_hex));
    if(id != VENDOR_SN_ID && id != VENDOR_IMEI_ID)
    {
        tmp = strlen(vendor_num);
        for(i = 0; i < tmp; i++)
        {
            if(vendor_num[i] < '0' ||
                    (vendor_num[i] > '9' && vendor_num[i] < 'A') ||
                    (vendor_num[i] > 'F' && vendor_num[i] < 'a') ||
                    vendor_num[i] > 'f')
            {
                //printf("%s must be HEX input\n", vendor_id_table[id + 1]);
                goto error;
            }

            /* string to hex */
            if(vendor_num[i] >= '0' && vendor_num[i] <= '9')
            {
                vendor_num[i] -= '0';
            }
            else if(vendor_num[i] >= 'a' && vendor_num[i] <= 'f')
            {
                vendor_num[i] -= 'a' - 10;
            }
            else
            {
                vendor_num[i] -= 'A' - 10;
            }

            if(i & 1)
            {
                vendor_hex[(i - 1) >> 1] = (vendor_num[i - 1] << 4) | vendor_num[i];
            }
        }
    }

    if(id == VENDOR_SN_ID || id == VENDOR_IMEI_ID)
    {
        if(vendor_storage_write(id, vendor_num))
        {
            goto error;
        }
    }
    else
    {
        if(vendor_storage_write(id, vendor_hex))
        {
            goto error;
        }
    }

    //printf("vendor write successful!\n");
    return;

form_error:
    printf("ERROR: form_error \n");
error:
    printf("ERROR: vendor_storage_write error \n");
    return;
}

char* substring(char* ch, int pos, int length)
{
    //定义字符指针 指向传递进来的ch地址
    char* pch = ch;
    //通过calloc来分配一个length长度的字符数组，返回的是字符指针。
    char* subch = (char*)calloc(sizeof(char), length + 1);
    int i;
    //只有在C99下for循环中才可以声明变量，这里写在外面，提高兼容性。
    pch = pch + pos;
    //是pch指针指向pos位置。
    for(i = 0; i < length; i++)
    {
        subch[i] = *(pch++);
        //循环遍历赋值数组。
    }
    subch[length] = '\0'; //加上字符串结束符。
    return subch;       //返回分配的字符数组地址。
}

int main(int argc, char *argv[])
{
    //char buf[COMMAND_VALUESIZE] = "wlan_test";
    //char result[COMMAND_VALUESIZE] = RESULT_PASS;
    //test_flag = (int)wlan_test(argv[1]);
    if(argc != 2)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }
    int length = 0;
    char *data = NULL;
    char *result = NULL;
    char *SN, *ID, *IMEI;
    char cmd[64];
    //printf("The input parameter is %s \n",argv[1]);

    length = strlen(argv[1]);
    data = (char *)malloc(length);
    strcpy(data, argv[1]);

    data = substring(data, 1, length - 2);
    vendor_storage_write_cmd_parse(data);


    printf("Write_SN=[OK] \"%s\" \n", argv[1]);
    return 0;
}
