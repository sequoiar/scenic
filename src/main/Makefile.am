include $(top_srcdir)/globals.mak

bin_PROGRAMS = milhouse

milhouse_SOURCES = milhouse.cpp mainTcp.cpp msgThreadFactory.cpp msgThreadFactory.h
if SVN
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) -D SVNVERSION=\"$(shell svnversion)\"
else
milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS) 
endif
LDADD = $(src)/gst/control/libgstthread.a $(src)/gst/engine/libgst.a $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(src)/gutil/libgutil.a $(src)/boost/libbutil.a $(BOOST_ASIO_LIB) \
		$(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)  $(LIBUTIL)      

#CXXFLAGS += -O3
