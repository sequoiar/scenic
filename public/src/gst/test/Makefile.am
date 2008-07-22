include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = gstTester gstTester2

gstTester_SOURCES = gstTestSuite.cpp
gstTester2_SOURCES = gstTestSuite2.cpp

LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(LIBLO_LIBS) $(JACK_LIBS) ../libgst.a
AM_CPPFLAGS= $(GST_CFLAGS) $(JACK_FLAGS)

