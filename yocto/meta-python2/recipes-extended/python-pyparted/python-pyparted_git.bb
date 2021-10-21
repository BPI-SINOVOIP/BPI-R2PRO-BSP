SUMMARY = "Python bindings for libparted"
DESCRIPTION = "pyparted is a set of Python modules that provide Python programmers \
an interface to libparted, the GNU parted library for disk partitioning and \
filesystem manipulation."
HOMEPAGE = "https://github.com/rhinstaller/pyparted"
SECTION = "devel/python"

LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "\
    file://COPYING;md5=8ca43cbc842c2336e835926c2166c28b \
    file://src/_pedmodule.c;beginline=10;endline=22;md5=9e53304db812b80d0939e11bb69dcab2 \
"

# 3.11.4 tag + gcc10 fixes for -fno-common
PV = "3.11.4+git${SRCPV}"

# upstream only publishes releases in github archives which are discouraged
SRCREV = "104ca13567c08c1188b126a395c4602841aae2c6"
SRC_URI = "git://github.com/rhinstaller/pyparted.git;protocol=https"

DEPENDS += "parted"

S = "${WORKDIR}/git"

RDEPENDS_${PN}_class-target += " \
    parted (>= 2.3) \
"
RDEPENDS_${PN}_class-native = ""

inherit distutils

RDEPENDS_${PN} += "python-stringold python-codecs python-math python-subprocess"
RDEPENDS_${PN}_class-native = ""

BBCLASSEXTEND = "native"
