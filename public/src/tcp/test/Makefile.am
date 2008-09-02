include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = tcpTester parserTester

tcpTester_SOURCES = tcpTester.cpp

parserTester_SOURCES = esc.cpp 

tcpTester_LDADD = $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 
parserTester_LDADD = $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 


