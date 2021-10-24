#!/bin/sh

killall start_rknn.sh
killall rknn_server
killall rknn_demo

cd /usr/bin/; ./rknn_demo &
