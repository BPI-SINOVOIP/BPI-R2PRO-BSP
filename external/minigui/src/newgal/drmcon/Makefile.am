## Makefile.am for MiniGUI using the framebuffer console video driver

AM_CPPFLAGS = -I$(abs_top_srcdir)/src/include -I$(abs_top_srcdir)/include \
			            -I$(abs_top_srcdir)/src/newgal/

noinst_LTLIBRARIES = libvideo_drmcon.la
libvideo_drmcon_la_SOURCES = $(DRMCON_SRCS)
libvideo_drmcon_la_CFLAGS = $(PIXMAN_CFLAGS)
libvideo_drmcon_la_LDFLAGS = $(PIXMAN_LIBS)

DRMCON_SRCS = \
    fbvideo.c fbvideo.h \
    drm_display.c drm_display.h libdrm_macros.h
