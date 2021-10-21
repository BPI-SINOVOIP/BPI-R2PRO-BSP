SUMMARY = "Backport of functools.lru_cache from Python 3.3"
HOMEPAGE = "https://github.com/jaraco/backports.functools_lru_cache"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a33f38bbf47d48c70fe0d40e5f77498e"

PYPI_PACKAGE = "backports.functools_lru_cache"

SRC_URI[md5sum] = "103000b21a8e683647e2ce41929f2a9d"
SRC_URI[sha256sum] = "8fde5f188da2d593bd5bc0be98d9abc46c95bb8a9dde93429570192ee6cc2d4a"

DEPENDS += "python-setuptools-scm-native"

inherit setuptools pypi python-backports-init

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-pkgutil \
    ${PYTHON_PN}-pickle \
    ${PYTHON_PN}-threading \
    "

do_install() {
    install -d ${D}${PYTHON_SITEPACKAGES_DIR}/backports
    install ${B}/backports/functools_lru_cache.py ${D}${PYTHON_SITEPACKAGES_DIR}/backports/
}

FILES_${PN} = "${PYTHON_SITEPACKAGES_DIR}/backports/functools_lru_cache.py"

BBCLASSEXTEND = "native nativesdk"
