include $(top_srcdir)/globals.mak
SUBDIRS= .

	
lib_LTLIBRARIES= libutil.la
# needed by libshared_video
include_HEADERS = $(inc)/videoSize.h

libutil_la_SOURCES = logWriter.cpp fileUtil.cpp mapMsg.cpp parser.cpp sigint.cpp \
					 programOptions.cpp $(inc)/programOptions.h \
					$(inc)/util.h $(inc)/lassert.h \
					$(inc)/mapMsg.h logWriter.h \
					sigint.h fileUtil.h $(inc)/videoSize.h $(inc)/except.h

LDADD=$(BOOST_LDFLAGS)
