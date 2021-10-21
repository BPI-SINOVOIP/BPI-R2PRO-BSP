SUMMARY = "A Python library providing data structure and operations for intervals"
DESCRIPTION = "This library provides data structure and operations for \
intervals in Python 2.7+ and Python 3.4+."
HOMEPAGE = "https://github.com/AlexandreDecan/python-intervals"
SECTION = "devel/python"

LICENSE = "LGPLv3"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=05f1e16a8e59ce3e9a979e881816c2ab"

SRC_URI[md5sum] = "8955317ff4e42590c90ba6247b1caaed"
SRC_URI[sha256sum] = "0d26746eaed0be78a61dd289bb7a10721b08770bb3e807614835f490d514f2a5"

PYPI_PACKAGE := "python-intervals"

inherit pypi setuptools

BBCLASSEXTEND = "native"
