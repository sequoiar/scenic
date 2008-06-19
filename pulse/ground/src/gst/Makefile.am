bin_PROGRAMS = gstApp

noinst_LIBRARIES = libgst.a

libgst_a_SOURCES = videoReceiver.cpp videoSender.cpp mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp mediaConfig.cpp \
				   audioConfig.cpp videoConfig.cpp

#libgst_a_LIBADD = $(GST_LIBS) $(GSTAUDIO_LIBS) 
libgst_a_CPPFLAGS= $(GST_CFLAGS) -I../

gstApp_SOURCES = gst_main.cpp gstApp.cpp
gstApp_LDADD = $(GST_LIBS) $(GSTAUDIO_LIBS) libgst.a
gstApp_CPPFLAGS= $(GST_CFLAGS) -I../
