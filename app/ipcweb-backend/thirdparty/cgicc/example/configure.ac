dnl -*-m4-*-
dnl $Id: configure.ac,v 1.4 2008/01/19 17:27:37 sebdiaz Exp $

dnl Process this file with autoconf to produce a configure script.
AC_INIT(cgicc-example, 0.2, bug-cgicc@gnu.org)
AC_CONFIG_SRCDIR(example.cpp)
AC_REVISION($Revision: 1.4 $)
AC_PREFIX_DEFAULT(/usr)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h:config.hin])


AC_LANG_CPLUSPLUS

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_CHECK_PROG(DOXYGEN, doxygen, doxygen, /bin/echo)

dnl Checks for header files
AC_CHECK_HEADERS([sys/time.h sys/utsname.h])

dnl setup for networking functions
AC_CHECK_LIB(nsl, gethostbyaddr, LIB_NET="-lnsl")
AC_SUBST(LIB_NET)

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

dnl Checks for libraries.

dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl Info on installed cgicc
AC_ARG_WITH(cgicc,
  [  --with-cgicc[=DIR]      use cgicc installation in DIR],
  [if ! test -d "${withval}"; then
    AC_MSG_ERROR(bad value '${withval}' for --with-cgicc: not a directory)
   fi
   cgicc_prefix=${withval}])

dnl Perform some sanity checks on the cgicc installation
CGICC_CHECK_INSTALLATION($cgicc_prefix)

dnl Output files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
