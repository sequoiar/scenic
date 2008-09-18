include $(top_srcdir)/src/globals.mak

noinst_LIBRARIES = libgst.a

libgst_a_SOURCES = mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp \
				   videoConfig.cpp rtpSession.cpp pipeline.cpp \
				   gstBase.cpp rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp gstLinkable.cpp videoSink.cpp audioSink.cpp jackUtils.cpp \
				   codec.cpp rtpPay.cpp audioLocal.cpp audioConfig.cpp remoteConfig.cpp \
				   videoReceiver.cpp videoSender.cpp videoSource.cpp videoLocal.cpp


libgst_a_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(LIBLO_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS)
