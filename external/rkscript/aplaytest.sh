#!/bin/sh

aplay -Dhw:0,0 -t raw -r 48000 -c 2 -fS16_LE /dev/urandom
