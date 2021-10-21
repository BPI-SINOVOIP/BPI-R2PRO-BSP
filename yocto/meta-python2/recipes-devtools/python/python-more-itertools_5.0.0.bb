SUMMARY = "More routines for operating on iterables, beyond itertools"
DESCRIPTION = "Python’s itertools library is a gem - you can compose elegant \
solutions for a variety of problems with the functions it provides. In \
more-itertools we collect additional building blocks, recipes, and routines \
for working with Python iterables."
HOMEPAGE = "https://github.com/erikrose/more-itertools"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3396ea30f9d21389d7857719816f83b5"

SRC_URI[md5sum] = "f2ea58aa336ce6c13b7b225b3bbe305d"
SRC_URI[sha256sum] = "38a936c0a6d98a38bcc2d03fdaaedaba9f412879461dd2ceff8d37564d6522e4"

inherit pypi setuptools

PE = "1"

BBCLASSEXTEND = "native nativesdk"
