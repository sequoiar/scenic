include $(top_srcdir)/globals.mak
SUBDIRS= . test

noinst_LIBRARIES= libutil.a

libutil_a_SOURCES = logWriter.cpp util.cpp mapMsg.cpp parser.cpp sigint.cpp baseThread.cpp \
					$(inc)/util.h $(inc)/baseModule.h $(inc)/baseThread.h $(inc)/lassert.h \
					$(inc)/mapMsg.h $(inc)/msgThread.h $(inc)/queuePair.h \
					logWriter.h sigint.h util.h $(inc)/except.h
