SUBDIRS = src locale data py doc utils man 
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = RELEASE

install-exec-hook:
	@if test -e doc/html; then mkdir -p $(DESTDIR)$(prefix)/share/doc/sphinx; cp -fvr py/docs/_build/html/ $(DESTDIR)$(prefix)/share/doc/sphinx; fi
	@if test -e doc/html; then \
	echo INSTALLING docs at $(DESTDIR)$(prefix); \
	mkdir -p $(DESTDIR)$(prefix)/share/doc/doxygen; \
	cp -rfv doc/html $(DESTDIR)$(prefix)/share/doc/doxygen; \
	fi
