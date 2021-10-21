SUMMARY = "A cross-platform clipboard module for Python."
DESCRIPTION = "Pyperclip is a cross-platform Python module for copy and paste \
clipboard functions (only handles plain text for now)."
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://PKG-INFO;md5=a428356ada7737b416ec4b63dc65d581"

SRC_URI[md5sum] = "6bbb8598579cc3ee50554b4c59d0cfae"
SRC_URI[sha256sum] = "979325468ccf682104d5dcaf753f869868100631301d3e72f47babdea5700d1c"

inherit pypi setuptools

RDEPENDS_${PN} += " \
    ${PYTHON_PN}-ctypes \
    ${PYTHON_PN}-contextlib \
    ${PYTHON_PN}-subprocess \
"

BBCLASSEXTEND = "native nativesdk"

