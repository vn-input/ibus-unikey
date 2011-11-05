SUBDIRS = \
	ukengine \
	src \
    setup \
	icons \
	m4 \
	po \
	$(NULL)

MAKEFLAGS = --no-print-directory

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
    ibus-unikey.spec.in

DISTCLEANFILES = \
	po/stamp-it \
	$(NULL)

rpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bb \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_specdir `pwd`" \
			@PACKAGE_NAME@.spec

srpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bs \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_specdir `pwd`" \
			@PACKAGE_NAME@.spec

clean-rpm:
	$(RM) -r "`uname -i`"
	$(RM) -r @PACKAGE_NAME@-@PACKAGE_VERSION@*.src.rpm

clean-local: clean-rpm
	$(RM) -r $(distdir)

