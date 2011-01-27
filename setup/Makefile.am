INCLUDES = \
    -I$(top_srcdir)/ukengine \
    -I$(top_srcdir)/src \
    -DPKGDATADIR=\"$(pkgdatadir)\" \
    -DGLADE_DATA_DIR=\"$(pkgdatadir)/setup\" \
    -DLOCALEDIR=\"$(localedir)\" \
    -Wall

glade_DATA = \
    setup-main.ui \
    setup-macro.ui

gladedir = $(pkgdatadir)/setup

EXTRA_DIST = $(glade_DATA)

libexec_PROGRAMS = ibus-setup-unikey

ibus_setup_unikey_SOURCES = \
    setup.cpp \
    dlg_main_setup.cpp \
    dlg_macro_table.cpp \
    config_utils.cpp

noinst_HEADERS = \
    dlg_main_setup.h \
    dlg_macro_table.h \
    config_utils.h


ibus_setup_unikey_CXXFLAGS = \
    @IBUS_CFLAGS@ \
    @GTK_CFLAGS@ \
    @GCONF_CFLAGS@

ibus_setup_unikey_LDFLAGS = \
    @IBUS_LIBS@ \
    @GTK_LIBS@ \
    @GCONF_LIBS@ \
    $(top_builddir)/ukengine/libunikey-ibus.la


