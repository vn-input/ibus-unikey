SUBDIRS = \
	ukengine \
	src \
    setup \
	icons \
	m4 \
	po \
	$(NULL)

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
    ibus-unikey.spec.in \
    debian

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

deb: distdir
	cd $(distdir) && debuild --no-tgz-check

clean-deb:
	$(RM) -r @PACKAGE_NAME@_@PACKAGE_VERSION@*.{dsc,tar.gz,build,changes,deb}

clean-local: clean-rpm clean-deb
	$(RM) -r $(distdir)

