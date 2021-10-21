#!/bin/sh

export DISPLAY=:0.0
#export GST_DEBUG=*:5
#export GST_DEBUG_FILE=/tmp/2.txt

su linaro -c ' \
    gst-launch-1.0 -v videotestsrc  ! "video/x-raw,width=1920,height=1080"  \
                ! queue ! jpegenc ! queue ! jpegparse ! queue ! mppvideodec ! rkximagesink \
'
