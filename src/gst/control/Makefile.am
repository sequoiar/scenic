include $(top_srcdir)/globals.mak
if PROJ_DEBUG
SUBDIRS = . #test
endif

lib_LTLIBRARIES= libgstthread.la

libgstthread_la_SOURCES = gstThread.cpp gstReceiverThread.h gstSenderThread.h gstThread.h $(src)/gst/videoFactory.h \
                          $(src)/gst/audioFactory.h $(src)/gst/engine.h


