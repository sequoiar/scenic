proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = gstTester gstTester2

gstTester_SOURCES = gstTestSuite.cpp
gstTester2_SOURCES = gstTestSuite2.cpp

LDADD = $(GST_LIBS) $(GSTAUDIO_LIBS) $(GSTCONTROLLER_LIBS) $(CPPTEST_LIBS) $(LIBLO_LIBS) ../libgst.a
AM_CPPFLAGS= $(GST_CFLAGS) -I../ 

