#!/bin/sh

USBHOST=sda
USBHOST_PATH=/dev/${USBHOST}
TIMEOUT=10
MOUNTPOINT=/tmp/usb_storage

echo Start testing USBHOST:${USBHOST}

for i in `seq ${TIMEOUT}`;do
	echo Waiting for USBHOST inserted ... `expr ${TIMEOUT} - ${i}`
	sleep 1
	ls ${USBHOST_PATH} >/dev/null 2>&1 || continue

	echo USBHOST inserted...

	capacity=`cat /proc/partitions | grep ${USBHOST} -w | busybox awk '{printf $3}'`

	echo "${USBHOST}: ${capacity}"

	if [ $capacity -gt 0 ] 
	then
		echo "usb capacity > 0,OK"
		exit 0
	fi
	
	exit 1
done

echo Timed out waiting for USBHOST:${USBHOST}...
exit 2
