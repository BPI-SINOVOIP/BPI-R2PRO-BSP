#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "parameter.h"
#include "common.h"

void time_flush(void)
{
    time_t t;

    if (!last_time)
    {
        last_time = (struct tm *)malloc(sizeof(struct tm));
    }
    if (now_time)
        memcpy(last_time, now_time, sizeof(struct tm));

    time(&t);
    now_time = localtime(&t);
    strftime(status_bar_date_str, sizeof(status_bar_date_str), "%Y-%m-%d", now_time);
    if (get_time_format() == USE_12_HOUR_FORMAT)
    {
        if (now_time->tm_hour >= 12)
        {
            sprintf(status_bar_time_str, "%02d:%02d PM", now_time->tm_hour % 12, now_time->tm_min);
        }
        else
        {
            sprintf(status_bar_time_str, "%02d:%02d AM", now_time->tm_hour, now_time->tm_min);
        }
    }
    else
    {
        sprintf(status_bar_time_str, "%02d:%02d", now_time->tm_hour, now_time->tm_min);
    }
    //printf("now time:%02d:%02d,last time:%02d:%02d\n",now_time->tm_hour,now_time->tm_min,last_time->tm_hour,last_time->tm_min);
}

void time_set(struct tm time)
{
    time_t t;
    t = mktime(&time);
    stime(&t);
    system("hwclock -w");
}

void write_timing_to_file(void)
{
    FILE *fp;
    char timing_buf[6] = {0};
    int i;
    fp = fopen(TIMING_FILE, "w");
    if (fp == NULL)
    {
        printf("open file %s failed: %s\n", TIMING_FILE, strerror(errno));
        return -1;
    }
    for (i = 0; i < TIMING_NUM; i++)
    {
        sprintf(timing_buf, "%04d%01d\n", timing_power_on[i].timing, timing_power_on[i].status);
        fwrite(timing_buf, sizeof(timing_buf), 1, fp);
    }
    for (i = 0; i < TIMING_NUM; i++)
    {
        sprintf(timing_buf, "%04d%01d\n", timing_power_off[i].timing, timing_power_off[i].status);
        fwrite(timing_buf, sizeof(timing_buf), 1, fp);
    }
    fclose(fp);
}

void timing_set(struct tm time, int n)
{
    if (n < 0 || n >= (2 * TIMING_NUM))
    {
        printf("%s:not in range %d\n", __func__, n);
        return;
    }
    if (n >= TIMING_NUM)
    {
        n -= TIMING_NUM;
        timing_power_off[n].timing = time.tm_hour * 100 + time.tm_min;
        printf("%s:timing_power_off[%d]:%d\n", __func__, n, timing_power_off[n].timing);
        timing_power_off_set();
    }
    else
    {
        timing_power_on[n].timing = time.tm_hour * 100 + time.tm_min;
        printf("%s:timing_power_on[%d]:%d\n", __func__, n, timing_power_on[n].timing);
        timing_power_on_set();
    }
}

void timing_power_off_set(void)
{
    int i;
    int final_time_i;
    int enable_cnt = 0;
    int time_temp1, time_temp2;
    time_temp1 = 9999;
    for (i = 0; i < TIMING_NUM; i++)
    {
        if (timing_power_off[i].status == 0) continue;
        if (timing_power_off[i].timing >= (now_time->tm_hour * 100 + now_time->tm_min))
            time_temp2 = timing_power_off[i].timing - (now_time->tm_hour * 100 + now_time->tm_min);
        else
            time_temp2 = 2400 - ((now_time->tm_hour * 100 + now_time->tm_min) - timing_power_off[i].timing);
        if (time_temp1 > time_temp2)
        {
            final_time_i = i;
            time_temp1 = time_temp2;
        }
        enable_cnt++;
    }
    if (enable_cnt > 0)
    {
        timing_power_off[TIMING_NUM].status = 1;
        timing_power_off[TIMING_NUM].timing = timing_power_off[final_time_i].timing;
        printf("%s:%d\n", __func__, timing_power_off[final_time_i].timing);
    }
    else
    {
        printf("no timing power off\n");
        timing_power_off[TIMING_NUM].status = 0;
    }
    write_timing_to_file();
}

void timing_power_on_set(void)
{
    int i;
    int final_time_i;
    int enable_cnt = 0;
    int time_temp1, time_temp2;
    time_temp1 = 9999;
    for (i = 0; i < TIMING_NUM; i++)
    {
        if (timing_power_on[i].status == 0) continue;
        if (timing_power_on[i].timing >= (now_time->tm_hour * 100 + now_time->tm_min))
            time_temp2 = timing_power_on[i].timing - (now_time->tm_hour * 100 + now_time->tm_min);
        else
            time_temp2 = 2400 - ((now_time->tm_hour * 100 + now_time->tm_min) - timing_power_on[i].timing);
        if (time_temp1 > time_temp2)
        {
            final_time_i = i;
            time_temp1 = time_temp2;
        }
        enable_cnt++;
    }
    if (enable_cnt > 0)
    {
        char alarm_time[20];
        timing_power_on[TIMING_NUM].status = 1;
        timing_power_on[TIMING_NUM].timing = timing_power_on[final_time_i].timing;
        if (timing_power_on[TIMING_NUM].timing >= (now_time->tm_hour * 100 + now_time->tm_min))
        {
            printf("%d %d >= %d\n", final_time_i, timing_power_on[TIMING_NUM].timing, (now_time->tm_hour * 100 + now_time->tm_min));
            sprintf(alarm_time, "%04d-%02d-%02d %02d:%02d:%02d", now_time->tm_year + 1900, now_time->tm_mon + 1, now_time->tm_mday,
                    (int)(timing_power_on[TIMING_NUM].timing / 100), (timing_power_on[TIMING_NUM].timing % 100), 0);
        }
        else
        {
            printf("%d %d < %d\n", final_time_i, timing_power_on[TIMING_NUM].timing, (now_time->tm_hour * 100 + now_time->tm_min));
            sprintf(alarm_time, "%04d-%02d-%02d %02d:%02d:%02d", now_time->tm_year + 1900, now_time->tm_mon + 1, now_time->tm_mday + 1,
                    (int)(timing_power_on[TIMING_NUM].timing / 100), (timing_power_on[TIMING_NUM].timing % 100), 0);
        }
        int res = write_rtc_alarm_time(alarm_time);
        printf("%s:%s | %d\n", __func__, alarm_time, res);
    }
    else
    {
        printf("no timing power on\n");
        timing_power_on[TIMING_NUM].status = 0;
    }
    write_timing_to_file();
}


