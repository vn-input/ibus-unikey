INCLUDES = @DEFINE@ \
	-I$(top_srcdir)/ukengine \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(localedir)\"

libexec_PROGRAMS = ibus-engine-unikey

ibus_engine_unikey_SOURCES = \
	main.cpp \
	engine.cpp \
	engine.h \
	engine_private.h \
	utils.cpp \
	utils.h \
	$(NULL)
ibus_engine_unikey_CXXFLAGS = \
	@IBUS_CFLAGS@ \
	$(NULL)
ibus_engine_unikey_LDFLAGS = \
	@IBUS_LIBS@ \
	$(top_builddir)/ukengine/libunikey-ibus.la \
	$(NULL)

component_DATA = \
	unikey.xml \
	$(NULL)
componentdir = @datadir@/ibus/component

EXTRA_DIST = \
	unikey.xml.in \
	$(NULL)

CLEANFILES = \
	unikey.xml \
	$(NULL)

unikey.xml: unikey.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

test: ibus-engine-unikey
	$(builddir)/ibus-engine-unikey
