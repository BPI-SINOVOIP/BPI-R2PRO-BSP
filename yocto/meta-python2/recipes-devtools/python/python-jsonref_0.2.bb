SUMMARY = "jsonref is a library for automatic dereferencing of JSON Reference objects for Python"
DESCRIPTION = "jsonref is a library for automatic dereferencing of JSON \
Reference objects for Python (supporting Python 2.6+ and Python 3.3+).\
\
This library lets you use a data structure with JSON reference objects, as if \
the references had been replaced with the referent data."
HOMEPAGE = "https://github.com/gazpachoking/jsonref"
SECTION = "devel/python"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a34264f25338d41744dca1abfe4eb18f"

SRC_URI[md5sum] = "42b518b9ccd6852d1d709749bc96cb70"
SRC_URI[sha256sum] = "f3c45b121cf6257eafabdc3a8008763aed1cd7da06dbabc59a9e4d2a5e4e6697"

inherit pypi setuptools

BBCLASSEXTEND = "native nativesdk"
