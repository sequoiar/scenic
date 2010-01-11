include $(top_srcdir)/globals.mak
SUBDIRS = src py doc utils
DIST_SUBDIRS = src py doc utils
ACLOCAL_AMFLAGS = -I m4

#EXTRA_DIST = doc/Makefile py/docs/Makefile 

dist-hook:
		./utils/list_miville_files $(top_distdir) 
		install -D utils/list_miville_files $(top_distdir)/utils/list_miville_files
#install -D doc/Makefile $(top_distdir)/doc/Makefile
	    find doc/ -exec install -D {} $(top_distdir)/{} \;
	    find py/docs/ -exec install -D {} $(top_distdir)/{} \;
install-exec-hook:
	@if test -e doc/html; then mkdir -p $(DESTDIR)$(prefix)/share/doc/sphinx; cp -fvr py/docs/_build/html/ $(DESTDIR)$(prefix)/share/doc/sphinx; fi
	@if test -e doc/html; then \
    echo INSTALLING docs at $(DESTDIR)$(prefix); \
	mkdir -p $(DESTDIR)$(prefix)/share/doc/doxygen; \
	cp -rfv doc/html $(DESTDIR)$(prefix)/share/doc/doxygen; \
	fi

