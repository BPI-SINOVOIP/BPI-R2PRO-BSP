SUMMARY = "Self-service finite-state machines for the programmer on the go"
DESCRIPTION = "Automat is a library for concise, idiomatic Python expression \
of finite-state automata (particularly deterministic finite-state transducers)."
HOMEPAGE = "https://github.com/glyph/Automat"
SECTION = "devel/python"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4ad213bcca81688e94593e5f60c87477"

FILESEXTRAPATHS_prepend := "${THISDIR}/python-automat:"

SRC_URI[md5sum] = "47e980a48201a1dabe37fa11f0187013"
SRC_URI[sha256sum] = "269a09dfb063a3b078983f4976d83f0a0d3e6e7aaf8e27d8df1095e09dc4a484"

SRC_URI_append = " \
    file://0001-setup.py-remove-the-dependency-on-m2r.patch \
"

DEPENDS += "${PYTHON_PN}-setuptools-scm-native"

PYPI_PACKAGE = "Automat"

inherit pypi setuptools

RDEPENDS_${PN} += "\
   ${PYTHON_PN}-attrs \
   ${PYTHON_PN}-six \
"
