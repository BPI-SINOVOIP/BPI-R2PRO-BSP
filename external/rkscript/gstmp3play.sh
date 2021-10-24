#!/bin/sh

audiofile=$1

if [ -z $audiofile ]
then
	audiofile=/oem/piano2-CoolEdit.mp3
fi

gst-launch-1.0 filesrc location="$audiofile" ! mpegaudioparse ! mpg123audiodec ! audioconvert ! autoaudiosink
