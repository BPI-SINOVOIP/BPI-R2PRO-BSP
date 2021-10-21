SUMMARY  = "A library for property based testing"
DESCRIPTION = "Hypothesis is an advanced testing library for Python. It lets \
you write tests which are parametrized by a source of examples, and then \
generates simple and comprehensible examples that make your tests fail. This \
lets you find more bugs in your code with less work."
HOMEPAGE = "https://github.com/HypothesisWorks/hypothesis/tree/master/hypothesis-python"
BUGTRACKER = "https://github.com/HypothesisWorks/hypothesis/issues"
SECTION = "devel/python"

LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=4ee62c16ebd0f4f99d906f36b7de8c3c"

SRC_URI[md5sum] = "2cf187bc0b3a61d08b13743985a71646"
SRC_URI[sha256sum] = "c50157747f80930c0e9367a2ef10599204cc098007e18ba8daeac0620ee674cb"

DEPENDS = "${PYTHON_PN}-attrs"

inherit pypi setuptools

RDEPENDS_${PN} += "\
    ${PYTHON_PN}-enum34 \
    "
