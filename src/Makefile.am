INCLUDES = \
	-I$(top_srcdir)/ukengine \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
    -DLIBEXECDIR=\"$(libexecdir)\" \
    -Wall

libexec_PROGRAMS = ibus-engine-unikey

ibus_engine_unikey_SOURCES = \
	main.cpp \
	engine.cpp \
	engine.h \
	engine_private.h \
	utils.cpp \
	utils.h

noinst_HEADERS = \
    engine.h \
    engine_private.h \
    engine_const.h \
    utils.h

ibus_engine_unikey_CXXFLAGS = @IBUS_CFLAGS@ @X11_CFLAGS@

ibus_engine_unikey_LDFLAGS = \
	@IBUS_LIBS@ \
	@X11_LIBS@ \
	$(top_builddir)/ukengine/libunikey-ibus.la

component_DATA = unikey.xml

componentdir = @datadir@/ibus/component

EXTRA_DIST = unikey.xml.in

CLEANFILES = unikey.xml

unikey.xml: unikey.xml.in
	$(AM_V_GEN)
	$(AM_V_at)( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

test: ibus-engine-unikey
	$(builddir)/ibus-engine-unikey

