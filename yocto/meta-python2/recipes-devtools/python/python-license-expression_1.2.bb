SUMMARY = "Utility library to parse, compare, simplify and normalize license expressions"
HOMEPAGE = "https://github.com/nexB/license-expression"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://apache-2.0.LICENSE;md5=e23fadd6ceef8c618fc1c65191d846fa"

SRC_URI[md5sum] = "fd4cb295cc345be1071274cdbd81c969"
SRC_URI[sha256sum] = "7960e1dfdf20d127e75ead931476f2b5c7556df05b117a73880b22ade17d1abc"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-booleanpy \
    "

RDEPENDS_${PN}_append_class-target = "\
    ${PYTHON_PN}-logging \
    "

BBCLASSEXTEND = "native nativesdk"
