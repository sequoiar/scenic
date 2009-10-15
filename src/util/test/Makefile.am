include $(top_srcdir)/globals.mak

bin_PROGRAMS = utilTester

utilTester_SOURCES = utilTester.cpp


utilTester_LDADD = $(top_builddir)/src/util/libutil.la $(BOOST_THREAD_LIB) $(top_builddir)/src/boost/libbutil.la -ldl
utilTester_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)

