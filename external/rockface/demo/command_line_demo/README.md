# RockFace Command Line Demo

## Build and Install Demo

### RK1808 Linux (Cross Compile)

#### Prepare

Download cross compile toolchain from [Link](https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/) or use sdk builtin toolchain.

Then modify `GCC_COMPILER_PATH` which in the `demo/build-linux-rk1808.sh` to your toolchain path.

#### Build

```
cd demo
./build-linux-rk1808.sh
```

Build output path: `install/rockface_rk1808_linux_aarch64`

#### Install

```
adb push install/rockface_rk1808_linux_aarch64 /userdata/
```

### RK3399Pro Linux (Cross Compile)

#### Prepare

Download cross compile toolchain from [Link](https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/) or use SDK builtin toolchain.

Then modify `GCC_COMPILER_PATH` which in the `demo/build-linux-rk3399pro.sh` to your toolchain path.

#### Build

```
cd demo
./build-linux-rk3399pro.sh
```

Build output path: `install/rockface_rk3399pro_linux_aarch64`

#### Install

```
adb push install/rockface_rk3399pro_linux_aarch64 /userdata/
```

or

```
scp -r install/rockface_rk3399pro_linux_aarch64 toybrick@<device-ip>:/home/toybrick/
```

### RK3399Pro Linux (Build On Device)

#### Prepare

install `cmake`, `gcc` and `g++`

#### Build

```
./build-linux-rk3399pro-on-device.sh
```

Build output path: `install/rockface_rk3399pro_linux_aarch64`

## RK3399Pro Android

### Prepare

Download NDK from [Link](https://developer.android.google.cn/ndk/downloads).

Then modify `ANDROID_NDK_PATH` which in the `demo/build-android-rk3399pro.sh` file to your installed NDK path.

### Build

```
cd demo
./build-android-rk3399pro.sh
```

Build output path: `install/rockface_rk3399pro_android`

### Install

Install to RK3399Pro device

```
adb push install/rockface_rk3399pro_android /data/
```

## Run Demo

### rockface_analyze

```
adb shell
cd /data/rockface_rk3399pro_android/rockface_analyze_demo
./face_analyze /path/to/key.lic Aaron_Eckhart_0001.jpg
```

NOTICE: android need set environment `export LD_LIBRARY_PATH=../lib` before run.

### rockface_recognition_1v1

```
cd rockface_recognition_1v1
./recognition_1v1 /path/to/key.lic Aaron_Eckhart_0001.jpg Aaron_Sorkin_0001.jpg
```

### rockface_recognition_1vn

- import library

put all image into one folder.

```
./import_face_library /path/to/key.lic /path/to/image_dir /path/to/rockface.db
```

- face search

```
./recognition_1vn /path/to/key.lic /path/to/face.jpg /path/to/face.db
```