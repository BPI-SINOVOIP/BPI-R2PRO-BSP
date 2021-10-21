SUMMARY = "A high-level Python Web framework"
DESCRIPTION = "A high-level Python Web framework that encourages rapid \
development and clean, pragmatic design."
HOMEPAGE = "http://www.djangoproject.com/"
SECTION = "devel/python"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f09eb47206614a4954c51db8a94840fa"

SRC_URI[md5sum] = "858e5417a10ce565a15d6e4a2ea0ee37"
SRC_URI[sha256sum] = "861db7f82436ab43e1411832ed8dca81fc5fc0f7c2039c7e07a080a63092fb44"

PYPI_PACKAGE = "Django"

UPSTREAM_CHECK_REGEX = "(?P<pver>1(\.\d+)+)"

FILES_${PN} += "${datadir}/django"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-argparse \
    ${PYTHON_PN}-compression \
    ${PYTHON_PN}-ctypes \
    ${PYTHON_PN}-datetime \
    ${PYTHON_PN}-email \
    ${PYTHON_PN}-html \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-logging \
    ${PYTHON_PN}-multiprocessing \
    ${PYTHON_PN}-netserver \
    ${PYTHON_PN}-numbers \
    ${PYTHON_PN}-pkgutil \
    ${PYTHON_PN}-pytz \
    ${PYTHON_PN}-subprocess \
    ${PYTHON_PN}-threading \
    ${PYTHON_PN}-unixadmin \
    ${PYTHON_PN}-xml \
    ${PYTHON_PN}-zlib \
"

BBCLASSEXTEND = "native nativesdk"
