#!/bin/bash

cd /sys/devices/system/cpu/cpu0/cpufreq
echo userspace > scaling_governor
unset FREQS_LIT
read -a FREQS_LIT < scaling_available_frequencies

cd /sys/devices/system/cpu/cpu1/cpufreq
echo userspace > scaling_governor
unset FREQS_BIG
read -a FREQS_BIG < scaling_available_frequencies

RANDOM=$$$(date +%s)
while true; do
  FREQ_LIT=${FREQS_LIT[$RANDOM % ${#FREQS_LIT[@]} ]}
  FREQ_BIG=${FREQS_BIG[$RANDOM % ${#FREQS_BIG[@]} ]}
  echo CPU:Now 0:${FREQ_LIT} and 1:${FREQ_BIG}
  echo ${FREQ_LIT} > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
  echo ${FREQ_BIG} > /sys/devices/system/cpu/cpu1/cpufreq/scaling_setspeed
  sleep 1
done
