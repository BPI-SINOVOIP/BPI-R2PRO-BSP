#!/bin/sh
#
# setup wayland environment for weston, start glmark2....
#

mkdir -p /tmp/.xdg &&  chmod 0700 /tmp/.xdg
export XDG_RUNTIME_DIR=/tmp/.xdg
weston --tty=2 --idle-time=0&
sleep 1
glmark2-es2-wayland --fullscreen
