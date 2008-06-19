bin_PROGRAMS = sipMain

noinst_LIBRARIES = libpjsip.a

libpjsip_a_SOURCES = sip.cpp sipStateless.cpp

#libgst_a_LIBADD = $(GST_LIBS) $(GSTAUDIO_LIBS) 
libpjsip_a_CPPFLAGS= $(PJSIP_CFLAGS) -I../

sipMain_SOURCES = sipMain.cpp
sipMain_LDADD = $(PJSIP_LIBS) libpjsip.a
sipMain_CPPFLAGS= $(PJSIP_CFLAGS) -I../
