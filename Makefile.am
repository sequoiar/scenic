include $(top_srcdir)/globals.mak
SUBDIRS = src 
DIST_SUBDIRS = src py doc
ACLOCAL_AMFLAGS = -I m4

#EXTRA_DIST = doc/Makefile py/docs/Makefile 

dist-hook:
		./utils/list_miville_files $(top_distdir) 
		install -D utils/list_miville_files $(top_distdir)/utils/list_miville_files
	    install -D doc/Makefile $(top_distdir)/doc/Makefile
	    install -D py/docs/Makefile $(top_distdir)/py/docs/Makefile
