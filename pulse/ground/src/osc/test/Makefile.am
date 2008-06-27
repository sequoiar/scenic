include $(top_srcdir)/src/globals.mak
bin_PROGRAMS = oscTester

oscTester_SOURCES = osc.cpp

oscTester_LDADD = ../libOscThread.a $(GLIB_LIBS) $(LIBLO_LIBS) $(CPPTEST_LIBS) 
oscTester_LDFLAGS= -pthread 
oscTester_CPPFLAGS= $(GLIB_CFLAGS) $(LIBLO_LIBS) -I..


