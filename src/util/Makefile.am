pkglib_LTLIBRARIES= libutil.la
# needed by libshared_video
pkginclude_HEADERS = $(top_srcdir)/src/include/video_size.h

libutil_la_SOURCES = log_writer.cpp sigint.cpp \
					log_writer.h sigint.h $(top_srcdir)/src/include/except.h
libutil_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src
LDADD = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB)
