proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = mainTester

mainTester_SOURCES = test.cpp

LDADD = $(GLIB_LIBS) $(proj_src)/gutil/libGUtil.a
AM_CPPFLAGS= $(GLIB_CFLAGS) 
