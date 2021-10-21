#!/bin/bash

AT_CMD_DEV="/dev/ttyUSB1"

shutdown_now()
{
    if [ -e $AT_CMD_DEV ];then
        echo AT+QPOWD > $AT_CMD_DEV
    fi
}

shutdown_now
