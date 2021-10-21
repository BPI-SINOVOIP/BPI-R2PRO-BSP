SUMMARY = "Resolve JSON Pointers in Python"
HOMEPAGE = "https://github.com/stefankoegl/python-json-pointer"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=32b15c843b7a329130f4e266a281ebb3"

SRC_URI[md5sum] = "741b98d0e693b08b5e44e0a9da5a7bb7"
SRC_URI[sha256sum] = "c192ba86648e05fdae4f08a17ec25180a9aef5008d973407b581798a83975362"

inherit pypi setuptools


RDEPENDS_${PN} += " \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-re \
"

BBCLASSEXTEND = "native nativesdk"
