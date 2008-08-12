include $(top_srcdir)/src/globals.mak

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia

bin_PROGRAMS = uacTester uasTester

uacTester_SOURCES = uacTestSuite.cpp
uasTester_SOURCES = uasTestSuite.cpp

uacTester_LDADD = $(PJSIP_LIBS) ../libsip.la -lpthread  $(CPPTEST_LIBS)
uacTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)  

uasTester_LDADD = $(PJSIP_LIBS) ../libsip.la -lpthread  $(CPPTEST_LIBS)
uasTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)  

