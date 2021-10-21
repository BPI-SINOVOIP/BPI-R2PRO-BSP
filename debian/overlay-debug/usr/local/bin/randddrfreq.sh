#!/bin/bash


unset FREQS
read -a FREQS < /sys/class/devfreq/dmc/available_frequencies

RANDOM=$$$(date +%s)
while true; do
  echo userspace > /sys/class/devfreq/dmc/governor
  FREQ=${FREQS[$RANDOM % ${#FREQS[@]} ]}
  echo DDR:Now ${FREQ}
  echo ${FREQ} > /sys/class/devfreq/dmc/userspace/set_freq
  sleep 1
done
