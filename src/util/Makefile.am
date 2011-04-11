noinst_LTLIBRARIES= libutil.la
libutil_la_SOURCES = logWriter.cpp sigint.cpp \
					 programOptions.cpp $(top_srcdir)/src/include/programOptions.h \
					$(top_srcdir)/src/include/util.h \
					logWriter.h sigint.h $(top_srcdir)/src/include/except.h
libutil_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src
LDADD = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
