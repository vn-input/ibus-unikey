m4_define([package_name], [ibus-unikey])
m4_define([ibus_unikey_version], [0.6.1])

AC_INIT([package_name], [ibus_unikey_version], [http://code.google.com/p/ibus-unikey], [package_name])
AM_INIT_AUTOMAKE([1.10])
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

GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Gettext package])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
IT_PROG_INTLTOOL


AC_ARG_WITH(gtk-version,
    AS_HELP_STRING([--with-gtk-version[=VERSION]],
                   [Choose GTK version to build with. VERSION=2 or 3 [default=2]]),
    gtk_version=$with_gtk_version
)
AS_IF([test "x$gtk_version" == "x3" || test "x$gtk_version" == "xgtk-3" || test "x$gtk_version" == "xgtk3"],
    gtk_version="gtk+-3.0",
    gtk_version="gtk+-2.0 >= 2.16"
)

PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= 1.2.99
])

PKG_CHECK_MODULES(X11, [
    x11
])
AC_SUBST(X11)

PKG_CHECK_MODULES(GTK,
    $gtk_version
)
AC_SUBST(GTK)

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

AC_MSG_RESULT([
Build options:
    Version         $VERSION
    Install prefix  $prefix
    GTK Version     $gtk_version
])

