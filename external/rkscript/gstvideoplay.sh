#!/bin/sh

videofile=$1

if [ -z $videofile ]
then
	videofile=/oem/SampleVideo_1280x720_5mb.mp4
fi

#gst-launch-1.0 filesrc location="$videofile" ! qtdemux ! aacparse ! faad ! alsasink
#gst-launch-1.0 filesrc location="$videofile" ! qtdemux ! h264parse ! mppvideodec ! kmssink
gst-launch-1.0 playbin uri=file://$videofile
