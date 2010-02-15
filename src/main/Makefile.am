include $(top_srcdir)/globals.mak
SUBDIRS = test

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../:$(PATH) trial

TESTS = test/test_milhouse.py

bin_PROGRAMS = milhouse
dist_pkgdata_DATA = milhouse-man.txt
BUILT_SOURCES = milhouse-man.txt

milhouse_SOURCES = milhouse.cpp milhouseLogger.cpp milhouseLogger.h ConcurrentQueue.h milhouse.h
if SVN
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) -D SVNVERSION=\"$(shell svnversion)\"
else
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) 
endif
LDADD = $(top_builddir)/src/gst/engine/libgst.la $(ALL_GST_LIBS) $(JACK_LIBS) \
        $(top_builddir)/src/tcp/libtcp.la \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(BOOST_ASIO_LIB) \
		$(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(LIBDC1394_LIBS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(LIBUTIL) $(top_builddir)/src/gutil/libgutil.la $(top_builddir)/src/shared_video/libshared_video.la

clean-local:
	rm -rf _trial_temp

# The man pages depend on the --help strings and the version number.
# common_mandeps = $(top_srcdir)/configure.ac
#
# # Depend on the source, not the binary; we don't need to regenerate the
# # binary when any source file changes, only the main one.
# # Use -o so that the `missing' program can infer the output file.
# hello.1: $(top_srcdir)/src/hello.c $(common_mandeps)
# 	$(HELP2MAN) --name="friendly greeting program" ../src/hello -o $@


# FIXME: this breaks distcheck
#install-exec-hook: 
#	help2man -N -i $(DESTDIR)$(prefix)/share/scenic/milhouse-man.txt -n "milhouse is a gstreamer-based application for transmission and reception of your fine audio-visual content (command line edition)." ./milhouse > milhouse.1
#	install -D -m 644 milhouse.1 $(DESTDIR)$(mandir)/man1/milhouse.1
