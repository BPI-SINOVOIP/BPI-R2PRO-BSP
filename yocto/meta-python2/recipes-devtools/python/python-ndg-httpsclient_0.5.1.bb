
SUMMARY = "Provides enhanced HTTPS support for httplib and urllib2 using PyOpenSSL"
DESCRIPTION = "A HTTPS client implementation for \
\
        httplib (Python 2), http.client (Python 3) and \
        urllib2 (Python 2) and urllib (Python 3) \
\
… based on PyOpenSSL. PyOpenSSL provides a more fully featured SSL \
implementation over the default provided with Python and importantly enables \
full verification of the SSL peer using pyasn1."
HOMEPAGE = "https://github.com/cedadev/ndg_httpsclient/"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://MANIFEST.in;md5=ce22c0cd986d2de3f7073cd6b5523ae0"

SRC_URI[md5sum] = "b0fc8ea38f87d2c1ab1ed79a95c078f9"
SRC_URI[sha256sum] = "d72faed0376ab039736c2ba12e30695e2788c4aa569c9c3e3d72131de2592210"

PYPI_PACKAGE = "ndg_httpsclient"

DEPENDS += " \
    ${PYTHON_PN}-pyopenssl \
    ${PYTHON_PN}-pyasn1 \
"

inherit pypi setuptools update-alternatives

RDEPENDS_${PN} += " \
    ${PYTHON_PN}-datetime \
    ${PYTHON_PN}-logging \
    ${PYTHON_PN}-pyopenssl \
    ${PYTHON_PN}-pyasn1 \
"

UPSTREAM_CHECK_REGEX = ""

ALTERNATIVE_${PN} = "ndg_httpclient"
ALTERNATIVE_LINK_NAME[ndg_httpclient] = "${bindir}/ndg_httpclient"
ALTERNATIVE_PRIORITY = "20"

BBCLASSEXTEND = "native nativesdk"
