#!/bin/bash -e

# Directory contains the target rootfs
TARGET_ROOTFS_DIR="binary"

if [ "$ARCH" == "armhf" ]; then
	ARCH='armhf'
elif [ "$ARCH" == "arm64" ]; then
	ARCH='arm64'
else
    echo -e "\033[36m please input is: armhf or arm64...... \033[0m"
fi

if [ ! $VERSION ]; then
	VERSION="release"
fi

if [ ! -e linaro-buster-alip-*.tar.gz ]; then
	echo "\033[36m Run mk-base-debian.sh first \033[0m"
fi

finish() {
	sudo umount $TARGET_ROOTFS_DIR/dev
	exit -1
}
trap finish ERR

echo -e "\033[36m Extract image \033[0m"
sudo tar -xpf linaro-buster-alip-*.tar.gz

# packages folder
sudo mkdir -p $TARGET_ROOTFS_DIR/packages
sudo cp -rf packages/$ARCH/* $TARGET_ROOTFS_DIR/packages

# overlay folder
sudo cp -rf overlay/* $TARGET_ROOTFS_DIR/

# overlay-firmware folder
sudo cp -rf overlay-firmware/* $TARGET_ROOTFS_DIR/

# overlay-debug folder
# adb, video, camera  test file
if [ "$VERSION" == "debug" ]; then
	sudo cp -rf overlay-debug/* $TARGET_ROOTFS_DIR/
fi
## hack the serial
sudo cp -f overlay/usr/lib/systemd/system/serial-getty@.service $TARGET_ROOTFS_DIR/lib/systemd/system/serial-getty@.service

# adb
if [[ "$ARCH" == "armhf" && "$VERSION" == "debug" ]]; then
	sudo cp -f overlay-debug/usr/local/share/adb/adbd-32 $TARGET_ROOTFS_DIR/usr/bin/adbd
elif [[ "$ARCH" == "arm64" && "$VERSION" == "debug" ]]; then
	sudo cp -f overlay-debug/usr/local/share/adb/adbd-64 $TARGET_ROOTFS_DIR/usr/bin/adbd
fi

# bt/wifi firmware
sudo mkdir -p $TARGET_ROOTFS_DIR/system/lib/modules/
sudo mkdir -p $TARGET_ROOTFS_DIR/vendor/etc
sudo find ../kernel/drivers/net/wireless/rockchip_wlan/*  -name "*.ko" | \
    xargs -n1 -i sudo cp {} $TARGET_ROOTFS_DIR/system/lib/modules/

echo -e "\033[36m Change root.....................\033[0m"
if [ "$ARCH" == "armhf" ]; then
	sudo cp /usr/bin/qemu-arm-static $TARGET_ROOTFS_DIR/usr/bin/
elif [ "$ARCH" == "arm64"  ]; then
	sudo cp /usr/bin/qemu-aarch64-static $TARGET_ROOTFS_DIR/usr/bin/
fi
sudo mount -o bind /dev $TARGET_ROOTFS_DIR/dev

cat << EOF | sudo chroot $TARGET_ROOTFS_DIR

apt-get update
apt-get upgrade -y

chmod o+x /usr/lib/dbus-1.0/dbus-daemon-launch-helper
chmod +x /etc/rc.local

#---------------system--------------
apt-get install -y git fakeroot devscripts cmake binfmt-support dh-make dh-exec pkg-kde-tools device-tree-compiler \
bc cpio parted dosfstools mtools dpkg-dev ntp rsyslog wget gdb net-tools inetutils-ping openssh-server \
ifupdown alsa-utils python vim ntp git libssl-dev vsftpd tcpdump can-utils i2c-tools strace network-manager onboard \
evtest sox libsox-fmt-all
apt-get install -f -y

#---------------power management --------------
apt-get install -y busybox pm-utils triggerhappy
cp /etc/Powermanager/triggerhappy.service  /lib/systemd/system/triggerhappy.service

#---------------Rga--------------
dpkg -i /packages/rga/*.deb

echo -e "\033[36m Setup Video.................... \033[0m"
apt-get install -y gstreamer1.0-plugins-bad gstreamer1.0-plugins-base gstreamer1.0-tools gstreamer1.0-alsa \
gstreamer1.0-plugins-base-apps qtmultimedia5-examples
apt-get install -f -y

dpkg -i  /packages/mpp/*
dpkg -i  /packages/gst-rkmpp/*.deb
dpkg -i  /packages/gst-base/*.deb
apt-mark hold gstreamer1.0-x
apt-get install -f -y

#---------Camera---------
echo -e "\033[36m Install camera.................... \033[0m"
apt-get install cheese v4l-utils -y
dpkg -i  /packages/rkisp/*.deb
dpkg -i  /packages/libv4l/*.deb

#---------Xserver---------
echo -e "\033[36m Install Xserver.................... \033[0m"
#apt-get build-dep -y xorg-server-source
apt-get install -y xserver-xorg-dev libaudit-dev libx11-xcb1 xtrans-dev xfonts-utils x11proto-dev libxdmcp-dev libxau-dev libxdmcp-dev libxfont-dev libxkbfile-dev libpixman-1-dev libpciaccess-dev libgcrypt-dev nettle-dev libudev-dev libselinux1-dev:arm64 libaudit-dev libgl1-mesa-dev libunwind-dev libxmuu-dev libxext-dev libx11-dev libxrender-dev libxi-dev libdmx-dev libxpm-dev libxaw7-dev libxt-dev libxmu-dev libxtst-dev libxres-dev libxfixes-dev libxv-dev libxinerama-dev libxshmfence-dev libepoxy-dev libegl1-mesa-dev libgbm-dev

apt-get install -f -y

dpkg -i /packages/xserver/*.deb
apt-get install -f -y
apt-mark hold xserver-common xserver-xorg-core xserver-xorg-legacy

#---------------Openbox--------------
echo -e "\033[36m Install openbox.................... \033[0m"
apt-get install -y openbox
dpkg -i  /packages/openbox/*.deb
apt-get install -f -y

#---------update chromium-----
apt-get install -y chromium
apt-get install -f -y /packages/chromium/*.deb

#------------------libdrm------------
echo -e "\033[36m Install libdrm.................... \033[0m"
dpkg -i  /packages/libdrm/*.deb
apt-get install -f -y

#------------------libdrm-cursor------------
echo -e "\033[36m Install libdrm-cursor.................... \033[0m"
dpkg -i  /packages/libdrm-cursor/*.deb
apt-get install -f -y

# Only preload libdrm-cursor for X
sed -i "/libdrm-cursor.so/d" /etc/ld.so.preload
sed -i "1aexport LD_PRELOAD=libdrm-cursor.so.1" /usr/bin/X

#------------------pcmanfm------------
echo -e "\033[36m Install pcmanfm.................... \033[0m"
dpkg -i  /packages/pcmanfm/*.deb
apt-get install -f -y

#------------------rkwifibt------------
echo -e "\033[36m Install rkwifibt.................... \033[0m"
dpkg -i  /packages/rkwifibt/*.deb
apt-get install -f -y
ln -s /system/etc/firmware /vendor/etc/

if [ "$VERSION" == "debug" ]; then
#------------------glmark2------------
echo -e "\033[36m Install glmark2.................... \033[0m"
dpkg -i  /packages/glmark2/*.deb
apt-get install -f -y
fi

# mark package to hold
# apt-mark hold libv4l-0 libv4l2rds0 libv4lconvert0 libv4l-dev v4l-utils
#apt-mark hold librockchip-mpp1 librockchip-mpp-static librockchip-vpu0 rockchip-mpp-demos
#apt-mark hold xserver-common xserver-xorg-core xserver-xorg-legacy
#apt-mark hold libegl-mesa0 libgbm1 libgles1 alsa-utils
#apt-get install -f -y

#---------------Custom Script--------------
systemctl mask systemd-networkd-wait-online.service
systemctl mask NetworkManager-wait-online.service
rm /lib/systemd/system/wpa_supplicant@.service

#---------------Clean--------------
rm -rf /var/lib/apt/lists/*

EOF

sudo umount $TARGET_ROOTFS_DIR/dev
