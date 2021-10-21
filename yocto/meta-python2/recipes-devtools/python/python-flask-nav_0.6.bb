SUMMARY = "Easily create navigation for Flask applications."
DESCRIPTION = "Flask-Nav is a Flask-Extension to ease the creation of \
navigational Elements in Applications. It provides means to Express the \
Navigational structure and different ways to render these, making it easy to \
custom tailor it for your application."
HOMEPAGE = "https://github.com/mbr/flask-nav"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://PKG-INFO;md5=2729ee82259d601d90d28b0574d12416"

SRC_URI[md5sum] = "4d51cfd06d58f8d0fe85775a6696c0e5"
SRC_URI[sha256sum] = "44e40b755380a1e68ab521a2f9174de259a2c94ddcdaabf36b3aca2e110a33f4"

PYPI_PACKAGE = "flask-nav"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-blinker \
    ${PYTHON_PN}-flask \
    "
