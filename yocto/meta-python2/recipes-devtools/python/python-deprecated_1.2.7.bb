SUMMARY = "Python @deprecated decorator to deprecate old python classes, functions or methods."
HOMEPAGE = "https://github.com/tantale/deprecated"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.rst;md5=44288e26f4896bdab14072d4fa35ff01"

SRC_URI[md5sum] = "93d65f6a4efc9aebebfd28c9413dfd19"
SRC_URI[sha256sum] = "408038ab5fdeca67554e8f6742d1521cd3cd0ee0ff9d47f29318a4f4da31c308"

PYPI_PACKAGE = "Deprecated"

inherit pypi setuptools

S = "${WORKDIR}/${PYPI_PACKAGE}-${PV}"

RDEPENDS_${PN} += "python-wrapt"
