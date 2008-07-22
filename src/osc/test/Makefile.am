include $(top_srcdir)/src/globals.mak
bin_PROGRAMS = oscTester

oscTester_SOURCES = oscTester.cpp

oscTester_LDADD = ../libOsc.a $(GLIB_LIBS) $(LIBLO_LIBS) $(CPPTEST_LIBS) 
#oscTester_LDFLAGS= -pthread 
oscTester_CPPFLAGS= $(GLIB_CFLAGS) 


