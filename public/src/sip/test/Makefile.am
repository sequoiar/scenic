include $(top_srcdir)/src/globals.mak

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia

bin_PROGRAMS = sipsessionTester

sipsessionTester_SOURCES = sipsessionTest.cpp

sipsessionTester_LDADD = $(PJSIP_LIBS) ../libsip.la -lpthread  $(CPPTEST_LIBS)
sipsessionTester_CPPFLAGS= $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)  


