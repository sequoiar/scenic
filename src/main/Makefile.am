include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = pof

pof_SOURCES = main.cpp mainTcp.cpp sropulpof.cpp

LDADD = $(src)/gutil/libGUtil.a $(src)/gst/control/libgstthread.a $(src)/gst/engine/libgst.a $(GLIB_LIBS) $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a $(src)/gst/engine/libgst.a $(LIBLO_LIBS) $(GTK_LIBS) $(LIBUTIL) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS)

pof_CPPFLAGS= $(AM_CPPFLAGS)  $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)

install-exec-local:
	ln -s $(prefix)/bin/pof $(prefix)/bin/mainTester
