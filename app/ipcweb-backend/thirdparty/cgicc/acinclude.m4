dnl $Id: acinclude.m4,v 1.9 2004/06/28 00:25:28 sbooth Exp $

dnl @TOP@

dnl Include support macros from autoconf macro archive
dnl http://www.gnu.org/software/ac-archive/
builtin(include, support/ac_cxx_namespaces.m4)dnl
builtin(include, support/ac_cxx_have_stl.m4)dnl

dnl CGICC_CHECK_LINK_STDCPP
AC_DEFUN([CGICC_CHECK_LINK_STDCPP], [
	AC_REQUIRE([AC_PROG_CXX])dnl
	AC_CACHE_CHECK(	
		[whether to link against libstdc++],
		[cgicc_cv_link_libstdcpp],
		[	AC_LANG_SAVE
			AC_LANG_CPLUSPLUS
			AC_TRY_LINK([#include <iostream>],
			std::cout << "foo" << std::endl;,
			cgicc_cv_link_libstdcpp=no,
			cgicc_cv_link_libstdcpp=yes)
			AC_LANG_RESTORE
		])
	if (test "$cgicc_cv_link_libstdcpp" = yes); then 
		LIBS="$LIBS -lstdc++"
	fi
])

dnl CGICC_CHECK_ACC
AC_DEFUN([CGICC_CHECK_ACC], [
	AC_CACHE_CHECK(
		[whether the C++ compiler ($CXX) is aCC],
		[cgicc_cv_acc],
		[	if echo $CXX | grep 'HP ANSI C++'> /dev/null 2>&1; then
				cgicc_cv_acc=yes
			else
				cgicc_cv_acc=no
			fi
			
	])
])




