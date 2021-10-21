SUMMARY = "Python API for MISP"
DESCRIPTION = "PyMISP is a Python library to access MISP platforms via their \
REST API. \
\
PyMISP allows you to fetch events, add or update events/attributes, add or \
update samples or search for attributes."
HOMEPAGE = "https://github.com/MISP/PyMISP"
LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a3639cf5780f71b125d3e9d1dc127c20"

SRC_URI[md5sum] = "57184785340e25469e4a80ff9ddce09e"
SRC_URI[sha256sum] = "594ea0a9e150052232425009eac6dd104a80f494d0e273cc48dd114d7ea8482b"

inherit pypi setuptools

RDEPENDS_${PN}_append_class-target = " \
    ${PYTHON_PN}-dateutil \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-jsonschema \
"

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-cachetools \
    ${PYTHON_PN}-deprecated \
    ${PYTHON_PN}-requests \
    ${PYTHON_PN}-six \
"
