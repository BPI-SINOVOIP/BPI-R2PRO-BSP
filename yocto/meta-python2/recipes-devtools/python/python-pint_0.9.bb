SUMMARY = "Physical quantities module"
DESCRIPTION = "Pint is a Python package to define, operate and manipulate \
physical quantities: the product of a numerical value and a unit of \
measurement. It allows arithmetic operations between them and conversions from \
and to different units. \
\
It is distributed with a comprehensive list of physical units, prefixes and \
constants. Due to its modular design, you can extend (or even rewrite!) the \
complete list without changing the source code. It supports a lot of numpy \
mathematical operations without monkey patching or wrapping numpy."
HOMEPAGE = "https://github.com/hgrecco/pint"
SECTION = "devel/python"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=bccf824202692270a1e0829a62e3f47b"

PYPI_PACKAGE = "Pint"

SRC_URI[md5sum] = "d0681cb7cfaca9fc68ce7edab0d08d88"
SRC_URI[sha256sum] = "32d8a9a9d63f4f81194c0014b3b742679dce81a26d45127d9810a68a561fe4e2"

inherit pypi setuptools

BBCLASSEXTEND = "native"
