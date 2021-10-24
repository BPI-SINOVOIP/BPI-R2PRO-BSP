#!/bin/sh

audiofile=$1
samplerate=$2

if [ -z $samplerate ]
then
	samplerate=48000
fi

if [ -z $audiofile ]
then
	audiofile=/oem/piano2-CoolEdit.mp3
fi

minimad <$audiofile | aplay -t raw -f S16_LE -c 2 -r $samplerate
