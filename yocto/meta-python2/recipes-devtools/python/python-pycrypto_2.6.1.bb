SUMMARY = "Cryptographic modules for Python."
DESCRIPTION = "This is a collection of both secure hash functions (such as \
SHA256 and RIPEMD160), and various encryption algorithms (AES, DES, RSA, \
ElGamal, etc.). The package is structured to make adding new modules easy. \
This section is essentially complete, and the software interface will almost \
certainly not change in an incompatible way in the future; all that remains to \
be done is to fix any bugs that show up."
HOMEPAGE = "http://www.pycrypto.org/"
LICENSE = "PSFv2"
LIC_FILES_CHKSUM = "file://COPYRIGHT;md5=35f354d199e8cb7667b059a23578e63d"

FILESEXTRAPATHS_prepend := "${THISDIR}/python-pycrypto:"

DEPENDS += " gmp"

export HOST_SYS

inherit autotools pypi autotools-brokensep distutils

SRC_URI += "file://cross-compiling.patch \
            file://CVE-2013-7459.patch \
           "

SRC_URI[md5sum] = "55a61a054aa66812daf5161a0d5d7eda"
SRC_URI[sha256sum] = "f2ce1e989b272cfcb677616763e0a2e7ec659effa67a88aa92b3a65528f60a3c"

do_compile[noexec] = "1"

# We explicitly call distutils_do_install, since we want it to run, but
# *don't* want the autotools install to run, since this package doesn't
# provide a "make install" target.
do_install() {
       distutils_do_install
}

BBCLASSEXTEND = "native nativesdk"
