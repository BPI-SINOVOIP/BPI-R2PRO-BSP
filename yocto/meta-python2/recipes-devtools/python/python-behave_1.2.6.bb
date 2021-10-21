SUMMARY = "A behavior-driven development framework, Python style"
DESCRIPTION = "Behavior-driven development (or BDD) is an agile software \
development technique that encourages collaboration between developers, QA and \
non-technical or business participants in a software project.\
\
behave uses tests written in a natural language style, backed up by Python code."
HOMEPAGE = "https://github.com/behave/behave"
SECTION = "devel/python"

LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d950439e8ea6ed233e4288f5e1a49c06"

SRC_URI[md5sum] = "3f05c859a1c45f5ed33e925817ad887d"
SRC_URI[sha256sum] = "b9662327aa53294c1351b0a9c369093ccec1d21026f050c3bd9b3e5cccf81a86"

inherit pypi setuptools

RDEPENDS_${PN} += " \
    ${PYTHON_PN}-argparse \
    ${PYTHON_PN}-enum34 \
    ${PYTHON_PN}-parse-type \
    ${PYTHON_PN}-setuptools \
    ${PYTHON_PN}-six \
    ${PYTHON_PN}-traceback2 \
    "
