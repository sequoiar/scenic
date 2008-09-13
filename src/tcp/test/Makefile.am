include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = tcpTester parserTester

tcpTester_SOURCES = tcpTester.cpp

parserTester_SOURCES = parserTester.cpp 

#LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 
LDADD = $(ALL_GST_LIBS) $(CPPTEST_LIBS) $(LIBLO_LIBS) $(JACK_LIBS) $(GTK_LIBS) $(LIBUTIL) ../libtcp.a 
#parserTester_LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 


