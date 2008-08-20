include $(top_srcdir)/src/globals.mak
PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsdp

## .so's are in .libs
noinst_LTLIBRARIES = libsip.la libsip_export.la

libsip_export_la_SOURCES = sip_export.cpp
libsip_export_la_LDFLAGS = $(src)/sip/libsip.la -l$(BOOST_PYTHON_LIB)

libsip_export_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS) -I$(PYTHON_INCLUDE_DIR) 
libsip_export_la_LIBADD = $(PJSIP_LIBS) -l$(BOOST_PYTHON_LIB) 



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

libsip_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS) -I$(PYTHON_INCLUDE_DIR) 


