m4_define([package_name], [ibus-unikey])
m4_define([ibus_unikey_version], [0.6.1])

AC_INIT([package_name], [ibus_unikey_version], [https://github.com/vn-input/ibus-unikey/issues], [package_name])
AM_INIT_AUTOMAKE([subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Gettext package])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
IT_PROG_INTLTOOL

GLIB_GSETTINGS

PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= 1.5
])

PKG_CHECK_MODULES(GTK,
    gtk+-3.0
)
AC_SUBST(GTK)

# OUTPUT files
AC_CONFIG_FILES( po/Makefile.in
    Makefile
    ukengine/Makefile
    src/unikey.xml.in
    src/Makefile
    setup/Makefile
    data/Makefile
    m4/Makefile
    ibus-unikey.spec
)

AC_OUTPUT

AC_MSG_RESULT([
Build options:
    Version         $VERSION
    Install prefix  $prefix
])

