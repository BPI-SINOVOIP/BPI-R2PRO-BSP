#!/bin/sh

export DISPLAY=:0.0 
#export GST_DEBUG=rgaconvert:5
#export GST_DEBUG_FILE=/tmp/2.txt

su linaro -c ' \
    gst-launch-1.0  filesrc location=/usr/local/test.mp4 ! qtdemux ! queue ! h264parse ! mppvideodec ! \
    rgaconvert output-io-mode=dmabuf-import capture-io-mode=dmabuf vpu-stride=true ! \
    "video/x-raw,format=NV12, width=1920,height=1080"  ! rkximagesink \
'
