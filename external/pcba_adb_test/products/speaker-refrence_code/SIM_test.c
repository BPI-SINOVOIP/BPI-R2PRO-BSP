/*
 *  SIM_test.c  --  sim test application
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
#include <termios.h>
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

int at_command(int fd, char *cmd, int to)
{
    fd_set rfds;
    struct timeval timeout;
    char buf[1024];
    int sel, len, i;
    int returnCode = 0;

    write(fd, cmd, strlen(cmd));
    tcdrain(fd);

    for(i = 0; i < 100; i++)
    {

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = to;

        if((sel = select(fd + 1, &rfds, NULL, NULL, &timeout)) > 0)
        {

            if(FD_ISSET(fd, &rfds))
            {
                memset(buf, 0, sizeof(buf));
                len = read(fd, buf, sizeof(buf));
                buf[len] = '\0';
                printf("DATA %s \n", buf);
                if(strstr(buf, "OK") != NULL)
                {
                    printf("at commad return just OK\n");
                    returnCode = 1;
                    break;
                }

                if(strstr(buf, "ERROR") != NULL)
                {
                    printf("at commad return error\n");
                    returnCode = 0;
                    break;
                }
            }

        }

    }

    return returnCode;
}

//add for LTE func
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    if(tcgetattr(fd, &oldtio)  !=  0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch(nBits)
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch(nEvent)
    {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
    }

    switch(nSpeed)
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    if(nStop == 1)
    {
        newtio.c_cflag &=  ~CSTOPB;
    }
    else if(nStop == 2)
    {
        newtio.c_cflag |=  CSTOPB;
    }
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    if((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

/* open  a given serial port
 * PARAMS:
 * fd  - file descriptor
 * comport - modem porot numver
 * RETURNS:
 * fd > 0 success , -1 otherwise
 */
int open_port(int fd, int comport)
{
    if(comport == 0)
    {
        //printf("open /dev/ttyUSB0");
        fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
        if(-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            printf("sucess open ttyUSB0 .....\n");
        }
    }
    else if(comport == 1)
    {
        fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY);
        if(-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            //  printf("open ttyUSB1 .....\n");
        }
    }
    else if(comport == 2)
    {
        fd = open("/dev/ttyUSB2", O_RDWR | O_NOCTTY | O_NDELAY);
        if(-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            //  printf("open ttyUSB2 .....\n");
        }
    }
    if(fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
    }
    else
    {
        //printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    }
    if(isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
    }
    else
    {
        //  printf("isatty success!\n");
    }
//  printf("fd-open=%d\n",fd);
    return fd;
}

int check_SIM()
{
    //printf("enter check_lte_isconnected\n");
    int tty_fd;
    int ret;
    char ret_buff[256] = {0};
    char temp_buff[256] = {0};
    int isLteConnected = 0;
    fd_set rd;

    if((tty_fd = open_port(tty_fd, 1)) < 0)
    {
        perror("open lte port open_port error");
        return -1;
    }

    if((ret = set_opt(tty_fd, 115200, 8, 'N', 1)) < 0)
    {
        perror("set lte port set_opt error");
        return -1;
    }

    //printf("fd=%d\n",tty_fd);

    FD_ZERO(&rd);

    if(!at_command(tty_fd, "AT\r\n", 10000))
    {
        printf("at commod send error.\n");
        isLteConnected = -1;
        close(tty_fd);
        return  isLteConnected;
    }
    else
    {
        printf("send AT ommod sucess ok ok ok.\n");
    }
    /*
        if (!at_command(tty_fd, "AT+QSCLK=1\r\n" , 10000)) {
            printf("lte AT+QSCLK=1  send error.\n");
            isLteConnected = false;
            close(tty_fd);
            return isLteConnected;
        } else {
            printf("send AT+QSCLK=1 sucess ok ok ok.\n");
        }
    */
    if(!at_command(tty_fd, "AT+CPIN?\r\n", 10000))
    {
        printf("check cpin no sim card.\n");
        isLteConnected = -1;
        close(tty_fd);
        return  isLteConnected;
    }
    else
    {
        printf("send AT+CPIN cheeck sim  Card ready.\n");
    }
    //if (!isLteConnected)
    //printf("lte is not connected.\n");
    return isLteConnected;
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

    int ret = 0;
    for(int i = 0; i < 2 ; i++)
    {
        ret = check_SIM();
        sleep(1);
    }
    if(ret == -1)
    {
        printf("SIM_test=[NG].\n");
    }
    else
    {
        printf("SIM_test=[OK].\n");
    }
    return 0;
}
