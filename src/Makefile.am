libexec_PROGRAMS = ibus-engine-unikey

ibus_engine_unikey_SOURCES = \
	main.cpp \
	engine.cpp \
	engine.h \
	$(NULL)
ibus_engine_unikey_CXXFLAGS = \
	@IBUS_CFLAGS@ \
	-I../ukengine/ \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	$(NULL)
ibus_engine_unikey_LDFLAGS = \
	@IBUS_LIBS@ \
	../ukengine/libunikey-zmod.la \
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
