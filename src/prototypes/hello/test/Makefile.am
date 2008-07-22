include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = helloTester

helloTester_SOURCES = test.cpp

LDADD = $(GLIB_LIBS) ../libhello.la $(src)/gutil/libGUtil.a
AM_CPPFLAGS= $(GLIB_CFLAGS) 
