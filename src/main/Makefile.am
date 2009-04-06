include $(top_srcdir)/globals.mak

bin_PROGRAMS = milhouse

milhouse_SOURCES = milhouse.cpp mainTcp.cpp

#CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
#LDADD = $(src)/gst/engine/libgst.a $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
#		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(LIBUTIL) $(src)/gutil/libgutil.a 

milhouse_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
LDADD = $(src)/gst/control/libgstthread.a $(src)/gst/engine/libgst.a $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(LIBUTIL) $(src)/gutil/libgutil.a $(BOOST_ASIO_LIB) \
		$(BOOST_LDFLAGS) $(BOOST_THREAD_LIB)
