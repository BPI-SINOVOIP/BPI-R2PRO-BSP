SUMMARY = "Module for decorators, wrappers and monkey patching."
DESCRIPTION = "The aim of the wrapt module is to provide a transparent object \
proxy for Python, which can be used as the basis for the construction of \
function wrappers and decorator functions.\
\
The wrapt module focuses very much on correctness. It therefore goes way \
beyond existing mechanisms such as functools.wraps() to ensure that decorators \
preserve introspectability, signatures, type checking abilities etc. The \
decorators that can be constructed using this module will work in far more \
scenarios than typical decorators and provide more predictable and consistent \
behaviour.\
\
To ensure that the overhead is as minimal as possible, a C extension module is \
used for performance critical components. An automatic fallback to a pure \
Python implementation is also provided where a target system does not have a \
compiler to allow the C extension to be compiled."
HOMEPAGE = "https://github.com/GrahamDumpleton/wrapt"
SECTION = "devel/python"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=fdfc019b57affbe1d7a32e3d34e83db4"

PYPI_PACKAGE = "wrapt"
SRC_URI[md5sum] = "cc15c001b129f81eb2f79b15eb99ffe5"
SRC_URI[sha256sum] = "565a021fd19419476b9362b05eeaa094178de64f8361e44468f9e9d7843901e1"

inherit pypi setuptools

S = "${WORKDIR}/${PYPI_PACKAGE}-${PV}"

RDEPENDS_${PN}_append_class-target = "\
    ${PYTHON_PN}-lang \
    ${PYTHON_PN}-threading \
"
