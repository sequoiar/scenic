proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = mainTester 

mainTester_SOURCES = gstThread.cpp main.cpp 




LDADD = $(GLIB_LIBS) $(GST_LIBS) $(GSTAUDIO_LIBS) $(GSTCONTROLLER_LIBS) $(LIBLO_LIBS) $(proj_src)/osc/libOsc.a $(proj_src)/gst/libgst.a $(proj_src)/gutil/libGUtil.a 
AM_CPPFLAGS= $(GLIB_CFLAGS) $(GST_CFLAGS)
