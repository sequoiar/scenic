proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = helloTester

helloTester_SOURCES = test.cpp

LDADD = -L../.. $(GLIB_LIBS) ../libhello.a $(proj_src)/gutil/libGUtil.a
AM_CPPFLAGS= $(GLIB_CFLAGS) -I..
