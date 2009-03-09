include $(top_srcdir)/globals.mak
if PROJ_DEBUG
SUBDIRS = . test
endif

noinst_LIBRARIES = libgst.a

libgst_a_SOURCES = mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp \
				   videoConfig.cpp rtpBin.cpp pipeline.cpp \
				   rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp gstLinkable.cpp videoSink.cpp audioSink.cpp jackUtils.cpp \
				   codec.cpp rtpPay.cpp audioLocal.cpp audioConfig.cpp remoteConfig.cpp \
				   videoReceiver.cpp videoSender.cpp videoSource.cpp videoLocal.cpp raw1394Util.cpp \
				   audioLevel.cpp glVideoSink.cpp playback.cpp dv1394.cpp v4l2util.cpp


libgst_a_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(GLU_CFLAGS)
