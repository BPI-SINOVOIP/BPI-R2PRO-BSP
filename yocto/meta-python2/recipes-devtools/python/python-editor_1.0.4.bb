SUMMARY = "Programmatically open and editor, capture the result"
DESCRIPTION = "python-editor is a library that provides the editor module for \
programmatically interfacing with your system’s $EDITOR."
HOMEPAGE = "https://github.com/fmoo/python-editor"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d2794c0df5b907fdace235a619d80314"

SRC_URI[md5sum] = "0e52648a4a6e7c89e3be44e9456530b4"
SRC_URI[sha256sum] = "51fda6bcc5ddbbb7063b2af7509e43bd84bfc32a4ff71349ec7847713882327b"

PYPI_PACKAGE = "python-editor"

inherit pypi setuptools
