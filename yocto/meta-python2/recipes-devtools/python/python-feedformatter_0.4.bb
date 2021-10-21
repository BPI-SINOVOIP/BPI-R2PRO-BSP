SUMMARY = "A Python library for generating news feeds in RSS and Atom formats"
DESCRIPTION = "feedformatter is intended to be in many ways an "opposite" to \
the well-known and well-loved Python module feedparser. Whereas feedparser \
tries to turn any news feed in any format into a friendly data structure of \
lists and dictionaries, feedformatter tries to turn a friendly data structure \
of lists and dictionaries into a well-formatted news feed of whatever kind you \
like."
HOMEPAGE = "http://code.google.com/p/feedformatter/"
SECTION = "devel/python"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://COPYING;md5=258e3f39e2383fbd011035d04311008d"

SRC_URI = "git://github.com/marianoguerra/feedformatter.git"
SRCREV = "7391193c83e10420b5a2d8ef846d23fc368c6d85"

S = "${WORKDIR}/git"

inherit setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-datetime \
    ${PYTHON_PN}-io \
    ${PYTHON_PN}-xml \
    "
