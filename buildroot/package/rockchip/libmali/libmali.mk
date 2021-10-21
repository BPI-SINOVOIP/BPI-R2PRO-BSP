################################################################################
#
# libmali
#
################################################################################

LIBMALI_VERSION = develop
LIBMALI_SITE = $(TOPDIR)/../external/libmali
LIBMALI_SITE_METHOD = local

LIBMALI_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_LIBMALI_HAS_EGL),y)
LIBMALI_PROVIDES += libegl
endif

ifeq ($(BR2_PACKAGE_LIBMALI_HAS_GBM),y)
LIBMALI_PROVIDES += libgbm
endif

ifeq ($(BR2_PACKAGE_LIBMALI_HAS_GLES),y)
LIBMALI_PROVIDES += libgles
endif

ifeq ($(BR2_PACKAGE_LIBMALI_HAS_OPENCL),y)
LIBMALI_PROVIDES += libopencl
endif

LIBMALI_DEPENDENCIES = libdrm

ifeq ($(BR2_PACKAGE_LIBMALI_ONLY_CL),y)
LIBMALI_PLATFORM = only-cl
else ifeq ($(BR2_PACKAGE_LIBMALI_DUMMY),y)
LIBMALI_PLATFORM = dummy
else ifeq ($(BR2_PACKAGE_LIBMALI_WAYLAND),y)
LIBMALI_PLATFORM = wayland
LIBMALI_DEPENDENCIES += wayland
else ifeq ($(BR2_PACKAGE_LIBMALI_X11),y)
LIBMALI_PLATFORM = x11
LIBMALI_DEPENDENCIES += libxcb xlib_libX11
else ifeq ($(BR2_PACKAGE_LIBMALI_GBM),y)
LIBMALI_PLATFORM = gbm
endif

ifeq ($(BR2_PACKAGE_PX3SE),y)
LIBMALI_GPU = utgard-400
LIBMALI_VER = r7p0
LIBMALI_SUBVER = r3p0
else ifneq ($(BR2_PACKAGE_RK312X)$(BR2_PACKAGE_RK3128H)$(BR2_PACKAGE_RK3036)$(BR2_PACKAGE_RK3032),)
LIBMALI_GPU = utgard-400
LIBMALI_VER = r7p0
LIBMALI_SUBVER = r1p1
else ifeq ($(BR2_PACKAGE_RK3328),y)
LIBMALI_GPU = utgard-450
LIBMALI_VER = r7p0
else ifeq ($(BR2_PACKAGE_RK3288),y)
LIBMALI_GPU = midgard-t76x
LIBMALI_VER = r18p0
LIBMALI_SUBVER = all
else ifneq ($(BR2_PACKAGE_RK3399)$(BR2_PACKAGE_RK3399PRO),)
LIBMALI_GPU = midgard-t86x
LIBMALI_VER = r18p0
else ifneq ($(BR2_PACKAGE_RK3326)$(BR2_PACKAGE_PX30),)
LIBMALI_GPU = bifrost-g31
LIBMALI_VER = rxp0
else ifeq ($(BR2_PACKAGE_RK356X),y)
LIBMALI_GPU = bifrost-g52
LIBMALI_VER = g2p0
endif

LIBMALI_CONF_OPTS = -Dwith-overlay=true -Dopencl-icd=false -Dkhr-header=true \
		    -Dplatform=$(LIBMALI_PLATFORM) -Dgpu=$(LIBMALI_GPU) \
		    -Dversion=$(LIBMALI_VER)

ifeq ($(BR2_PACKAGE_LIBMALI_WITHOUT_CL),y)
LIBMALI_SUBVER += without-cl
endif

ifeq ($(BR2_PACKAGE_LIBMALI_WITH_DUMMY)|$(BR2_PACKAGE_LIBMALI_DUMMY),y|)
LIBMALI_SUBVER += dummy
endif

LIBMALI_CONF_OPTS += -Dsubversion=$(subst $(eval) $(eval),-,$(LIBMALI_SUBVER))

$(eval $(meson-package))
