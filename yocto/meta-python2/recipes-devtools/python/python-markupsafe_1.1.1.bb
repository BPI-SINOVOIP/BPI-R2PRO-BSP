SUMMARY = "Implements a XML/HTML/XHTML Markup safe string for Python"
DESCRIPTION = "MarkupSafe implements a text object that escapes characters so \
it is safe to use in HTML and XML. Characters that have special meanings are \
replaced so that they display as the actual characters. This mitigates \
injection attacks, meaning untrusted user input can safely be displayed on a \
page."
HOMEPAGE = "https://palletsprojects.com/p/markupsafe/"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.rst;md5=ffeffa59c90c9c4a033c7574f8f3fb75"

SRC_URI[md5sum] = "43fd756864fe42063068e092e220c57b"
SRC_URI[sha256sum] = "29872e92839765e546828bb7754a68c418d927cd064fd4708fab9fe9c8bb116b"

PYPI_PACKAGE = "MarkupSafe"

inherit pypi setuptools

RDEPENDS_${PN} += "${PYTHON_PN}-re ${PYTHON_PN}-stringold"

BBCLASSEXTEND = "native nativesdk"
