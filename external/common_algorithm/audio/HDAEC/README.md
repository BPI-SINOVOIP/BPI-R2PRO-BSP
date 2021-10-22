# Rockchip HDAEC算法

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

* 内存占用情况：RSS 1.5M
* 计算耗时 cpu 9%

  * ./test_hdaec_file txin.pcm rxin.pcm txout.pcm rxout.pcm

    elapse 4 ms   //50ms数据
	elapse 5 ms   //50ms数据

* 稳定性测试
  	* 多实例循环压力测试通过：没有内存泄露，可以稳定长时运行

	# cat /proc/1194/status
	Name:   test_hdaec_file
	State:  R (running)
	Tgid:   1194
	Ngid:   0
	Pid:    1194
	PPid:   645
	TracerPid:      0
	Uid:    0       0       0       0
	Gid:    0       0       0       0
	FDSize: 64
	Groups:
	VmPeak:     2376 kB
	VmSize:     2376 kB
	VmLck:         0 kB
	VmPin:         0 kB
	VmHWM:      1444 kB
	VmRSS:      1444 kB
	VmData:      160 kB
	VmStk:       132 kB
	VmExe:         4 kB
	VmLib:      1852 kB
	VmPTE:        20 kB
	VmPMD:         8 kB
	VmSwap:        0 kB
	Threads:        1
