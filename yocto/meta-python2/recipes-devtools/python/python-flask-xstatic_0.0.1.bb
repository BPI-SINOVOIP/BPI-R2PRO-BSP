SUMMARY = "Flask support for XStatic assets"
DESCRIPTION = "Flask is a Python microframework for building websites. \
\
XStatic is a format for packaging static assets. For example, XStatic-jQuery \
is jquery in a form that you can pip install. \
\
This (Flask-XStatic) is a Flask extension that simplifies using XStatic assets \
in your project."
LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=659968f6ebd4b70b6c3190d20b4a924c"

SRC_URI[md5sum] = "2f56023e1444c8bd1fec41afe93de743"
SRC_URI[sha256sum] = "226ea8e97065a9488b59bfe5c94af4c6e2ea70a25052e301fb231a1381490133"

FILESEXTRAPATHS_prepend := "${THISDIR}/python-flask-xstatic:"
SRC_URI += "file://remove-pip-requires.patch"

PYPI_PACKAGE = "Flask-XStatic"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-flask \
    ${PYTHON_PN}-xstatic \
    "
