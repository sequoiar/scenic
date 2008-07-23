include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = audioTester videoTester syncTester gstTester2

audioTester_SOURCES = audioTestSuite.cpp gstTestSuite.cpp
videoTester_SOURCES = videoTestSuite.cpp gstTestSuite.cpp
syncTester_SOURCES = syncTestSuite.cpp gstTestSuite.cpp
gstTester2_SOURCES = gstTestSuite2.cpp

LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(LIBLO_LIBS) $(JACK_LIBS) ../libgst.a
AM_CPPFLAGS= $(GST_CFLAGS) $(JACK_CFLAGS)

