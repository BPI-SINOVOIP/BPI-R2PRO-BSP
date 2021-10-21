SUMMARY = "A high-level Python efficient arrays of booleans -- C extension"
DESCRIPTION = "This module provides an object type which efficiently represents \
an array of booleans. Bitarrays are sequence types and behave very much like \
usual lists. Eight bits are represented by one byte in a contiguous block of \
memory. The user can select between two representations: little-endian and \
big-endian. All of the functionality is implemented in C. Methods for \
accessing the machine representation are provided. This can be useful when bit \
level access to binary files is required, such as portable bitmap image files \
(.pbm). Also, when dealing with compressed data which uses variable bit length \
encoding, you may find this module useful."
HOMEPAGE = "https://github.com/ilanschnell/bitarray"
SECTION = "devel/python"

LICENSE = "PSF"
LIC_FILES_CHKSUM = "file://PKG-INFO;md5=dc301a25ebe210dcc53b0a2d5a038eae"

SRC_URI[md5sum] = "a46bf869f6adf34f5b0dc82b469793b7"
SRC_URI[sha256sum] = "2ed675f460bb0d3d66fd8042a6f1f0d36cf213e52e72a745283ddb245da7b9cf"

inherit pypi setuptools

BBCLASSEXTEND = "native nativesdk"
