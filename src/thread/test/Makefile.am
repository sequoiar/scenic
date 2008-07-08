proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = threadTester

threadTester_SOURCES = thread.cpp

threadTester_LDADD = $(GLIB_LIBS) ../libGUtil.a
threadTester_CPPFLAGS = $(GLIB_CFLAGS) -I..
