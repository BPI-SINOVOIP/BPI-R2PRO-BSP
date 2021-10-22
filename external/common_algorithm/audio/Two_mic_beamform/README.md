# Rockchip TWO MIC BEAMFORM算法

## compile

mkdir -p build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/video/rv1109/buildroot/output/rockchip_puma_toolchain/host/share/buildroot/toolchainfile.cmake
make

优化选项： -O3 -mfpu=neon -mfloat-abi=hard

## 资源占用情况

### 测试环境

* RV1808 EVB

* cpu:  关闭ARM核保留一个核，定频 1Ghz

   echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

   echo 1008000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

   echo 0 > /sys/devices/system/cpu/cpu1/online

* ddr : LPDDR3 933MHz

### Test

* 内存占用情况：RSS 3.9M

* 计算耗时 cpu 90%

  * ./test_beamform hw:0,0 beamform.wav
    hw_params format set
    period_size = 256
    buffer_size = 1024
    set threshold 1
    alsa_name =hw:0,0 format=2 num_channels=2 sample_rate=16000 sample_bits=16 num_samples=2147483647
    Succeed to initialize skv_preprocessor!
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 9 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
    elapse 14 ms
  
* 稳定性测试
   * 多实例循环压力测试通过：没有内存泄露，可以稳定长时运行

   # cat /proc/1194/status
   / # cat /proc/656/status
   Name:   test_beamform
   State:  R (running)
   Tgid:   656
   Ngid:   0
   Pid:    656
   PPid:   609
   TracerPid:      0
   Uid:    0       0       0       0
   Gid:    0       0       0       0
   FDSize: 64
   Groups:
   VmPeak:     5404 kB
   VmSize:     5404 kB
   VmLck:         0 kB
   VmPin:         0 kB
   VmHWM:      3924 kB
   VmRSS:      3924 kB
   VmData:      476 kB
   VmStk:       132 kB
   VmExe:        12 kB
   VmLib:      4112 kB
   VmPTE:        28 kB
   VmPMD:         8 kB
   VmSwap:        0 kB
   Threads:        1
   SigQ:   0/7992
   SigPnd: 0000000000000000
   ShdPnd: 0000000000000000
   SigBlk: 0000000000000000
   SigIgn: 0000000000000000
   SigCgt: 0000000180000000
   CapInh: 0000000000000000
   CapPrm: 0000003fffffffff
   CapEff: 0000003fffffffff
   CapBnd: 0000003fffffffff
   CapAmb: 0000000000000000
   Speculation_Store_Bypass:       unknown
   Cpus_allowed:   3
   Cpus_allowed_list:      0-1
   Mems_allowed:   1
   Mems_allowed_list:      0
   voluntary_ctxt_switches:        875
   nonvoluntary_ctxt_switches:     9