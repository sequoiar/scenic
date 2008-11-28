include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = mainTester 

mainTester_SOURCES = main.cpp 


LDADD = $(src)/gst/control/libgstthread.a  $(GLIB_LIBS) $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a $(src)/gst/engine/libgst.a $(LIBLO_LIBS) $(GTK_LIBS) $(LIBUTIL) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS)
mainTester_CPPFLAGS= $(AM_CPPFLAGS)  $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
