# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Tobias Maedel

define Device/iHC-3308GW
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := iHC-3308GW
  SOC := rk3308
endef
TARGET_DEVICES += iHC-3308GW

define Device/ROC-RK3568-PC
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := ROC-RK3568-PC
  SOC := rk3568
endef
TARGET_DEVICES += ROC-RK3568-PC
