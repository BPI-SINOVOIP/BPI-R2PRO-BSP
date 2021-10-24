#!/bin/sh

videofile=$1
gst-launch-1.0 filesrc location="$videofile" ! tsdemux ! mpegvideoparse ! mppvideodec ! kmssink
