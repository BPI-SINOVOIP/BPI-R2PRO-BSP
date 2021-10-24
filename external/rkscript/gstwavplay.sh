#!/bin/sh

audiofile=$1

if [ -z $audiofile ]
then
	audiofile=/usr/share/sounds/alsa/Noise.wav
fi

gst-launch-1.0 filesrc location="$audiofile" ! wavparse ! audioconvert ! autoaudiosink
