/*
 * Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "cJSON/cJSON.h"

#define LOG_TAG "ringmic_test"
#include "common.h"

#define AUDIO_CHANNAL_CNT 8
#define RINGMIC_BAD_ERROR 93 /* has bad mic */
#define THRES_REC_PASS_RATE     (60)
#define SAMPLE_STANDARD    1200

#define DEBUG 0

int channel_type[8];

int *bytesToInt(char *src, int *length)
{
    int value = 0, offset = 0, i = 0;
    int *ret = NULL;
    int int_len = 0;

    int_len = (*length / 4) + 1;
    ret = (int *)malloc(int_len * 4);
    if(!ret)
    {
        return NULL;
    }

    while((offset < *length) && (*length - offset >= 4))
    {
        value = (int)((src[offset] & 0xFF) |
                      ((src[offset + 1] & 0xFF) << 8) |
                      ((src[offset + 2] & 0xFF) << 16) |
                      ((src[offset + 3] & 0xFF) << 24));
        offset += 4;
        ret[i++] = value;
    }

    *length = int_len;
    return ret;
}
int linear_amplification(void *data, void *out, int bytes)
{
    int i = 0, j = 0;
    int mChannels = AUDIO_CHANNAL_CNT;
    for(i = 0; i < bytes / 2;)
    {
        for(j = 0; j < mChannels; j++)
        {
            short raw = (*((short *)data + i + j));
            int tmp_data = (int)raw;
#if 1
            tmp_data = tmp_data << 1;
            if(tmp_data > 32767)
            {
                tmp_data = 32767;
            }
            else if(tmp_data < -32768)
            {
                tmp_data = -32768;
            }
#endif
            *((short *)out + i + j) = (short)tmp_data;
        }
        i += mChannels;
    }
    return 0;
}

int preProcessBuffer(void *data, void *out, int bytes)
{
    int i = 0, j = 0;

    for(i = 0; i < bytes / 2 ;)
    {
        for(j = 0; j < AUDIO_CHANNAL_CNT; j++)
        {
            int tmp = 0;
            short tmp_data = (*((short *)data + i + j));
            tmp = ((tmp_data) << 16 | ((j + 1) << 8)) & 0xffffff00;
            *((int *)out + i + j) = tmp;
        }
        i += AUDIO_CHANNAL_CNT;
    }

    return 0;
}

static int add_channel(char *src, char **dst, int len)
{
    int fd = 0;
    int dst_len = 0;
    int ret = 0;

    dst_len = len * 2;
    *dst = (char *)malloc(dst_len);
    if(!*dst)
    {
        return -ENOMEM;
    }

    preProcessBuffer(src, *dst, len);

    return dst_len;
}

// 单独检查各个channel音频文件中的标号
static int check_index(int signal, int input[], int input_length)
{
    int i = 0, j = 0;
    int buf;
    int channel_index, first_index;

    //printf("int bit is %d\n", sizeof(int));
    // 跳过最初的20个字节
    i = 20;
    first_index = (input[i] >> 8) & 0xf;
    i++;
    while(i < input_length)
    {
        channel_index = (input[i] >> 8) & 0xf;
        //printf("file index is %d, first_index is %d, channel_index is %d", signal, first_index, channel_index);
        if(channel_index != first_index)
        {
            //printf("check index fail, lost data, file index is %d, first_index is %d, channel_index is %d", signal, first_index, channel_index);
            return -12;
        }
        i++;
    }

#if DEBUG
    printf("check index success, channel_index is %d\n", channel_index);
#endif

    return 0;
}

// 检查mic数据
static int check_data_right(int signal, int input[], int input_length)
{
    int i = 0, j = 0;
    int buf;
    int channel_index, data_fre;
    unsigned long pass_count = 0; // 通过的次数
    unsigned long total_count = 0; // 总的次数
    int pass_rate = 0;

    // 跳过最初的20个字节
    i = 20;
    i++;
    while(i < input_length)
    {
        total_count++;
        data_fre = (input[i] >> 16) & 0xffff;
        if((input[i] >> 16) & 0x8000)
        {
            data_fre = ~(input[i] >> 16) + 1;
        }
        else
        {
            data_fre = (input[i] >> 16) & 0xffff;
        }

        if(data_fre > SAMPLE_STANDARD)
        {
#if 0
            printf("signal : %d, data_fre is %d \n", signal, data_fre);
#endif
            pass_count++;
        }
#if DEBUG
        else
        {
            printf("signal : %d, data_fre is %d \n", signal, data_fre);
        }
#endif
        i++;
    }
    pass_rate = pass_count * 100 / total_count;
#if DEBUG
    printf("signal : %d, pass_count is %d, total_count is %d, pass_rate is %d\n", signal, pass_count, total_count, pass_rate);
#endif
    if(pass_rate < THRES_REC_PASS_RATE)
    {
        return -1;
    }
    return 0;
}

int* recordTestWr(int audio_data[], int audio_length)
{
    int *buf;
    static int ccids[13];

    int signal = 0;
    int i = 0, ret = 0;
    int rlength = audio_length / 8;
    int pcmIndex[8];
    int *pcmInput[8];

    //printf("start to check audio , recordtest!!!\n");
    buf = audio_data;
    memset(pcmIndex, 0, sizeof(pcmIndex));
    memset(ccids, 0, sizeof(ccids));
    if(NULL == buf)
    {
        return NULL;
    }

    for(i = 0; i < 8; i++)
    {
        pcmInput[i] = malloc(sizeof(int) * rlength);
        if(pcmInput[i] == NULL)
        {
            printf("pcmInput malloc failed \n");
            return NULL;
        }
    }

    //printf("split audio begin\n");
    while(i < audio_length)
    {
        signal = (buf[i] >> 8) & 15;
        if(signal > 0 && signal <= 8)
        {
            signal--;
            if(pcmIndex[signal] < rlength)
            {
                *(pcmInput[signal] + pcmIndex[signal]) = buf[i];
                pcmIndex[signal] ++;
            }
        }
        else
        {
            // printf("signal : %d , less than 1 or larger than 8 \n", signal);
        }
        i++;
    }
    //printf("split audio success\n");

    // 检查标号
    for(i = 0; i < 8; i++)
    {
        ret = check_index(i, pcmInput[i], pcmIndex[i]);
        if(ret < 0)
        {
            //printf("check index fail\n");
            if(ret != 0)
            {
                ccids[i] = 1;
            }
        }
    }

    // 检查mic数据
    for(i = 0; i < 8; i++)
    {
        if(i <= 7 &&  channel_type[i] == 0)
        {
            ret = 0;
        }
        else
        {
            ret = check_data_right(i, pcmInput[i], pcmIndex[i]);
        }
        if(ret != 0)
        {
            //printf("check data fail\n");
            ccids[i] = 1;
        }
        else
        {
            //printf("check data success\n");
        }
    }

    for(i = 0; i < 8; i++)
    {
        free(pcmInput[i]);
    }
    return ccids;
}

/*
 * This function includes a ring mic recording test
 * and vibration test, select the test type by flag.
 *    flag = 0:ring mic record test.
 *    flag = 1:ring mic vibration test.
 */
int ringmic_test(char *result, int flag)
{
    int *record_ret = NULL;
    int fd = 0;
    int rf_len = 0; /* record file length */
    int pre_len = 0;
    char *rf_buff = NULL; /* record file buffer */
    char *pre_buff = NULL;
    char *gain_buff = NULL;
    int *buffer = NULL;
    char path[128] = {0};
    int ret = 0, i = 0;


    log_info("Start record test.\n");
    sprintf(path, "%s", "/tmp/ringmic_record.pcm");
    /* Play the specified file, and recording. */
    system("killall -9 arecord");
    system("rm /tmp/ringmic_record.pcm");
    system("aplay /data/rectest_400hz.wav &");
    usleep(200000);
    system("arecord -t raw -f S16_LE -c 8 -r 16000 -d 5 /tmp/ringmic_record.pcm");

    system("killall -9 arecord");
    system("killall aplay");

    //log_info("Parsing audio file...\n");
    fd = open(path, O_RDONLY);
    if(fd <= 0)
    {
        log_err("open %s failed(%s).\n", path, strerror(errno));
        return errno;
    }
    /* get length of the record file. */
    rf_len = lseek(fd, 0, SEEK_END);
    rf_buff = (char *)malloc(rf_len);
    if(!rf_buff)
    {
        close(fd);
        return -ENOMEM;
    }
    memset(rf_buff, 0, rf_len);
    /* Read from the beginning of the file. */
    lseek(fd, 0, SEEK_SET);
    ret = read(fd, rf_buff, rf_len);
    if(ret != rf_len)
    {
        log_err("read %s failed(%s).\n", path, strerror(errno));
        close(fd);
        free(rf_buff);
        return errno;
    }
    close(fd);

#if 1
    /* Increase gain */
    gain_buff = (char *)malloc(rf_len);
    if(!gain_buff)
    {
        close(fd);
        free(rf_buff);
        return -ENOMEM;
    }
    memset(gain_buff, 0, rf_len);
    linear_amplification(rf_buff, gain_buff, rf_len);
    memcpy(rf_buff, gain_buff, rf_len);
    free(gain_buff);
#endif
    FILE *fp;

    fp = fopen("/tmp/gain-rec.pcm", "wb");
    if(fp)
    {
        fwrite(rf_buff, 1, rf_len, fp);
        fclose(fp);
    }

    /* Add channel numbers to the original recording file */
    pre_len = add_channel(rf_buff, &pre_buff, rf_len);
    if(pre_len < 0)
    {
        free(rf_buff);
        return pre_len;
    }
    free(rf_buff);

    fp = fopen("/tmp/addchan-rec.pcm", "wb");
    if(fp)
    {
        fwrite(pre_buff, 1, pre_len, fp);
        fclose(fp);
    }

    buffer = bytesToInt(pre_buff, &pre_len);
    if(!buffer)
    {
        log_err("bytesToInt() failed!\n");
        free(pre_buff);
        return -ENOMEM;
    }
    free(pre_buff);
    /*
        fp = fopen("/tmp/bytesToInt-rec.pcm", "wb");
        if (fp) {
            fwrite((int *)buffer+128000, 4, pre_len-128000, fp);
            fclose(fp);
        }
    */

    /* Call library interface */
    {
        record_ret = recordTestWr((int *)buffer + 128000, pre_len - 128000);
        for(i = 0; i < AUDIO_CHANNAL_CNT; i++)
        {
            if(*(record_ret + i))
            {
                //log_info("recordTest:#%d mic is bad!\n", i);
                *result++ = 1;
            }
            else
            {
                //log_info("recordTest:#%d mic is ok!\n", i);
                *result++ = 0;
            }
        }

//		system("rm -rf /tmp/ringmic_record.pcm");
    }

    free(buffer);
    return 0;
}


int main(int argc, char *argv[])
{
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

    char buf[COMMAND_VALUESIZE] = {0};
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    unsigned char record_ret[AUDIO_CHANNAL_CNT] = {0};
    unsigned char vibration_ret[AUDIO_CHANNAL_CNT] = {0};
    int ispass = 1;
    int i = 0, ret = 0;

    system("amixer set Master Playback 99%");
    usleep(800000);

    FILE            *fp = NULL;
    cJSON           *json;
    char            line[1024] = {0};
    char            str[10000];

    if(NULL != (fp = fopen("/tmp/factory/ringmic.json", "r")))
    {
        while(NULL != fgets(line, sizeof(line), fp))
        {
            line[strlen(line) - 1] = '\0';
            strcat(str, line);
        }
    }
    else if(NULL != (fp = fopen("/etc/ringmic.json", "r")))
    {
        while(NULL != fgets(line, sizeof(line), fp))
        {
            line[strlen(line) - 1] = '\0';
            strcat(str, line);
        }
    }
    else if(NULL != (fp = fopen("/data/ringmic.json", "r")))
    {
        while(NULL != fgets(line, sizeof(line), fp))
        {
            line[strlen(line) - 1] = '\0';
            strcat(str, line);
        }
    }
    else
    {
        printf("parase ringmic.json file error! \n");
        printf("no found  ringmic.json file in /etc/ or /tmp/factory/ \n");
        return -1;
    }

    close(fp);
    strcat(str, "}");
    json = cJSON_Parse(str);
    cJSON *item = cJSON_GetObjectItem(json, "channal_0");
    channel_type[0] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_1");
    channel_type[1] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_2");
    channel_type[2] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_3");
    channel_type[3] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_4");
    channel_type[4] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_5");
    channel_type[5] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_6");
    channel_type[6] = atoi(item->valuestring);
    item = cJSON_GetObjectItem(json, "channal_7");
    channel_type[7] = atoi(item->valuestring);

    /*
    	channel_type[0] = 1;
    	channel_type[1] = 1;
    	channel_type[2] = 0;
    	channel_type[3] = 0;
    	channel_type[4] = 0;
    	channel_type[5] = 0;
    	channel_type[6] = 0;
    	channel_type[7] = 0;
    */

    ret = ringmic_test(record_ret, 0);
    if(ret)
    {
        printf("unexpect error happened!!!\n");
        printf("Ringmic_test=[NG]\n");
        ispass = 0;
    }
    else
    {
        for(i = 0; i < 2; i++)
        {
            if(record_ret[i])
            {
                ispass = 0;
                printf("Mic_%d=[NG]\n", i);
            }
            else
            {
                printf("Mic_%d=[OK]\n", i);
            }
        }
    }
    return 0;
}
