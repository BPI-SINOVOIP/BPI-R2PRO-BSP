#!/bin/sh

gst-launch-1.0 audiotestsrc ! audioconvert ! autoaudiosink
