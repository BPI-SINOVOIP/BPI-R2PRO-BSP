SUMMARY = "Screen-scraping library"
DESCRIPTION = "Beautiful Soup is a library that makes it easy to scrape \
information from web pages. It sits atop an HTML or XML parser, providing \
Pythonic idioms for iterating, searching, and modifying the parse tree."
HOMEPAGE = " https://www.crummy.com/software/BeautifulSoup/bs4"
SECTION = "devel/python"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.txt;md5=f2d38d8a40bf73fd4b3d16ca2e5882d1"

SRC_URI[md5sum] = "5dbdb56c009e4632bae7bed1b385804b"
SRC_URI[sha256sum] = "05fd825eb01c290877657a56df4c6e4c311b3965bda790c613a3d6fb01a5462a"

inherit pypi setuptools

RDEPENDS_${PN} = "\
    ${PYTHON_PN}-html5lib \
    ${PYTHON_PN}-lxml \
    ${PYTHON_PN}-soupsieve \
"

RDEPENDS_${PN}_append_class-target = "\
    ${PYTHON_PN}-html \
    ${PYTHON_PN}-logging \
"

BBCLASSEXTEND = "native nativesdk"
