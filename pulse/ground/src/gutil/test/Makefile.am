proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = threadTester argsTester

threadTester_SOURCES = thread.cpp
argsTester_SOURCES = test.cpp

LDADD = $(GLIB_LIBS) ../libGUtil.a
AM_CPPFLAGS = $(GLIB_CFLAGS) -I..
