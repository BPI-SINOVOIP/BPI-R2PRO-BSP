#!/bin/bash

AT_CMD_DEV="/dev/ttyUSB1"

open_ec200s()
{
    if [ ! -e "$AT_CMD_DEV" ];then
        return 1
    else 
        exec 8<> $AT_CMD_DEV
        return 0
    fi
}

close_ec200s()
{
    exec 8>&-
    exec 8<&-
}

at_nop()
{
    echo "AT" >&8
    while read -r -t 1 attr <&8 ; do
	echo > /dev/null
    done
    sleep 1
}

check_sim_ready()
{
    at_nop
    echo "AT+CPIN?" >&8
    while read -r -t 2 attr <&8 ; do
        if echo "$attr"|grep -q "+CPIN: READY" ;then
            return 0
        fi
    done
    return 1
}

set_ecm_type()
{
    at_nop
    echo AT+QCFG="usbnet",1 >&8
    while read -r -t 2 attr <&8 ; do
        if echo "$attr"|grep -q "OK" ;then
            return 0
        fi
    done
    return 1
}

shutdown_now()
{
    at_nop
    echo AT+QPOWD >&8
}

call()
{
    at_nop
    echo AT+QNETDEVCTL=1,1,1 >&8
    while read -r -t 2 attr <&8 ; do
        if echo "$attr"|grep -q "OK" ;then
            return 0
        fi
    done
    return 1
}

config_ec200s()
{
    open_ec200s
    [ $? != 0 ] && return 1

    if check_sim_ready ;then
        echo "Sim : ready"
    else
        echo "Sim : no ready"
        close_ec200s
        return 1
    fi

    if set_ecm_type ;then
        echo "Set type : ECM"
    else
        echo "Set ECM type failed"
        close_ec200s
        return 1
    fi

    if call ;then
        echo "Call : success"
    else
        echo "Call : Failed"
        close_ec200s
        return 1
    fi

    close_ec200s

    [ -f /usr/bin/ff_net_forward ] && sh /usr/bin/ff_net_forward

    return 0
}

if ! ifconfig usb0 &> /dev/null ;then
    return    
fi

try_times=10
for((i=0; i<try_times; i++))
do
	if config_ec200s ;then
		break
	fi
done

