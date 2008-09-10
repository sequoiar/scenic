include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = mainTester 

mainTester_SOURCES = gstThread.cpp gstSenderThread.cpp main.cpp 

LDADD = $(GLIB_LIBS) $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a $(src)/gst/libgst.a $(src)/gutil/libGUtil.a  $(LIBLO_LIBS) $(GTK_LIBS)
mainTester_CPPFLAGS= $(AM_CPPFLAGS)  $(GST_CFLAGS) $(GTK_CFLAGS)
