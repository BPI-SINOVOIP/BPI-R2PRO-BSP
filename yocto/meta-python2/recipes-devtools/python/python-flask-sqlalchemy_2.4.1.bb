SUMMARY = "Adds SQLAlchemy support to your Flask application."
DESCRIPTION = "Flask-SQLAlchemy is an extension for Flask that adds support \
for SQLAlchemy to your application. It aims to simplify using SQLAlchemy with \
Flask by providing useful defaults and extra helpers that make it easier to \
accomplish common tasks."
HOMEPAGE = "https://github.com/pallets/flask-sqlalchemy"
BUGTRACKER = "https://github.com/pallets/flask-sqlalchemy/issues"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.rst;md5=ffeffa59c90c9c4a033c7574f8f3fb75"

SRC_URI[md5sum] = "1f5781cf3e1a2b1aabda47a5b20d2073"
SRC_URI[sha256sum] = "6974785d913666587949f7c2946f7001e4fa2cb2d19f4e69ead02e4b8f50b33d"

PYPI_PACKAGE = "Flask-SQLAlchemy"

inherit pypi setuptools

RDEPENDS_${PN} = "${PYTHON_PN}-sqlalchemy ${PYTHON_PN}-flask"
