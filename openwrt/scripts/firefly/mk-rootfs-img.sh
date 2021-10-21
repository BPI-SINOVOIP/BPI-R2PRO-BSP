#!/bin/bash
if [ $# -lt 2 ] || [ ! -d $1 ] || [ ! -d $2 ]; then
    echo "Usage: $0 <rootfs dir> <output dir> [image size/MB]"
    echo "example:"
    echo "    ./build-rootfs-img.sh openwrt/rootfs ./ 104"
    exit 1
fi

ROOTFS_DIR=$1
OUTPUT_DIR=$2

ROOTFS_SIZE_MB=256
if [ $# -eq 3 ]; then
    ROOTFS_SIZE_MB=$3
fi

EXT4_BLOCKSIZE=4096

STAGING_DIR_HOST=../staging_dir/host
ROOTFS_PARTSIZE=$(($ROOTFS_SIZE_MB*1024*1024))
TIMESTAMP=$(date +%s)
TARGET_IMG=$OUTPUT_DIR/root.ext4

$STAGING_DIR_HOST/bin/make_ext4fs -L rootfs \
-l $ROOTFS_PARTSIZE -b $EXT4_BLOCKSIZE \
-m 0 -J -T $TIMESTAMP \
$TARGET_IMG $ROOTFS_DIR

if [ $? == 0 ];then
    echo "Success Build : $(realpath $TARGET_IMG)"
    file $(realpath $TARGET_IMG)
    exit 0
else
    echo "Failed to Build :$TARGET_IMG"
    exit 1
fi
