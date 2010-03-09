utildir = $(libdir)/scenic
util_LTLIBRARIES= libutil.la
# needed by libshared_video
include_HEADERS = $(top_srcdir)/src/include/videoSize.h

libutil_la_SOURCES = logWriter.cpp sigint.cpp \
					 programOptions.cpp $(top_srcdir)/src/include/programOptions.h \
					$(top_srcdir)/src/include/util.h $(top_srcdir)/src/include/lassert.h \
					logWriter.h sigint.h $(top_srcdir)/src/include/except.h

LDADD=$(BOOST_LDFLAGS)
INCLUDES = -I$(top_srcdir)/src/include -I$(top_srcdir)/src
