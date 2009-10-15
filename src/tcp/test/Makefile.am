include $(top_srcdir)/globals.mak

bin_PROGRAMS = parserTester

parserTester_SOURCES = parserTester.cpp 

#LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 
LDADD = $(CPPTEST_LIBS) $(GLIB_LIBS) $(top_builddir)/src/tcp/libtcp.la \
    $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(LIBUTIL) $(BOOST_PROGRAM_OPTIONS_LIB) $(top_builddir)/src/boost/libbutil.la -ldl
#parserTester_LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(CPPTEST_LIBS) 


