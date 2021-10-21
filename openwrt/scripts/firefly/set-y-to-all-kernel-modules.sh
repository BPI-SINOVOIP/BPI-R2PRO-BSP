#!/bin/bash
set -e

config_file=$1

if grep -Eq '^CONFIG_ALL_KMODS=y' $config_file;then
    sed -i "s|^\(CONFIG_PACKAGE_kmod.*\)=m|\1=y|" $config_file
fi
