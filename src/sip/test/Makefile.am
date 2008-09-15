include $(top_srcdir)/src/globals.mak

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia

WRAPPER_LIBS =  /usr/lib/libpython2.5.so.1.0 -lboost_python 

bin_PROGRAMS = cliTester mediaTester

cliTester_SOURCES = cliTestSuite.cpp
mediaTester_SOURCES = mediaTestSuite.cpp

cliTester_LDADD = $(PJSIP_LIBS) -lpthread ../libsip.la  $(CPPTEST_LIBS) $(WRAPPER_LIBS)
cliTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS) -I$(PYTHON_INCLUDE_DIR)

mediaTester_LDADD = $(PJSIP_LIBS) -lpthread ../libsip.la  $(CPPTEST_LIBS) $(WRAPPER_LIBS)
mediaTester_CPPFLAGS= $(AM_CPPFLAGS) $(PJ_CFLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS) -I$(PYTHON_INCLUDE_DIR)
