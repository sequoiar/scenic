include $(top_srcdir)/globals.mak

bin_PROGRAMS = milhouse

milhouse_SOURCES = milhouse.cpp mainTcp.cpp msgThreadFactory.cpp msgThreadFactory.h
if SVN
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) -D SVNVERSION=\"$(shell svnversion)\"
else
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) 
endif
LDADD = $(top_builddir)/src/gst/control/libgstthread.la $(top_builddir)/src/gst/engine/libgst.la $(ALL_GST_LIBS) $(JACK_LIBS) \
        $(top_builddir)/src/tcp/libtcp.la \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(top_builddir)/src/boost/libbutil.la $(BOOST_ASIO_LIB) \
		$(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(LIBUTIL) $(top_builddir)/src/gutil/libgutil.la

#CXXFLAGS += -O3
