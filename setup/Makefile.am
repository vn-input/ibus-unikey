ui_DATA = ibus-unikey.ui

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
    main.cpp \
	signal_handlers.cpp \
	signal_handlers.h \
    macro_utils.cpp \
    macro_utils.h

noinst_HEADERS = \
	signal_handlers.h \
    macro_utils.h

ibus_setup_unikey_CXXFLAGS = \
    @GTK_CFLAGS@

ibus_setup_unikey_LDFLAGS = \
    -rdynamic \
    @GTK_LIBS@ \
    $(top_builddir)/ukengine/libunikey-ibus.la \
    $(top_builddir)/src/config/ibus_engine_unikey-unikey_config.o

