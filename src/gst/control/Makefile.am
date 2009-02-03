include $(top_srcdir)/src/globals.mak
if PROJ_DEBUG
SUBDIRS = . #test
endif

noinst_LIBRARIES= libgstthread.a

libgstthread_a_SOURCES = gstReceiverThread.cpp gstThread.cpp \
					msgThreadFactory.cpp gstSenderThread.h gstReceiverThread.h \
					gstThread.h msgThreadFactory.h

