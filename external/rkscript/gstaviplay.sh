#!/bin/sh

videofile=$1
gst-launch-1.0 filesrc location="$videofile" ! avidemux ! h264parse ! mppvideodec ! kmssink
