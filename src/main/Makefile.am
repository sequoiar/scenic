include $(top_srcdir)/src/globals.mak
bin_PROGRAMS = mainTester 

mainTester_SOURCES = gstThread.cpp main.cpp 




LDADD = $(GLIB_LIBS) $(GST_LIBS) $(GSTAUDIO_LIBS) $(GSTCONTROLLER_LIBS) $(LIBLO_LIBS) $(src)/osc/libOsc.a $(src)/gst/libgst.a $(src)/gutil/libGUtil.a 
AM_CPPFLAGS= $(GLIB_CFLAGS) $(GST_CFLAGS)
