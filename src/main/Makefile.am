include $(top_srcdir)/globals.mak

bin_PROGRAMS = milhouse

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

install-exec-hook: 
		help2man -N -i milhouse-man.txt -n "milhouse is a gstreamer-based application for transmission and reception of your fine audio-visual content (command line edition)." ./milhouse > milhouse.1
		install -D -m 644 milhouse.1 $(DESTDIR)$(mandir)/man1/milhouse.1
