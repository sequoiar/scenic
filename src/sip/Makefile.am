include $(top_srcdir)/src/globals.mak

PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsdp

WRAPPER_LIBS =  /usr/lib/libpython2.5.so.1.0 -lboost_python 
WRAPPER_FLAGS = -I /usr/include/python2.5 -I /usr/include/boost/python/

sip_export_so_SOURCES = sip_export.cpp
sip_export_so_CXXFLAGS = -fPIC -g -Wall $(WRAPPER_FLAGS)
sip_export_so_LDFLAGS = -shared $(WRAPPER_LIBS) libsip.la

noinst_LTLIBRARIES = libsip.la
noinst_PROGRAMS = sip_export.so

libsip_la_SOURCES = \
					uri.cpp \
					uri.h \
					sdpmedia.cpp \
					sdpmedia.h \
					sdpcodec.cpp \
					sdp.cpp \
					useragent.cpp \
					sipsession.cpp \
					sdpcodec.h \
					sdp.h \
					useragent.h \
					session.h \
					sipsession.h 

libsip_la_LDFLAGS = $(PJSIP_LIBS) -lpthread $(WRAPPER_LIBS)
libsip_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS) $(WRAPPER_FLAGS)


