m4_define([package_name], [ibus-unikey])
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [0])
m4_define([ibus_minor_version], [3])
m4_define([ibus_micro_version], [0])

m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version)

AC_INIT([package_name], [ibus_version], [http://code.google.com/p/ibus-unikey], [package_name])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# check ibus
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0
])

#check python
AM_PATH_PYTHON([2.5])

# define GETTEXT_* variables
GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])


AC_ARG_WITH(DEBUG, 
	AC_HELP_STRING([--with-DEBUG],[define DEBUG (Default: no)]), 
	[defDEBUG=$withval], [defDEBUG=no])

if test "$defDEBUG" = yes; then
	DEFINE=-DDEBUG
fi
AC_SUBST(DEFINE)

# OUTPUT files
AC_CONFIG_FILES( po/Makefile.in
    Makefile
    ukengine/Makefile
    src/unikey.xml.in
    src/Makefile
    icons/Makefile
    m4/Makefile
)

AC_OUTPUT
