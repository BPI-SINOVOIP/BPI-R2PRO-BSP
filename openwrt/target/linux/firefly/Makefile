# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

BOARD:=firefly
BOARDNAME:=Firefly
FEATURES:=targz gpio rtc usb fpu
SUBTARGETS:=armv8

KERNEL_PATCHVER=5.4

define Target/Description
	Build firmware images for firefly boards
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image

$(eval $(call BuildTarget))
