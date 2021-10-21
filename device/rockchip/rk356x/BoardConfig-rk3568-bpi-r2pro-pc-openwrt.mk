#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/BoardConfig.mk

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rk3568-bpi-r2pro
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rockchip_bpi-r2pro_linux_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3568-bpi-r2pro-pc
# packagefile for make update image
export RK_PACKAGE_FILE=rk356x-package-file-bpi-r2pro-openwrt
# parameter for GPT table
export RK_PARAMETER=parameter-rk3568-bpi-r2pro-pc-openwrt-fit.txt

# Buildroot config
export RK_CFG_BUILDROOT=
# Recovery config
export RK_CFG_RECOVERY=
# OEM config
export RK_OEM_DIR=
# userdata config
 export RK_USERDATA_DIR=
# rootfs_system
export RK_ROOTFS_SYSTEM=openwrt
