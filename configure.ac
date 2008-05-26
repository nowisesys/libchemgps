# -*- sh -*-  -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libchemgps], 0.6.2, anders.lovgren@bmc.uu.se)
AM_INIT_AUTOMAKE([libchemgps], 0.6.2)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
CGPS_CHECK_COMPILER
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
CGPS_CHECK_SIMCAQ

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset strerror])

AC_CONFIG_FILES([Makefile
                 docs/Makefile
		 src/libchemgps.pc
		 src/Makefile])
AC_OUTPUT
