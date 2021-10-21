#!/bin/bash

COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi

cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)
DEBIAN=$TOP_DIR/distro
sudo $DEBIAN/rkdebootstrap --base $* --shrink --package

if [ $? -ne 0 ]; then
	exit 1
fi

