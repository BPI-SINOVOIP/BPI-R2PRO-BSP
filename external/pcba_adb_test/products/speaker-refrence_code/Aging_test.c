#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include "common.h"

#define 	LOG_TAG 	("echo_laohua_test")
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

    if(fopen(FACTORYMODE, "r") == NULL)
    {
        printf("Please enter testMode first! \n");
        return -1;
    }

    int fd;
    int ret = 0;
    int err_code = 0;
    int time = 60;
    fd_set rdfds;
    struct input_event key_event;
    int idx = 0;
    int fds[MAX_INPUT_COUNT] = {0};
    char path[64];
    int fdcount = 0;
    int max_fd = 0;
    int i, k;
    struct timeval sel_timeout_tv;
    struct timeval tv_down, tv_up;
    int delay_t = 0;

    //fork a process to show led
    pid_t pid;
    pid = fork();
    if(pid < 0)
    {
        printf("fork led process error\n");
        return -1;
    }


    for(i = 0; i < MAX_INPUT_COUNT; i++)
    {
        sprintf(path, "/dev/input/event%d", i);
        fd = open(path, O_RDONLY | O_NOCTTY);
        if(fd < 0)
        {
            //printf("open fail:%s\n", strerror(errno));
            continue;
        }
        fds[fdcount++] = fd;
    }

    if(fdcount < 1)
    {
        //printf("input keyevent open fail");
        return -1;
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
                        //log_info("value %d\n", key_event.value);
                        if(key_event.value)    //down
                        {
                            log_info("key(%d) is down\n", key_event.code);
                            gettimeofday(&tv_down, NULL);
                        }
                        else
                        {
                            log_info("key(%d) is up\n", key_event.code);
                            gettimeofday(&tv_up, NULL);

                            delay_t = (tv_up.tv_sec - tv_down.tv_sec) * 1000000 + (tv_up.tv_usec - tv_down.tv_usec);
                            log_info("interval (%d)\n", delay_t);
                            if(delay_t > 3000000 && key_event.code == KEY_VOLUMEUP)
                            {
                                system("/oem/sayinfoos.sh stop");
                                sleep(1);
                                system("amixer set Master Playback 80");
                                if(pid == 0)
                                {
                                    printf("********I am child :%d\t**********\n", getpid());
                                    system("echo 1 0x000000 > /sys/class/leds/blueled/debug");
                                    system("echo 0 > sys/class/leds/blueled/brightness");
                                    system("echo 0 > sys/class/leds/greenled/brightness");
                                    system("echo 0 > sys/class/leds/redled/brightness");

                                    while(1)
                                    {
                                        system("echo 0 > sys/class/leds/redled/brightness");
                                        system("echo 255 > sys/class/leds/greenled/brightness");
                                        sleep(5);
                                        system("echo 0 > sys/class/leds/greenled/brightness");
                                        system("echo 255 > sys/class/leds/blueled/brightness");
                                        sleep(5);
                                        system("echo 0 > sys/class/leds/blueled/brightness");
                                        system("echo 255 > sys/class/leds/redled/brightness");
                                        sleep(5);
                                    }
                                }
                                else
                                {
                                    do
                                    {
                                        if(fopen("/data/factory_aging_test.wav", "r") == NULL)
                                        {
                                            sleep(2);
                                        }
                                        else
                                        {
                                            system("aplay /data/factory_aging_test.wav");
                                        }
                                    }
                                    while(1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


