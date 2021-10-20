dnl -*-m4-*-
dnl $Id: configure.ac,v 1.19 2017/06/25 16:19:22 sebdiaz Exp $

dnl Process this file with autoconf to produce a configure script.
AC_INIT(cgicc, 3.2.19, sebastien.diaz@gmail.com)
AC_CONFIG_SRCDIR(cgicc/Cgicc.cpp)
AC_REVISION([$Revision: 1.19 $])
AC_CONFIG_AUX_DIR(support)
AC_PREFIX_DEFAULT(/usr)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(cgicc/config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_LANG_CPLUSPLUS

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_CHECK_PROG(DOXYGEN, doxygen, doxygen, /bin/echo)



dnl Determine host system type


dnl Checks for libraries.

dnl setup for networking functions
AC_CHECK_LIB(nsl, gethostbyaddr, LIB_NET="-lnsl")
AC_SUBST(LIB_NET)

dnl check whether to link against libstdc++
dnl CGICC_CHECK_LINK_STDCPP

dnl Checks for header files
AC_CHECK_HEADERS([sys/time.h sys/utsname.h])

dnl Determine host system type
AC_DEFINE_UNQUOTED(HOST, "$host", [The host system cgicc was configured for])

dnl Check whether we're using aCC
CGICC_CHECK_ACC
if test "$cgicc_cv_acc" = yes; then
  CXXFLAGS="-AA $CXXFLAGS"
  DEMO_LIBS="-lstd_v2 -lCsup_v2"
fi
AC_SUBST(DEMO_LIBS)

dnl verify namespace support
AC_CXX_NAMESPACES

dnl verify STL support
AC_CXX_HAVE_STL

dnl Enable maximum compiler warnings for g++
AC_MSG_CHECKING(maximum C++ compiler warning level)
if test "$GXX" = yes; then
  CXXFLAGS_WARN="-Wall -W -pedantic"
  CXXFLAGS="$CXXFLAGS_WARN $CXXFLAGS"
fi
AC_MSG_RESULT($CXXFLAGS_WARN)
unset CXXFLAGS_WARN

dnl Checks for library functions.
AC_CHECK_FUNCS(gettimeofday uname)

dnl enable/disable the demos, if specified
AC_ARG_ENABLE(demos, 
  [  --enable-demos          build demos [[default=yes]]],
  [case "${enableval}" in
    yes) cgicc_demos=yes ;;
    no)  cgicc_demos=no ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-demos) ;;
  esac], cgicc_demos=yes)
AC_MSG_CHECKING(whether to build the cgicc demos)
AC_MSG_RESULT($cgicc_demos)
AM_CONDITIONAL(DEMOS, test "$cgicc_demos" = yes)

if test "$cgicc_demos" = yes; then
  AC_CONFIG_FILES(demo/Makefile)
  AC_CHECK_FUNCS([gethostbyaddr])
  AC_CHECK_FUNCS([gethostbyname]) 
  AC_CHECK_FUNCS([inet_ntoa])
  AC_CHECK_HEADERS([arpa/inet.h])
  AC_CHECK_HEADERS([netdb.h])
  AC_CHECK_HEADERS([netinet/in.h])
  AC_CHECK_HEADERS([sys/socket.h])
  AC_CHECK_HEADER_STDBOOL
fi
AC_CHECK_HEADERS([fcntl.h])
AC_C_INLINE
AC_PROG_RANLIB
AC_TYPE_SIZE_T


dnl enable/disable the FastCgi demos, if specified
AC_ARG_ENABLE(fcgi-demos, 
  [  --enable-fcgi-demos     build FastCgi demos [[default=no]]],
  [case "${enableval}" in
    yes) cgicc_fcgi_demos=yes ;;
    no)  cgicc_fcgi_demos=no ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-fcgi-demos) ;;
  esac], cgicc_fcgi_demos=no)
AC_MSG_CHECKING(whether to build the cgicc fcgi demos)
AC_MSG_RESULT($cgicc_fcgi_demos)
AM_CONDITIONAL(FCGI_DEMOS, test "$cgicc_fcgi_demos" = yes)

if test "$cgicc_fcgi_demos" = yes; then
  AC_CHECK_LIB(fcgi, FCGX_Accept, FCGI_LIBS="-lfcgi++ -lfcgi",
	AC_MSG_ERROR([FastCGI libraries not found]))
fi
AC_SUBST(FCGI_LIBS)

dnl Output contrib/Makefile if either of the demos are enabled
if test "$cgicc_demos" = yes || test "$cgicc_fcgi_demos" = yes; then
  AC_CONFIG_FILES(contrib/Makefile)
fi

dnl Always output certain files
AC_CONFIG_FILES([Makefile cgicc/Makefile cgicc/CgiDefs.h 
  support/Makefile support/cgicc-config doc/Makefile doc/Doxyfile])

AC_OUTPUT( cgicc.pc )
