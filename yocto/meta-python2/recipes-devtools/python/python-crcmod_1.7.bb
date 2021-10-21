SUMMARY = "A Python module for generating objects that compute the Cyclic Redundancy Check."
DESCRIPTION = "The software in this package is a Python module for generating \
objects that compute the Cyclic Redundancy Check (CRC). There is no attempt in \
this package to explain how the CRC works. There are a number of resources on \
the web that give a good explanation of the algorithms. Just do a Google search \
for 'crc calculation' and browse till you find what you need. Another resource \
can be found in chapter 20 of the book 'Numerical Recipes in C' by Press et. al"
HOMEPAGE = "https://pypi.org/project/crcmod"
SECTION = "devel/python"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f9a19291627cad2d1dfbfcf3c9fb85c2"

SRC_URI[md5sum] = "2d5b92117d958dcead94f9e17f54cd32"
SRC_URI[sha256sum] = "dc7051a0db5f2bd48665a990d3ec1cc305a466a77358ca4492826f41f283601e"

inherit pypi setuptools

RDEPENDS_${PN} += "${PYTHON_PN}-unittest"

BBCLASSEXTEND = "native nativesdk"
