proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia

bin_PROGRAMS = sipsessionTester

sipsessionTester_SOURCES = sipsessionTest.cpp

sipsessionTester_LDADD =     $(PJSIP_LIBS) ../libsip.la -lpthread  $(CPPTEST_LIBS)
AM_CPPFLAGS= $(PJSIP_FLAGS) $(CPPTEST_CFLAGS) $(GST_CFLAGS)  -I../

