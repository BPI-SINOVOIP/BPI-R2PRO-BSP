SUMMARY = "Python HTTP for Humans."
DESCRIPTION = "Requests allows you to send HTTP/1.1 requests extremely easily. \
There’s no need to manually add query strings to your URLs, or to form-encode \
your POST data. Keep-alive and HTTP connection pooling are 100% automatic, \
thanks to urllib3."
HOMEPAGE = "http://python-requests.org"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=19b6be66ed463d93fa88c29f7860bcd7"

FILESEXTRAPATHS_prepend := "${THISDIR}/python-requests:"

SRC_URI[md5sum] = "abfdc28db1065bbd0bc32592ac9d27a6"
SRC_URI[sha256sum] = "b3f43d496c6daba4493e7c431722aeb7dbc6288f52a6e04e7b6023b0247817e6"

inherit pypi setuptools

RDEPENDS_${PN} += " \
    ${PYTHON_PN}-chardet \
    ${PYTHON_PN}-email \
    ${PYTHON_PN}-idna \
    ${PYTHON_PN}-json \
    ${PYTHON_PN}-ndg-httpsclient \
    ${PYTHON_PN}-netserver \
    ${PYTHON_PN}-pyasn1 \
    ${PYTHON_PN}-pyopenssl \
    ${PYTHON_PN}-pysocks \
    ${PYTHON_PN}-urllib3 \
    ${PYTHON_PN}-zlib \
"

CVE_PRODUCT = "requests"

BBCLASSEXTEND = "native nativesdk"

