ui_DATA = \
    setup-main.ui \
    setup-macro.ui

uidir = $(pkgdatadir)/ui

INCLUDES = \
    -I$(top_srcdir)/ukengine \
    -I$(top_srcdir)/src \
    -DPKGDATADIR=\"$(pkgdatadir)\" \
    -DUI_DATA_DIR=\"$(uidir)\" \
    -DLOCALEDIR=\"$(localedir)\" \
    -Wall

EXTRA_DIST = $(ui_DATA)

libexec_PROGRAMS = ibus-setup-unikey

ibus_setup_unikey_SOURCES = \
    setup.cpp \
    dlg_main_setup.cpp \
    dlg_macro_table.cpp

noinst_HEADERS = \
    dlg_main_setup.h \
    dlg_macro_table.h

ibus_setup_unikey_CXXFLAGS = \
    @IBUS_CFLAGS@ \
    @GTK_CFLAGS@

ibus_setup_unikey_LDFLAGS = \
    @IBUS_LIBS@ \
    @GTK_LIBS@ \
    $(top_builddir)/ukengine/libunikey-ibus.la \
    $(top_builddir)/src/ibus_engine_unikey-utils.o

