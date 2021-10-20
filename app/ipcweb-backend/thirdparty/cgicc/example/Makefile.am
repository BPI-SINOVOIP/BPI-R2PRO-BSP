## $Id: Makefile.am,v 1.5 2001/09/02 19:58:34 sbooth Exp $

CLEANFILES = *~

EXTRA_DIST = cgicc.m4

cgicc_libdir=@cgicc_libdir@
cgicc_includedir=@cgicc_includedir@

bin_PROGRAMS = example.cgi

INCLUDES = -I$(cgicc_includedir)

example_cgi_SOURCES = example.cpp
example_cgi_LDADD = -L$(cgicc_libdir) -lcgicc
