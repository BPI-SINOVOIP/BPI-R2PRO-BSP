#!/bin/sh

export DISPLAY=:0.0 
#export GST_DEBUG=rgaconvert:5
#export GST_DEBUG_FILE=/tmp/2.txt

su linaro -c ' \
    gst-launch-1.0 -v videotestsrc ! "video/x-raw,format=BGRA, width=1920,height=1080,framerate=30/1" ! \
    rgaconvert hflip=false vflip=false rotation=90 input-crop=0x0x1920x1080 output-crop=0x0x640x360 \
     output-io-mode=dmabuf capture-io-mode=dmabuf ! \
    "video/x-raw,format=NV12, width=640,height=360,framerate=30/1" ! rkximagesink \
'
