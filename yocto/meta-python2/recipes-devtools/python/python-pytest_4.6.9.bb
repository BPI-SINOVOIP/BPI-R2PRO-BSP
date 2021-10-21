SUMMARY = "Simple powerful teting with python"
HOMEPAGE = "http://pytest.org"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=81eb9f71d006c6b268cf4388e3c98f7b"

SRC_URI += " file://0001-setup.py-remove-setup_requires-for-setuptools-scm.patch"

SRC_URI[md5sum] = "d0457c5ddd0438e3b68b7939339d915f"
SRC_URI[sha256sum] = "19e8f75eac01dd3f211edd465b39efbcbdc8fc5f7866d7dd49fedb30d8adf339"

inherit update-alternatives pypi setuptools

RDEPENDS_${PN}_class-target += " \
    ${PYTHON_PN}-argparse \
    ${PYTHON_PN}-atomicwrites \
    ${PYTHON_PN}-attrs \
    ${PYTHON_PN}-compiler \
    ${PYTHON_PN}-debugger \
    ${PYTHON_PN}-doctest \
    ${PYTHON_PN}-funcsigs \
    ${PYTHON_PN}-importlib-metadata \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-logging \
    ${PYTHON_PN}-more-itertools \
    ${PYTHON_PN}-packaging \
    ${PYTHON_PN}-pathlib2 \
    ${PYTHON_PN}-pluggy \
    ${PYTHON_PN}-py \
    ${PYTHON_PN}-setuptools \
    ${PYTHON_PN}-six \
    ${PYTHON_PN}-wcwidth \
"

RDEPENDS_${PN}-ptest += "\
    ${PYTHON_PN}-hypothesis \
"

FILESEXTRAPATHS_prepend := "${THISDIR}/python-pytest:"

ALTERNATIVE_${PN} += "py.test pytest"

NATIVE_LINK_NAME[pytest] = "${bindir}/pytest"
ALTERNATIVE_TARGET[pytest] = "${bindir}/pytest"

ALTERNATIVE_LINK_NAME[py.test] = "${bindir}/py.test"
ALTERNATIVE_TARGET[py.test] = "${bindir}/py.test"

ALTERNATIVE_PRIORITY = "10"

BBCLASSEXTEND = "native nativesdk"
