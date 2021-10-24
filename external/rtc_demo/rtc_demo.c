#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "rtc_demo.h"

static const char default_rtc[] = "/dev/rtc0";

/* funtion:read the rtc time */
int read_rtc_time()
{
    int i, fd, retval;
    struct rtc_time rtc_tm;
    const char *rtc = default_rtc;
    fd = open(rtc, O_RDONLY);
    if (fd == -1){
        perror(rtc);
        exit(errno);
    }
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1){
        perror("RTC_RD_TIME ioctl");
        exit(errno);
    }
    fprintf(stderr, "ReadRTCTime()%d-%d-%d,\%02d:%02d:%02d.\n",
        rtc_tm.tm_year + 1900,rtc_tm.tm_mon + 1,rtc_tm.tm_mday,
        rtc_tm.tm_hour,rtc_tm.tm_min, rtc_tm.tm_sec);

    close(fd);
    return 0;
}

/*
 *  funtion: set rtc time
 *  @param: char *pt="2006-4-20 20:30:30";
 *
 */
 int  write_rtc_time(char *dt)
{
    int   fd, retval;
    struct rtc_time rtc_tm;
    const char *rtc = default_rtc;
    fd = open(rtc, O_RDWR);
    if (fd == -1){
        perror(rtc);
        exit(errno);
    }
    sscanf(dt, "%d-%d-%d %d:%d:%d",
        &rtc_tm.tm_year,&rtc_tm.tm_mon,&rtc_tm.tm_mday,
        &rtc_tm.tm_hour, &rtc_tm.tm_min, &rtc_tm.tm_sec);

    fprintf(stderr, "WriteRTCTime(char *dt) %d-%d-%d %d:%d:%d\n",
        rtc_tm.tm_year,rtc_tm.tm_mon, rtc_tm.tm_mday,rtc_tm.tm_hour,
        rtc_tm.tm_min, rtc_tm.tm_sec);

    rtc_tm.tm_mon--;
    rtc_tm.tm_year=rtc_tm.tm_year-1900;
    retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
    if (retval == -1){
        perror("RTC_SET_TIME ioctl");
        exit(errno);
    }
    close(fd);
    return 0;
 }

/*
 *  funtion: enable RTC alarm power on interrupt
 *  @param: char *pt="2006-4-20 20:30:30";
 *
 */
 int  write_rtc_alarm_time(char *dt)
 {

    int fd, retval;
    struct rtc_time rtc_tm;
    const char *rtc = default_rtc;
    fd = open(rtc, O_RDWR);
    if (fd == -1){
        perror(rtc);
        exit(errno);
    }

    sscanf(dt, "%d-%d-%d %d:%d:%d",
        &rtc_tm.tm_year,&rtc_tm.tm_mon, &rtc_tm.tm_mday,
        &rtc_tm.tm_hour, &rtc_tm.tm_min, &rtc_tm.tm_sec);

    rtc_tm.tm_mon--;
    rtc_tm.tm_year=rtc_tm.tm_year-1900;
    /* Set the current alarm time */
    retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
    if (retval == -1){
        if (errno == ENOTTY){
            fprintf(stderr,"\n...Alarm IRQs not supported.\n");
            //goto test_PIE;
        }
        perror("RTC_ALM_SET ioctl");
        exit(errno);
    }

    /* Read the current alarm settings */
    retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
    if (retval == -1){
        perror("RTC_ALM_READ ioctl");
        exit(errno);
    }

    fprintf(stderr, "WriteRTCAlarmTime() %d-%d-%d %d:%d:%d\n",
        rtc_tm.tm_year+1900,rtc_tm.tm_mon+1, rtc_tm.tm_mday,
        rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    fflush(stderr);
    /* Enable alarm interrupts */
    retval = ioctl(fd, RTC_AIE_ON, 0);
    if (retval == -1){
        perror("RTC_AIE_ON ioctl");
        exit(errno);
    }

    close(fd);
    return 0;
  }

 /*funtion: Read RTC alarm time*/
int  read_rtc_alarm_time()
{
    int   fd, retval;
    struct rtc_time rtc_tm;
    const char *rtc = default_rtc;
    fd = open(rtc, O_RDWR);
    if (fd == -1){
        perror(rtc);
        exit(errno);
    }

    /* Read the current alarm settings */
    retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
    if (retval == -1){
        perror("RTC_ALM_READ ioctl faild");
        exit(errno);
    }

    fprintf(stderr, "ReadRTCAlarmTime() %d-%d-%d %d:%d:%d\n",
        rtc_tm.tm_year+1900,rtc_tm.tm_mon+1, rtc_tm.tm_mday,
        rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    fflush(stderr);
    close(fd);
    return 0;
}

 /*funtion:  disable the RTC alarm  interrupt*/
int   set_rtc_alarm_time_off()
{

    int fd, retval;
    const char *rtc = default_rtc;
    fd = open(rtc, O_RDWR);
    if (fd == -1)
    {
        perror(rtc);
        exit(errno);
    }
    retval = ioctl(fd, RTC_AIE_OFF, 0);
    if (retval == -1)
    {
        perror("ioctl RTC_AIE_OFF faild!!!\n");
        exit(errno);
    }
    close(fd);
    return 0;
}
