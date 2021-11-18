#!/bin/bash
OPENWRT_VERSION=$1
OPENWRT_DEFCOFNIG=$2

TOP_DIR=$(pwd)
OPENWRT_DIR="$TOP_DIR/$OPENWRT_VERSION"
OPENWRT_CONFIG_PATH="$OPENWRT_DIR/configs/$OPENWRT_DEFCOFNIG"

if [ ! -d "$OPENWRT_DIR" ];then
        echo "$OPENWRT_DIR is not exist"
        exit 1
fi

if [ ! -e "$OPENWRT_CONFIG_PATH" ];then
        echo "$OPENWRT_CONFIG_PATH is not exist"
        exit 1
fi

cd $OPENWRT_DIR

./scripts/feeds update -a
./scripts/feeds install -a

echo "using $OPENWRT_DEFCOFNIG"
cp "$OPENWRT_CONFIG_PATH" .config && make defconfig

make download -j$(nproc)
find dl -size -1024c -exec ls -l {} \;
find dl -size -1024c -exec rm -f {} \;

make -j$(nproc) V=s
RET=$?
cd -
exit $RET
