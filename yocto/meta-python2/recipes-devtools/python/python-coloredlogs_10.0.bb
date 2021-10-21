SUMMARY = "Colored terminal output for Python's logging module"
DESCRIPTION = "The coloredlogs package enables colored terminal output for \
Python’s logging module. The ColoredFormatter class inherits from \
logging.Formatter and uses ANSI escape sequences to render your logging \
messages in color. It uses only standard colors so it should work on any UNIX \
terminal. It’s currently tested on Python 2.6, 2.7, 3.4, 3.5, 3.6 and PyPy. On \
Windows coloredlogs automatically pulls in Colorama as a dependency and \
enables ANSI escape sequence translation using Colorama."
HOMEPAGE = "https://coloredlogs.readthedocs.io"
SECTION = "devel/python"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=690da298a43805797a4fa7bbe180b3c6"

SRC_URI[md5sum] = "0a186966a1955fff8cf9489373e691d9"
SRC_URI[sha256sum] = "b869a2dda3fa88154b9dd850e27828d8755bfab5a838a1c97fbc850c6e377c36"

inherit pypi setuptools

do_compile_prepend() {
    sed -ie "s/find_pth_directory(),/'',/g" ${S}/setup.py
}

do_install_append() {
    rm -rf ${D}${datadir}
}

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-humanfriendly \
"

BBCLASSEXTEND = "native"
