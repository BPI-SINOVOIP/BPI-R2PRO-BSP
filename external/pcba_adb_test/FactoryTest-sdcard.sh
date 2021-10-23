#!/bin/sh

SDCARD=mmcblk1
SDCARD_PATH=/dev/${SDCARD}
TIMEOUT=10
MOUNTPOINT=/tmp/sdcard

echo Start testing SDCARD:${SDCARD}

for i in `seq ${TIMEOUT}`;do
	echo Waiting for SDCARD inserted ... `expr ${TIMEOUT} - ${i}`
	sleep 1
	ls ${SDCARD_PATH} >/dev/null 2>&1 || continue

	echo SDCARD inserted...

	capacity=`cat /proc/partitions | grep ${SDCARD} -w | busybox awk '{printf $3}'`

	echo "${SDCARD}: ${capacity}"

	if [ $capacity -gt 0 ] 
	then
		echo "sdcard capacity > 0,OK"
		exit 0
	fi
	
	exit 1
done

echo Timed out waiting for SDCARD:${SDCARD}...
exit 2
