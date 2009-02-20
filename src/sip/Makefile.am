include $(top_srcdir)/globals.mak
#if x86_64 pjsip must be built with -fPIC option see note below
PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsdp

## .so's are in .libs
noinst_LTLIBRARIES = libsip.la 

lib_LTLIBRARIES = libsip_export.la

libsip_export_la_SOURCES = sip_export.cpp

libsip_export_la_LDFLAGS = -version-info 1:0:0 -l$(BOOST_PYTHON_LIB)
libsip_export_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS) -I$(PYTHON_INCLUDE_DIR) 
libsip_export_la_LIBADD = libsip.la $(PJSIP_LIBS) -l$(BOOST_PYTHON_LIB)  


libsip_la_SOURCES = \
					instantmessaging.cpp \
					instantmessaging.h \
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
					sipsession.h \
					../util/util.cpp

libsip_la_CPPFLAGS =  $(AM_CPPFLAGS) $(PJ_CFLAGS) -I$(PYTHON_INCLUDE_DIR) 
libsip_la_LDFLAGS = $(PJSIP_LIBS) -lpthread  

# Note:
#   To build pjsip for x86_64 in a shared lib context use
#   ./aconfigure CFLAGS='-Wno-unused-label -fPIC' 
#   when building pjsip
#
#   http://lists.pjsip.org/pipermail/pjsip_lists.pjsip.org/2007-November/000783.html
#
#

