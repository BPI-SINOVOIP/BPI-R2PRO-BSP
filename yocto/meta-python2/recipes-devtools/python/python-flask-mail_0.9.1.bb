SUMMARY = "Flask extension for sending email"
DESCRIPTION = "One of the most basic functions in a web application is the \
ability to send emails to your users.\
\
The Flask-Mail extension provides a simple interface to set up SMTP with your \
Flask application and to send messages from your views and scripts."
HOMEPAGE = "https://github.com/mattupstate/flask-mail"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=5b16dfa6d3f275ace5985bb92949f770"

SRC_URI[md5sum] = "04b35a42a44ec7aa724ec8ce55e2e08e"
SRC_URI[sha256sum] = "22e5eb9a940bf407bcf30410ecc3708f3c56cc44b29c34e1726fe85006935f41"

PYPI_PACKAGE = "Flask-Mail"

inherit pypi setuptools

RDEPENDS_${PN} = "${PYTHON_PN}-flask"
