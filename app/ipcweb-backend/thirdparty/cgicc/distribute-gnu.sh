#! /bin/sh

# distribute-gnu.sh - upload file for distribution to gnu ftp site and mirrors.
# Copyright (C) 2006 Free Software Foundation, Inc.
# Written by Claudio Fontana <claudio@gnu.org>, 2006.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# This software depends on the following programs to run correctly:
# /bin/sh
# basename
# gpg
# ftp
#
# This script also assumes that you already generated/registered your key
# for the ftp uploads (see link below).
#
# For more information about the procedure see the
# "Information for maintainers of GNU software"
#
# http://www.gnu.org/prep/maintain/

as_me=`basename ${0}`
PACKAGE_VERSION=distribute-gnu-0.0

if test $# -lt 1 ; then
   echo "$as_me: insufficient arguments. Try --help" >&2
   exit 2
fi

HOST=ftp-upload.gnu.org
TARGET=ftp
DIRECTORY=
PACKAGE=
COMMENT=

simulate=0
debug=0
processing_options=1

for ARG in "$@"
do
    if test $processing_options = 1 ; then
	case $ARG in
	    -h | --help)
		echo "Usage: $as_me [OPTIONS] DIRECTORY FILENAME"
		echo " "
		echo "OPTIONS:"
		echo "-h, --help                 this help"
		echo "-v, --version              show script version"
		echo " "
		echo "-h, --host=HOSTNAME        (def:ftp-upload.gnu.org)"
		echo "-t, --target=HOSTNAME      [ftp|alpha] (def:ftp)"
		echo "-c, --comment=COMMENT      (def:no comment)"
		echo "-s, --simulation           do not perform the upload"
		echo "-d, --debug                show more information"
		echo " "
		echo "DIRECTORY: the destination directory on the remote host"
		echo "FILENAME:  the file to distribute"
		echo " "
		exit 0
		;;
	    
	    -v | --version)
		echo "$PACKAGE_VERSION (C) 2006 Free Software Foundation Inc."
		echo "Written by Claudio Fontana, 2006."
		exit 0
		;;
	    
	    -h*)
		HOST="${ARG#-h}"
		continue
		;;

	    --host=*)
	        HOST="${ARG#--host=}"
		continue
		;;

	    -t*)
		TARGET="${ARG#-t}"
		continue
		;;

	    --target=*)
		TARGET="${ARG#--target=}"
		continue
		;;

	    -c*)
		COMMENT="${ARG#-c}"
		continue
		;;

	    --comment=*)
	        COMMENT="${ARG#--comment=}"
		continue
		;;

	    -s | --simulation)
		simulate=1
		continue
		;;
	    
	    -d | --debug)
		debug=1
		continue
		;;

	    -*)
		echo "$as_me: unknown option: $ARG" >&2
		exit 2
	esac
    fi
    
    processing_options=0

    if test "x$DIRECTORY" = "x" ; then
	DIRECTORY="${ARG}"
	continue
    fi
	
    if test "x$PACKAGE" = "x" ; then
	PACKAGE="${ARG}"
	continue;
    fi

    echo "$as_me: too many arguments." >&2
    exit 2
done

if test "x$DIRECTORY" = "x" ; then
    echo "$as_me: missing required DIRECTORY argument." >&2
    exit 2
fi

if test "x$PACKAGE" = "x" ; then
    echo "$as_me: missing required PACKAGE argument." >&2
    exit 2
fi

if test -f "$PACKAGE" ; then
    :
else
    echo "$as_me: ${PACKAGE} is not an existing regular file." >&2
    exit 2
fi

if test "$debug" = "1" ; then
    echo "HOST=${HOST}"
    echo "TARGET=${TARGET}"
    echo "DIRECTORY=${DIRECTORY}"
    echo "PACKAGE=${PACKAGE}"
    echo "COMMENT=${COMMENT}"
fi

if gpg -b --yes ${PACKAGE} ; then
    :
else
    echo "$as_me: failed to sign package using gpg." >&2
    exit 2
fi
echo "replace: true" > ${PACKAGE}.directive
echo "version: 1.2" > ${PACKAGE}.directive
echo "directory: ${DIRECTORY}" >> ${PACKAGE}.directive
echo "filename: ${PACKAGE}" >> ${PACKAGE}.directive

if test "x$COMMENT" != "x" ; then
    echo "comment: ${COMMENT}" >> ${PACKAGE}.directive
fi

if gpg --clearsign --yes ${PACKAGE}.directive ; then
    :
else
    echo "$as_me: failed to sign directive file using gpg." >&2
    exit 2
fi

if test "$simulate" != "0" ; then
    exit 0
fi

#upload results to ftp.
echo ${HOST}
ftp -n -p ${HOST}<<EOF
user
anonymous
cd /incoming/${TARGET}
binary
put ${PACKAGE}
put ${PACKAGE}.sig
put ${PACKAGE}.directive.asc
EOF
