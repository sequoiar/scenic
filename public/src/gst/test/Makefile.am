include $(top_srcdir)/src/globals.mak

#bin_PROGRAMS = audioTester rtpAudioTester videoTester syncTester rtpVideoTester
bin_PROGRAMS = allTester
allTester_SOURCES =   syncTestSuite.cpp videoTestSuite.cpp rtpVideoTestSuite.cpp rtpAudioTestSuite.cpp audioTestSuite.cpp gstTestSuite.cpp main.cpp
#audioTester_SOURCES = audioTestSuite.cpp gstTestSuite.cpp
#rtpAudioTester_SOURCES = rtpAudioTestSuite.cpp gstTestSuite.cpp
#videoTester_SOURCES = videoTestSuite.cpp gstTestSuite.cpp
#rtpVideoTester_SOURCES = rtpVideoTestSuite.cpp gstTestSuite.cpp
#syncTester_SOURCES = syncTestSuite.cpp gstTestSuite.cpp

LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(JACK_LIBS) $(GTK_LIBS) ../libgst.a $(src)/tcp/libtcp.a $(LIBUTIL) $(LIBAVC1394_LIBS)
allTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS)
#audioTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) 
#rtpAudioTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS)
#videoTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) 
#rtpVideoTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS) 
#syncTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) 

