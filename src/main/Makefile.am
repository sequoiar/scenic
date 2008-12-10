include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = pof

pof_SOURCES = main.cpp mainTcp.cpp sropulpof.cpp

LDADD = $(src)/gutil/libgutil.a $(src)/gst/control/libgstthread.a $(src)/gst/engine/libgst.a \
		$(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
		$(GTK_LIBS) $(LIBUTIL) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS)

pof_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)

install-exec-local:
	ln -sf $(prefix)/bin/pof $(prefix)/bin/mainTester
uninstall-local:
	rm -f $(prefix)/bin/mainTester
