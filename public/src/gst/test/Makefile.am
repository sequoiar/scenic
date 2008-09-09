include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = audioTester videoTester syncTester 

audioTester_SOURCES = audioTestSuite.cpp gstTestSuite.cpp
videoTester_SOURCES = videoTestSuite.cpp gstTestSuite.cpp
syncTester_SOURCES = syncTestSuite.cpp gstTestSuite.cpp

LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(LIBLO_LIBS) $(JACK_LIBS) $(GTK_LIBS) ../libgst.a
audioTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(LIBLO_CFLAGS)
videoTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS)
syncTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(LIBLO_CFLAGS)

