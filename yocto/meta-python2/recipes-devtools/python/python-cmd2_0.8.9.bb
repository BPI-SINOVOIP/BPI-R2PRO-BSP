SUMMARY = "Extra features for standard library's cmd module"
DESCRIPTION = "cmd2 is a tool for building interactive command line \
applications in Python. Its goal is to make it quick and easy for developers \
to build feature-rich and user-friendly interactive command line applications. \
It provides a simple API which is an extension of Python's built-in cmd module.\
 cmd2 provides a wealth of features on top of cmd to make your life easier and \
eliminates much of the boilerplate code which would be necessary when using cmd."
HOMEPAGE = "http://packages.python.org/cmd2/"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=60bc6842001870a418935bd84570b676"

SRC_URI[md5sum] = "878976772c305486dfbca3aff4b4e779"
SRC_URI[sha256sum] = "145cb677ebd0e3cae546ab81c30f6c25e0b08ba0f1071df854d53707ea792633"

DEPENDS += "${PYTHON_PN}-setuptools-scm-native"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-argparse \
    ${PYTHON_PN}-doctest \
    ${PYTHON_PN}-pyparsing \
    ${PYTHON_PN}-pyperclip \
    ${PYTHON_PN}-shell \
    ${PYTHON_PN}-six \
    ${PYTHON_PN}-stringold \
    ${PYTHON_PN}-subprocess \
    ${PYTHON_PN}-threading \
    ${PYTHON_PN}-textutils \
    "

BBCLASSEXTEND = "native nativesdk"
