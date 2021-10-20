dnl
dnl $Id: cgicc.m4,v 1.10 2009/01/18 13:58:25 sebdiaz Exp $
dnl
dnl Copyright (C) 2000 - 2004 Stephen F. Booth
dnl                      2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
dnl Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 3 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl 
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
dnl


dnl
dnl CGICC_CHECK_CONFIG([PREFIX])
dnl
dnl Check for the configuration script 'cgicc-config' in PATH.  Define
dnl  the macros cgicc_libdir and cgicc_includedir to the values
dnl  obtained from 'cgicc-config'.
dnl
AC_DEFUN([CGICC_CHECK_CONFIG], [
  AC_PATH_PROG(CGICC_CONFIG, cgicc-config, no, [$1/bin $PATH])
  if test "$CGICC_CONFIG" = no; then
    AC_MSG_ERROR(cgicc-config not found in $1/bin:$PATH)
  fi

  cgicc_libdir=`$CGICC_CONFIG --libdir`
  cgicc_includedir=`$CGICC_CONFIG --includedir`

  AC_SUBST(cgicc_libdir)
  AC_SUBST(cgicc_includedir)
])

dnl
dnl CGICC_CHECK_HEADERS([INCLUDEDIR])
dnl
dnl Perform a sanity check on the cgicc installation, making sure all
dnl  headers are present.
dnl
AC_DEFUN([CGICC_CHECK_HEADERS], [
  AC_REQUIRE([CGICC_CHECK_CONFIG])
  missing=
  AC_MSG_CHECKING(whether cgicc headers look good)
  cgicc_headers_found=yes

  cgicc_include_dir=ifelse([$1], ,$cgicc_includedir,$1)

  list="CgiDefs.h CgiEnvironment.h CgiInput.h CgiUtils.h Cgicc.h FormEntry.h FormFile.h HTMLAtomicElement.h HTMLAttribute.h HTMLAttributeList.h HTMLBooleanElement.h HTMLClasses.h HTMLDoctype.h HTMLElement.h HTMLElementList.h HTTPContentHeader.h HTTPCookie.h HTTPHTMLHeader.h HTTPHeader.h HTTPPlainHeader.h HTTPRedirectHeader.h HTTPResponseHeader.h HTTPStatusHeader.h MStreamable.h"
  for file in $list; do
    if ! test -e "$cgicc_include_dir/cgicc/$file"; then
      cgicc_headers_found=no
      missing="$missing $file"
    fi
  done

  AC_MSG_RESULT($cgicc_headers_found)  

  if test "$cgicc_headers_found" = no; then
    AC_MSG_ERROR(missing cgicc header(s): $missing)
  fi
])

dnl
dnl CGICC_CHECK_VERSION([MINIMUM-VERSION])
dnl
dnl Verify that the installed version of cgicc is at least
dnl  MINIMUM-VERSION, if specified, otherwise 3.2.1
dnl
AC_DEFUN([CGICC_CHECK_VERSION], [
  AC_REQUIRE([CGICC_CHECK_CONFIG])
  cgicc_min_version=ifelse([$1], ,3.2.1,$1)
  AC_MSG_CHECKING(for cgicc version >= $cgicc_min_version)
  cgicc_version=
  cgicc_major_version=
  cgicc_minor_version=

  cgicc_req_major_version=`echo $cgicc_min_version | cut -d '.' -f 1`
  cgicc_req_minor_version=`echo $cgicc_min_version | cut -d '.' -f 2`

  cgicc_version=`$CGICC_CONFIG --version`

  cgicc_major_version=`echo $cgicc_version | cut -d '.' -f 1`
  cgicc_minor_version=`echo $cgicc_version | cut -d '.' -f 2`

  AC_MSG_RESULT($cgicc_version)

  if (test $cgicc_major_version -lt $cgicc_req_major_version || test $cgicc_minor_version -lt $cgicc_req_minor_version); then
    AC_MSG_ERROR(cgicc version >= $cgicc_min_version required)
  fi
])

dnl
dnl CGICC_CHECK_INSTALLATION([PREFIX[,INCLUDEDIR[,MINIMUM-VERSION]]])
dnl
dnl Check the cgicc installation for:
dnl  - Presence of the 'cgicc-config' script [in PREFIX]
dnl  - Presence of all headers [in INCLUDEDIR]
dnl  - Installed version >= 3.2.1 [or MINIMUM-VERSION if specified]
AC_DEFUN([CGICC_CHECK_INSTALLATION][
  CGICC_CHECK_CONFIG($1)
  CGICC_CHECK_HEADERS($2)
  CGICC_CHECK_VERSION($3)
])
