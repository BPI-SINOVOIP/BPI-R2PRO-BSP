SUMMARY = "Python package for creating and manipulating graphs and networks"
DESCRIPTION = "NetworkX is a Python package for the creation, manipulation, \
and study of the structure, dynamics, and functions of complex networks."
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=3376ff7c9c58048c62d91431f7f08cde"

SRC_URI[md5sum] = "82608a3686fb3e61f20cf13bfd3c1b4a"
SRC_URI[sha256sum] = "45e56f7ab6fe81652fb4bc9f44faddb0e9025f469f602df14e3b2551c2ea5c8b"

inherit pypi setuptools

PYPI_PACKAGE_EXT = "zip"

RDEPENDS_${PN} += "${PYTHON_PN}-decorator"
