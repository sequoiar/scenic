include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = allTester
allTester_SOURCES =   syncTestSuite.cpp rtpSyncTestSuite.cpp videoTestSuite.cpp rtpVideoTestSuite.cpp rtpAudioTestSuite.cpp audioTestSuite.cpp gstTestSuite.cpp sropulpof.cpp main.cpp 

LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(JACK_LIBS) $(GTK_LIBS) ../libgst.a $(src)/gutil/libGUtil.a $(src)/tcp/libtcp.a $(LIBUTIL) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS)
allTester_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)

