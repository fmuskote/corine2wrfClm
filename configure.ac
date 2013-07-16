#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([corine2wrfClm], [0.1], [s.schwampf@gmx.de])
AC_PREREQ([2.59])
AC_LANG([C++])
AM_INIT_AUTOMAKE([1.10 no-define])
#AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

# CHECK FOR GDAL #
##################

AC_ARG_WITH([gdal-config],
            [AS_HELP_STRING([--with-gdal-config=FILE], [specify an alternative gdal-config file])],
            [GDAL_CONFIG="$withval"], [GDAL_CONFIG=""])
if test -z $GDAL_CONFIG;
then
    AC_PATH_PROG([GDAL_CONFIG], [gdal-config])
    if test -z $GDAL_CONFIG;
    then
        AC_MSG_ERROR([could not find gdal-config from libgdal within the current path. You may need to try re-running configure with a --with-gdal-config parameter.])
    fi
else
    if test -f $GDAL_CONFIG;
    then
        AC_MSG_RESULT([Using user-specified gdal-config file: $GDAL_CONFIG])
    else
        AC_MSG_ERROR([the user-specified gdal-config file $GDAL_CONFIG does not exist])
    fi
fi

GDAL_CFLAGS="`$GDAL_CONFIG --cflags`"
GDAL_LIBS="`$GDAL_CONFIG --libs`"

# CHECK FOR NETCDF #
####################

AC_ARG_WITH([nc-config],
            [AS_HELP_STRING([--with-nc-config=FILE], [specify an alternative nc-config file])],
            [NETCDF_CONFIG="$withval"], [NETCDF_CONFIG=""])
if test -z $NETCDF_CONFIG;
then
    AC_PATH_PROG([NETCDF_CONFIG], [nc-config])
    if test -z $NETCDF_CONFIG;
    then
        AC_MSG_ERROR([could not find nc-config from libnetcdf within the current path. You may need to try re-running configure with a --with-nc-config parameter.])
    fi
else
    if test -f $NETCDF_CONFIG;
    then
        AC_MSG_RESULT([Using user-specified nc-config file: $NETCDF_CONFIG])
    else
        AC_MSG_ERROR([the user-specified nc-config file $NETCDF_CONFIG does not exist])
    fi
fi

AC_CHECK_HEADERS([boost/scoped_array.hpp], [], [AC_MSG_ERROR(You need the Boost libraries.)])
AC_CHECK_HEADERS([boost/shared_ptr.hpp], [], [AC_MSG_ERROR(You need the Boost libraries.)])
AC_CHECK_HEADERS([boost/multi_array.hpp], [], [AC_MSG_ERROR(You need the Boost libraries.)])
AC_CHECK_HEADERS([boost/array.hpp], [], [AC_MSG_ERROR(You need the Boost libraries.)])

NETCDF_CFLAGS="`$NETCDF_CONFIG --cflags`"
NETCDF_LIBS="`$NETCDF_CONFIG --libs`_c++"

# CHECK FOR OPENMP #
####################

AC_OPENMP
OPENMP_CFLAGS="$OPENMP_CXXFLAGS"

CFLAGS="$CFLAGS -Wall"

AC_SUBST([AM_CXXFLAGS],["$GDAL_CFLAGS $NETCDF_CFLAGS $OPENMP_CFLAGS $CFLAGS"])
AC_SUBST([LIBS], ["$LIBS $GDAL_LIBS $NETCDF_LIBS"])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
