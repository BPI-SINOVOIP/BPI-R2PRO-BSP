#!/bin/bash

unset FREQS
read -a FREQS < /sys/class/devfreq/10091000.gpu/available_frequencies

RANDOM=$$$(date +%s)
while true; do
  echo userspace > /sys/class/devfreq/10091000.gpu/governor
  FREQ=${FREQS[$RANDOM % ${#FREQS[@]} ]}
  echo GPU:Now ${FREQ}
  echo ${FREQ} > /sys/class/devfreq/10091000.gpu/userspace/set_freq
  sleep 1
done
