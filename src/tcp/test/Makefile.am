include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = tcpTester parserTester

tcpTester_SOURCES = tcpTester.cpp

parserTester_SOURCES = parserTester.cpp 

#LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 
LDADD = $(CPPTEST_LIBS) $(GLIB_LIBS) ../libtcp.a $(LIBUTIL) 
#parserTester_LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(CPPTEST_LIBS) 


