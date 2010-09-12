m4_define([package_name], [ibus-unikey])
m4_define([ibus_unikey_released], [0])
m4_define([ibus_unikey_major_version], [0])
m4_define([ibus_unikey_minor_version], [5])
m4_define([ibus_unikey_micro_version], [1])

m4_define([ibus_unikey_version],
    ibus_unikey_major_version.ibus_unikey_minor_version.ibus_unikey_micro_version)

AC_INIT([package_name], [ibus_unikey_version], [http://code.google.com/p/ibus-unikey], [package_name])
AM_INIT_AUTOMAKE([1.10])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
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

AM_PATH_PYTHON([2.5])

PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= 1.2
])

GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

PKG_CHECK_MODULES(X11, [
    x11
])
AC_SUBST(X11)

PKG_CHECK_MODULES(GTK, [
    gtk+-2.0 >= 2.12
])
AC_SUBST(GTK)

PKG_CHECK_MODULES(GCONF, [
    gconf-2.0
])
AC_SUBST(GCONF)

# OUTPUT files
AC_CONFIG_FILES( po/Makefile.in
    Makefile
    ukengine/Makefile
    src/unikey.xml.in
    src/Makefile
    setup/Makefile
    icons/Makefile
    m4/Makefile
    ibus-unikey.spec
)

AC_OUTPUT

