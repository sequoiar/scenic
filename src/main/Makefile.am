include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = propulseart

propulseart_SOURCES = sropulpof.cpp mainTcp.cpp

#CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
#LDADD = $(src)/gst/engine/libgst.a $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
#		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(LIBUTIL) $(src)/gutil/libgutil.a 

propulseart_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
LDADD = $(src)/gst/control/libgstthread.a $(src)/gst/engine/libgst.a $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
		$(GTK_LIBS) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(LIBUTIL) $(src)/gutil/libgutil.a 
