#
#    Ophidia Primitives
#    Copyright (C) 2012-2022 CMCC Foundation
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.60])
AC_INIT([ophidia-primitives], [1.7.1], [ophidia-info@cmcc.it])
# AC_CONFIG_SRCDIR([src/mpiTest.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC_C89
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_OPENMP

# Disable static library creation
LT_INIT([disable-static])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_LIB([m],[cos])

# Automake conditionals
# AM_CONDITIONAL([HAVE_SQLITE3], [test -n "$SQLITE3_CFLAGS"])

AX_LIB_MYSQL([5.6])
if test -n $HAVE_MYSQL; then
  AC_MSG_NOTICE([MySQL libraries available])
else
	AC_MSG_ERROR([MySQL libraries not found! Aborting configure!])
fi

GSL_SUPPORT=
AC_ARG_ENABLE(gsl_lib,
	   [  --disable-gsl-lib
          Disable GSL. (Enabled by default)],
	   [ gsl_lib="no"
          AC_MSG_WARN("GSL support disabled")],
     [ gsl_lib="yes"
          AC_MSG_NOTICE("GSL support enabled")]
	   )
AM_CONDITIONAL([GSL_SUPPORT], [test "x$gsl_lib" = "xyes"])

if test "x${gsl_lib}" = "xyes"; then
	PKG_CHECK_MODULES(GSL,[gsl >= 1.8])
	AC_ARG_WITH(gsl_header_path,
		   [  --with-gsl-header-path=PATH        Location of GSL includes directory],
		   [
		    gsl_header_path="-I$withval"
		   ],
		 [
		gsl_header_path="$GSL_CFLAGS"
		 ])
	AC_ARG_WITH(gsl_lib_path,
		   [  --with-gsl-lib-path=PATH       Location of GSL lib],
		   [
		    gsl_lib_path="-L$withval"
		   ],
		 [
	   gsl_lib_path="$GSL_LIBS"
		 ])
	GSL_INCLUDE="$gsl_header_path"
	AC_SUBST(GSL_INCLUDE)
	GSL_LIB="$gsl_lib_path"
	AC_SUBST(GSL_LIB)
fi

matheval_support=no
MATHEVAL_CONFIG=
AC_ARG_WITH(matheval_path,
           [  --with-matheval-path=PATH  Set the MathEval library root dir],
           [
                matheval_path="$withval"
                                matheval_support=yes
                                MATHEVAL_CONFIG="${withval}/pkgconfig/libmatheval.pc"
           ],
           [
                        MATHEVAL_CONFIG="/usr/local/lib/pkgconfig/libmatheval.pc"
                        echo -n "checking for $MATHEVAL_CONFIG... "
                        if test -e "$MATHEVAL_CONFIG" ; then
                                matheval_support=yes
                        fi
                        echo "$matheval_support"
           ]
           )
if test "x${matheval_support}" = "xyes"; then
	MATHEVAL_CFLAGS="`pkg-config $MATHEVAL_CONFIG --cflags`"
	MATHEVAL_LIBS="`pkg-config $MATHEVAL_CONFIG --libs`"
        if test "x${MATHEVAL_LIBS}" != "x"; then
                AC_SUBST(MATHEVAL_CFLAGS)
                AC_SUBST(MATHEVAL_LIBS)
                AC_MSG_NOTICE("Matheval support enabled")
        else
                AC_MSG_WARN("Matheval support disabled")
                matheval_support=no
        fi
else
	AC_MSG_WARN("Matheval support disabled")
fi
AM_CONDITIONAL([MATHEVAL_SUPPORT], [test "x$matheval_support" = "xyes"])

OPT=""
AC_ARG_ENABLE(debug,
	   [  --enable-debug	Enable debug. (Disabled by default)],
	   [ debug="yes"],
           [ debug="no"]
	   )
if test "x$debug" = "xyes"; then
	AC_MSG_WARN("Debug activated")
	OPT="-g3 -O0"
else
	OPT="-O3"
fi

AC_SUBST(OPT)

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
