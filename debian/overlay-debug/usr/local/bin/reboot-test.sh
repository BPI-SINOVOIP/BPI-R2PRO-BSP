#!/bin/bash

function rand(){
    min=$1
    max=$(($2-$min+1))
    num=$(date +%s%N)
    echo $(($num%$max+$min))
}

rnd=$(rand 10 50)
echo $rnd"s"

dmesg | grep "Modules linked in"
if [ $? -eq 0 ]
then
echo "find bug1"
dmesg > /root/bug$(date +%y%m%d)
else
echo "didn't find1"
fi

sleep $rnd"s"
reboot
