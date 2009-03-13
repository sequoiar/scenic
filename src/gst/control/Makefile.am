include $(top_srcdir)/globals.mak
if PROJ_DEBUG
SUBDIRS = . #test
endif

noinst_LIBRARIES= libgstthread.a

libgstthread_a_SOURCES = gstThread.cpp msgThreadFactory.cpp \
                         gstReceiverThread.h gstSenderThread.h gstThread.h \
                         ../audioFactory.h ../videoFactory.h \
                         $(inc)/msgThreadFactory.h $(inc)/msgThread.h \
                         ../ports.h ../engine.h

