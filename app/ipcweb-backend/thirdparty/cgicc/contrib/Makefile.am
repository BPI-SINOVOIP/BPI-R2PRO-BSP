## $Id: Makefile.am,v 1.5 2014/04/23 20:55:11 sebdiaz Exp $

CLEANFILES = *~

EXTRA_DIST = FCgiIO.h
EXTRA_PROGRAMS = fcgi-test.cgi

if FCGI_DEMOS
FCGI_TEST = fcgi-test.cgi
else
FCGI_TEST = 
endif

noinst_PROGRAMS = nph-hello.cgi nph-login.cgi $(FCGI_TEST)

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)

nph_hello_cgi_SOURCES = nph-hello.cpp
nph_hello_cgi_LDADD = ../cgicc/libcgicc.la

nph_login_cgi_SOURCES = nph-login.cpp
nph_login_cgi_LDADD = ../cgicc/libcgicc.la

fcgi_test_cgi_SOURCES = fcgi-test.cpp FCgiIO.cpp
fcgi_test_cgi_LDADD = ../cgicc/libcgicc.la @FCGI_LIBS@
