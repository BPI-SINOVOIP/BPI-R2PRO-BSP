#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <time.h>  
#include <signal.h>
#include <linux/input.h>  

static int main_thread_run_flag = 1;

static void ff_key_signo_handle(int32_t signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo){
        printf("Signal received, ready to exit...\n");
        main_thread_run_flag = 0;
    }   
}

//rk29-keypad
int get_event_num(char *input_name)
{
    if(!input_name) return -1;

    char          name[64];           /* RATS: Use ok, but could be better */  
    char          buf[256] = { 0, };  /* RATS: Use ok */  
    int           fd = 0;   
    int           i;    

    for (i = 0; i < 32; i++) 
    {  
        sprintf(name, "/dev/input/event%d", i);  
        if ((fd = open(name, O_RDONLY, 0)) >= 0) 
        {  
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);   
            /* printf("event%d:%s\n",i, buf); */
            if(strstr(buf, input_name))
            {
                close(fd);
                return i;
            }
            close(fd);  
        }
    }
    return -1;
}

static struct timeval now()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv;
}

static void reboot()
{
    FILE  *fp = popen("reboot","r");
    pclose(fp);
}

static void reset()
{
    FILE  *fp = popen("rm -rf /userdata/*","r");
    pclose(fp);
    fp = popen("sync", "r");
    pclose(fp);
}

static void led_ctrl(char *led_name, int on_off)
{
    if(led_name){
        char cmd[100] = {0};
        snprintf(cmd, 100, "echo %d > /sys/class/leds/%s/brightness", on_off, led_name);
        FILE  *fp = popen(cmd,"r");
        pclose(fp);
    }
}

/*
 *argv[1] : input_name
 argv[2] : 重启按下时间(单位：s)
 argv[3] : 复位按下时间(单位：s)
 argv[4] : led灯
 */
int main(int argc, char **argv)
{
    int fd;
    int ret;
    struct input_event t;
    fd_set readset;

    struct timeval pressed_timeval, released_timeval;
    int reset_flag = 0, reboot_flag = 0;
    int has_pressed = 0, has_released = 0;
    int reboot_sec = 2 , reset_sec = 5;

    if(argc<2){
        fprintf(stderr, "Help: %s <input_name> [reboot seconds] [reset seconds] [led]\n", argv[0]);
        fprintf(stderr, "  eg: %s adc-keys 2 5 firefly:green:L2\n", argv[0]);
        return -1;
    }

    //获取和打开设备
    int event_num = get_event_num(argv[1]);
    if(event_num < 0){
        fprintf(stderr, "could not find target pdev_name in /dev/input/event*\n");
        return -1;
    }
    char input_dev[64] = {0};
    snprintf(input_dev, 64, "/dev/input/event%d", event_num);
    fd = open(input_dev, O_RDONLY);
    if (fd <= 0){
        fprintf(stderr, "open %s device error!\n", input_dev);
        return 0;
    }

    //中断信号处理
    signal(SIGINT, ff_key_signo_handle);
    signal(SIGTERM, ff_key_signo_handle);

    while (main_thread_run_flag){
        sleep(0);
        FD_ZERO(&readset);                       /* 清空文件描述符集合 */
        FD_SET(fd, &readset);                /* 添加文件描述符到集合 */

        ret = select(fd + 1, &readset, NULL, NULL, NULL); //No block
        if(ret <= 0)
            continue;

        if (read(fd, &t, sizeof (t)) == sizeof (t) && t.type == EV_KEY){
            if(t.value == 1){ //Pressed
                led_ctrl(argv[4], 1);
                pressed_timeval = now();
                has_pressed = 1;
            }else{
                led_ctrl(argv[4], 0);
                released_timeval = now();
                has_released = 1;
            }
            if(has_pressed && has_released){
                int sub = released_timeval.tv_sec - pressed_timeval.tv_sec; 
                if( sub >= 0 && sub <= reboot_sec ){
                    reboot_flag = 1;
                }else if(sub >= reset_sec){
                    reset_flag = 1;
                }else{ //reinit
                    reboot_flag = reset_flag = 0;
                    has_pressed = has_released = 0;
                }
            }

            if(reboot_flag){
                printf("reboot now...\n");
                reboot();
                break;
            }
            if(reset_flag){
                printf("reset and reboot now...\n");
                reset();
                reboot();
                break;
            }
        }
    }

    close(fd);
}
