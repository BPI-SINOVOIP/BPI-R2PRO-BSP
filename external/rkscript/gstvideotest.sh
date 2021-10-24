#!/bin/sh

gst-launch-1.0 -v videotestsrc pattern=snow ! video/x-raw,width=1280,heigh=720 ! kmssink
