# RockX Command Line Demo

## RK3399Pro Android

### Prepare

Download NDK from [Link](https://developer.android.google.cn/ndk/downloads).

Then modify `ANDROID_NDK_PATH` which in the `demo/build-android-rk3399pro-v8a.sh` or `demo/build-android-rk3399pro-v7a.sh` file  to your installed NDK path.

### Build

```
cd demo
./build-android-rk3399pro-v8a.sh
```

Build output path: `install/rockx_rk3399pro_android_arm64-v8a`

### Install

Install to RK3399Pro device

```
adb push install/rockx_rk3399pro_android_arm64-v8a /data/
```

### Run

```
adb shell
cd /data/rockx_rk3399pro_android_arm64-v8a/rockx_face_landmark_demo
export LD_LIBRARY_PATH=../lib/
./rockx_face_landmark face4.jpg 68
```

After execution, it will generate `out_landmark.jpg`.

## RK3399Pro Linux (Cross Compile)

### Prepare

Download cross compile toolchain from [Link](https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/).

Then modify `GCC_COMPILER_PATH` which in the `demo/build-linux-rk3399pro.sh` to your toolchain path.

### Build

```
cd demo
./build-linux-rk3399pro.sh
```

Build output path: `install/rockx_rk3399pro_linux_aarch64`

### Install

```
adb push install/rockx_rk3399pro_linux_aarch64 /userdata/
```

or

```
scp -r install/rockx_rk3399pro_linux_aarch64 toybrick@<device-ip>:/home/toybrick/
```

### Run

```
cd rockx_rk3399pro_linux_aarch64/rockx_face_landmark_demo
./rockx_face_landmark face4.jpg 68
```

After execution, it will generate `out_landmark.jpg`.

## RK3399Pro Linux (Build On Device)

### Prepare

install `cmake`, `gcc` and `g++`

### Build

```
./build-linux-rk3399pro-on-device.sh
```

### Run

```
cd install/rockx_linux_rk3399pro/rockx_face_landmark_demo
./rockx_face_landmark face4.jpg 68
```

After execution, it will generate `out_landmark.jpg`.

## RK1808 Linux (Cross Compile)

### Prepare

Download cross compile toolchain from [Link](https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/).

Then modify `GCC_COMPILER_PATH` which in the `demo/build-linux-rk1808.sh` to your toolchain path.

### Build

```
cd demo
./build-linux-rk1808.sh
```

Build output path: `install/rockx_rk1808_linux_aarch64`

### Install

```
adb push install/rockx_rk1808_linux_aarch64 /userdata/
```

### Run

```
cd /userdata/rockx_rk1808_linux_aarch64/rockx_face_landmark_demo
./rockx_face_landmark face4.jpg 68
```

After execution, it will generate `out_landmark.jpg`.

## PC Linux

### Prepare

Install `cmake`, `gcc` and`g++`

### Build

```
./build-linux-x86.sh
```

### Run

Insert RK1808 Compute Stick, then execute the command below:

```
cd install/rockx_linux_x86/rockx_face_landmark_demo
./rockx_face_landmark face4.jpg 68
```

After execution, it will generate `out_landmark.jpg`.