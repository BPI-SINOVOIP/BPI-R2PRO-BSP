
SUMMARY = "Python Jinja2: A small but fast and easy to use stand-alone template engine written in pure python."
DESCRIPTION = "Python Jinja2: A small but fast and easy to use stand-alone template engine written in pure python."
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.rst;md5=5dc88300786f1c214c1e9827a5229462"

SRC_URI[md5sum] = "7883559bc5cc3e2781d94b4be61cfdcd"
SRC_URI[sha256sum] = "9fe95f19286cfefaa917656583d020be14e7859c6b0252588391e47db34527de"

PYPI_PACKAGE = "Jinja2"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-crypt \
    ${PYTHON_PN}-io \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-lang \
    ${PYTHON_PN}-markupsafe \
    ${PYTHON_PN}-math \
    ${PYTHON_PN}-netclient \
    ${PYTHON_PN}-pickle \
    ${PYTHON_PN}-pprint \
    ${PYTHON_PN}-re \
    ${PYTHON_PN}-shell \
    ${PYTHON_PN}-textutils \
    ${PYTHON_PN}-threading \
    ${PYTHON_PN}-numbers \
"

CLEANBROKEN = "1"

BBCLASSEXTEND = "native nativesdk"
