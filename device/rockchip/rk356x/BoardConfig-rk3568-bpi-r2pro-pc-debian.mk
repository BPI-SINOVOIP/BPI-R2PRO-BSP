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
export RK_PACKAGE_FILE=rk356x-package-file-bpi-r2pro-debian
# build idblock
export RK_IDBLOCK_UPDATE=true
# parameter for GPT table
export RK_PARAMETER=parameter-rk3568-bpi-r2pro-pc-debian-fit.txt
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=ext4

# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk356x_recovery
# Recovery config
export RK_CFG_RECOVERY=rockchip_rk356x_recovery
# OEM config
export RK_OEM_DIR=
# userdata config
 export RK_USERDATA_DIR=
# rootfs_system
export RK_ROOTFS_SYSTEM=debian
