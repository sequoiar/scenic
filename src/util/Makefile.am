include $(top_srcdir)/globals.mak
SUBDIRS= . test

	
lib_LTLIBRARIES= libutil.la

libutil_la_SOURCES = logWriter.cpp util.cpp mapMsg.cpp parser.cpp sigint.cpp baseThread.cpp \
					$(inc)/util.h $(inc)/baseModule.h $(inc)/baseThread.h $(inc)/lassert.h \
					$(inc)/mapMsg.h $(inc)/msgThread.h $(inc)/queuePair.h \
					logWriter.h sigint.h util.h $(inc)/except.h
