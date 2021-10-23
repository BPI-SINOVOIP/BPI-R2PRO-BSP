/*
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

static void rknand_print_hex_data(uint8 *s, struct rk_vendor_req *buf, uint32 len)
{
    unsigned char i = 0;

#ifdef VENDOR_STORAGE_DEBUG
    fprintf(stdout, "%s\n", s);
    fprintf(stdout, "tag = %d // id = %d // len = %d // data = 0x%p\n", buf->tag, buf->id, buf->len, buf->data);
#endif

    printf("Read_SN=[");
    //printf("%s: ", vendor_id_table[buf->id - 1]);
    if(buf->id == VENDOR_SN_ID ||
            buf->id == VENDOR_IMEI_ID)
    {
        for(i = 0; i < len; i++)
        {
            printf("%c", buf->data[i]);
        }
    }
    else
    {
        for(i = 0; i < len; i++)
        {
            printf("%02x", buf->data[i]);
        }
    }
    printf("]");
    fprintf(stdout, "\n");
}

static int vendor_storage_read(int cmd)
{
    uint32 i;
    int ret ;
    uint8 p_buf[100]; /* malloc req buffer or used extern buffer */
    struct rk_vendor_req *req;

    req = (struct rk_vendor_req *)p_buf;
    memset(p_buf, 0, 100);
    int sys_fd = open("/dev/vendor_storage", O_RDWR, 0);
    if(sys_fd < 0)
    {
        printf("vendor_storage open fail\n");
        return -1;
    }

    req->tag = VENDOR_REQ_TAG;
    req->id = cmd;
    req->len = 50;

    ret = ioctl(sys_fd, VENDOR_READ_IO, req);

    if(ret)
    {
        printf("vendor read error %d\n", ret);
        return -1;
    }
    close(sys_fd);

    rknand_print_hex_data("vendor read:", req, req->len);

    return 0;
}

int main(int argc, char *argv[])
{
    //char buf[COMMAND_VALUESIZE] = "wlan_test";
    //char result[COMMAND_VALUESIZE] = RESULT_PASS;
    //test_flag = (int)wlan_test(argv[1]);
    if(argc != 1)
    {
        printf("The input parameter is incorrect! \n");
        return -1;
    }

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }
    int id = VENDOR_SN_ID;

    vendor_storage_read(id);

    return 0;
}
