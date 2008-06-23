proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = sipTester

sipTester_SOURCES = sipTestSuite.cpp

sipTester_LDADD = $(proj_src)/sip/libsip.a $(proj_src)/sdp/libsdp.a $(PJSIP_LIBS) $(CPPTEST_LIBS) 
sipTester_LDFLAGS= -pthread 
sipTester_CPPFLAGS= $(PJSIP_CFLAGS) -I..


