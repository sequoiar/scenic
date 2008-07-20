include $(top_srcdir)/src/globals.mak

noinst_LIBRARIES = libgst.a

libgst_a_SOURCES = videoReceiver.cpp videoSender.cpp mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp mediaConfig.cpp \
				   audioConfig.cpp videoConfig.cpp rtpSession.cpp pipeline.cpp \
				   gstBase.cpp rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp syncTester.cpp


libgst_a_CPPFLAGS= $(CPPFLAGS) $(GST_CFLAGS) $(LLO_CFLAGS) 
#libgst_a_LIBADD = $(GST_LIBS) $(GSTAUDIO_LIBS) $(GSTCONTROLLER_LIBS) 

#bin_PROGRAMS = gstApp
#gstApp_SOURCES = gst_main.cpp gstApp.cpp
#gstApp_LDADD = $(GST_LIBS) $(GSTAUDIO_LIBS) libgst.a
#gstApp_CPPFLAGS= $(GST_CFLAGS) -I../
