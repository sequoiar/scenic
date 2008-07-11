proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = mainTester gstTester

mainTester_SOURCES = test.cpp 
gstTester_SOURCES = gstThread.cpp




LDADD = $(GLIB_LIBS) $(GST_LIBS) $(proj_src)/gst/libgst.a $(proj_src)/gutil/libGUtil.a 
AM_CPPFLAGS= $(GLIB_CFLAGS) $(GST_CFLAGS)

