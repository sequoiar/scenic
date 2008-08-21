include $(top_srcdir)/src/globals.mak

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia

WRAPPER_LIBS =  /usr/lib/libpython2.5.so.1.0 -lboost_python 

bin_PROGRAMS = uacTester uasTester cliTester

uacTester_SOURCES = uacTestSuite.cpp
uasTester_SOURCES = uasTestSuite.cpp
cliTester_SOURCES = cliTestSuite.cpp

uacTester_LDADD = $(PJSIP_LIBS) -lpthread ../libsip.la  $(CPPTEST_LIBS) $(WRAPPER_LIBS) 
uacTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)

uasTester_LDADD = $(PJSIP_LIBS) -lpthread ../libsip.la  $(CPPTEST_LIBS) $(WRAPPER_LIBS)
uasTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)

cliTester_LDADD = $(PJSIP_LIBS) -lpthread ../libsip.la  $(CPPTEST_LIBS) $(WRAPPER_LIBS)
cliTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)
