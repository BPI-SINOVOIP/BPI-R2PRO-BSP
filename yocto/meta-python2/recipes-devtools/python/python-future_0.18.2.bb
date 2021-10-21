SUMMARY = "Clean single-source support for Python 3 and 2"
DESCRIPTION = "future is the missing compatibility layer between Python 2 and \
Python 3. It allows you to use a single, clean Python 3.x-compatible codebase \
to support both Python 2 and Python 3 with minimal overhead."
HOMEPAGE = "https://python-future.org"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=a253924061f8ecc41ad7a2ba1560e8e7"

SRC_URI[md5sum] = "e4579c836b9c025872efe230f6270349"
SRC_URI[sha256sum] = "b1bead90b70cf6ec3f0710ae53a525360fa360d306a86583adc6bf83a4db537d"

PYPI_PACKAGE_HASH = "99abde815842bc6e97d5a7806ad51236630da14ca2f3b1fce94c0bb94d3d"

inherit pypi setuptools

RDEPENDS_${PN}_append_class-target = " python-misc"

BBCLASSEXTEND = "native"
