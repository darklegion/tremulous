VERSION=1.1.0_SVN$(shell LANG=C svnversion .)

release debug clean distclean copyfiles installer:
	$(MAKE) -C src/unix $@

dist:
	rm -rf tremulous-$(VERSION)
	svn export . tremulous-$(VERSION)
	which convert >/dev/null 2>&1 && convert web/images/thenameofthisprojectis3.jpg tremulous-$(VERSION)/code/unix/setup/splash.xpm || true
	tar --force-local -cjf tremulous-$(VERSION).tar.bz2 tremulous-$(VERSION)
	rm -rf tremulous-$(VERSION)

.PHONY: release debug clean distclean copyfiles installer
