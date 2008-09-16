include $(top_srcdir)/src/globals.mak

noinst_LIBRARIES = libgst.a

libgst_a_SOURCES = videoReceiver.cpp videoSender.cpp videoSource.cpp mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp mediaConfig.cpp \
				   audioConfig.cpp videoConfig.cpp rtpSession.cpp pipeline.cpp \
				   gstBase.cpp rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp gstLinkable.cpp videoSink.cpp audioSink.cpp jackUtils.cpp \
				   codec.cpp rtpPay.cpp #../util/util.cpp


libgst_a_CPPFLAGS= $(AM_CPPFLAGS) $(GST_CFLAGS) $(LIBLO_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS)
