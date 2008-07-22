include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = threadTester argsTester 

threadTester_SOURCES = thread.cpp
argsTester_SOURCES = test.cpp

LDADD = $(GLIB_LIBS) ../libGUtil.a
AM_CPPFLAGS = $(GLIB_CFLAGS) 
