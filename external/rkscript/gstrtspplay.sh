#!/bin/sh

URL=$1

if [ -z $URL ]
then
	URL=rtsp://b1.dnsdojo.com:1935/live/sys3.stream
fi

gst-launch-1.0 playbin uri=$URL
