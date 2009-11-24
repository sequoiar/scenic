include $(top_srcdir)/globals.mak
if PROJ_DEBUG
SUBDIRS = . test
endif

lib_LTLIBRARIES = libgst.la

libgst_la_SOURCES = caps.cpp mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp \
				   videoConfig.cpp rtpBin.cpp pipeline.cpp \
				   rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp gstLinkable.cpp videoSink.cpp audioSink.cpp jackUtils.cpp \
				   codec.cpp rtpPay.cpp audioConfig.cpp remoteConfig.cpp messageDispatcher.cpp messageHandler.cpp \
				   videoReceiver.cpp videoSender.cpp videoSource.cpp raw1394Util.cpp \
				   audioLevel.cpp glVideoSink.cpp playback.cpp dv1394.cpp v4l2util.cpp fileSource.cpp \
				   sharedVideoSink.cpp sharedVideoBuffer.cpp capsParser.cpp busMsgHandler.cpp \
                   mediaBase.h messageDispatcher.h messageHandler.h busMsgHandler.h ../ports.h \
				   audioSender.h audioReceiver.h $(inc)/alsa.h caps.h \
				   videoConfig.h rtpBin.h pipeline.h \
				   rtpSender.h rtpReceiver.h audioSource.h \
				   gstLinkable.h videoSink.h audioSink.h $(inc)/jackUtils.h \
				   codec.h rtpPay.h audioConfig.h remoteConfig.h \
				   videoReceiver.h videoSender.h videoSource.h raw1394Util.h \
				   audioLevel.h glVideoSink.h playback.h dv1394.h v4l2util.h \
				   fileSource.h sharedVideoSink.h sharedVideoBuffer.h capsParser.h \
				   dc1394.cpp dc1394.h videoScale.h videoScale.cpp

BUILT_SOURCES = caps.cpp

caps.cpp: $(top_srcdir)/utils/make_caps.py
	$< $@

clean-local:
	rm -f caps.cpp
		
libgst_la_CPPFLAGS=$(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(GLU_CFLAGS) $(BOOST_CPPFLAGS) $(DC1394_CFLAGS)

#CXXFLAGS += -O3
