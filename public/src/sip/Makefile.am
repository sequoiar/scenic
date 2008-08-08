include $(top_srcdir)/src/globals.mak


PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsdp

noinst_LTLIBRARIES = libsip.la

libsip_la_SOURCES = \
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

libsip_la_LDFLAGS = $(PJSIP_LIBS) -lpthread 
libsip_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS)
   





#BINdir = /usr/bin
# -lpjsua -lpjsip -lpjmedia -lpjsip-simple -lpjsip-ua -lpjmedia-codec -lpjlib-util -lpj
# PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsip-ua -lpjmedia -lpjmedia-codec
