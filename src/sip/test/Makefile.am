bin_PROGRAMS = sipTester

sipTester_SOURCES = sipTestSuite.cpp

sipTester_LDADD = ../libsip.a ../../sdp/libsdp.a $(PJSIP_LIBS) $(CPPTEST_LIBS) 
sipTester_LDFLAGS= -pthread
sipTester_CPPFLAGS= $(PJSIP_CFLAGS) -I../ -I../../


