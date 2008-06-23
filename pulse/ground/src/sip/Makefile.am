proj_src = $(top_srcdir)/src
INCLUDES = -I$(proj_src)/include -I$(proj_src)

bin_PROGRAMS = sipMain

noinst_LIBRARIES = libsip.a

libsip_a_SOURCES = sip.cpp sipStateless.cpp

libsip_a_CPPFLAGS= $(PJSIP_CFLAGS) -I../

sipMain_SOURCES = sipMain.cpp 
sipMain_LDADD = libsip.a $(proj_src)/sdp/libsdp.a $(PJSIP_LIBS) $(GST_LIBS) 
sipMain_CPPFLAGS= $(PJSIP_CFLAGS) $(GST_CFLAGS) -I../

