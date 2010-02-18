include $(top_srcdir)/globals.mak
	
utildir = $(libdir)/scenic
util_LTLIBRARIES= libutil.la
# needed by libshared_video
include_HEADERS = $(inc)/videoSize.h

libutil_la_SOURCES = logWriter.cpp mapMsg.cpp parser.cpp sigint.cpp \
					 programOptions.cpp $(inc)/programOptions.h \
					$(inc)/util.h $(inc)/lassert.h \
					$(inc)/mapMsg.h logWriter.h \
					sigint.h $(inc)/videoSize.h $(inc)/except.h

LDADD=$(BOOST_LDFLAGS)
