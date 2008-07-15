proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = sdpTester

sdpTester_SOURCES = sdpTestSuite.cpp

LDADD = $(CPPTEST_LIBS) ../libsdp.a
AM_CPPFLAGS= $(CPPTEST_CFLAGS) $(GST_CFLAGS) -I../

