pkglib_LTLIBRARIES = libgst.la

libgst_la_SOURCES = mediaBase.cpp \
				   audioSender.cpp audioReceiver.cpp \
				   videoConfig.cpp rtpBin.cpp pipeline.cpp \
				   rtpSender.cpp rtpReceiver.cpp audioSource.cpp \
				   interleave.cpp gstLinkable.cpp videoSink.cpp audioSink.cpp jackUtils.cpp \
				   codec.cpp rtpPay.cpp audioConfig.cpp remoteConfig.cpp messageDispatcher.cpp messageHandler.cpp \
				   videoReceiver.cpp videoSender.cpp localAudio.cpp localVideo.cpp videoSource.cpp raw1394Util.cpp \
				   audioLevel.cpp playback.cpp dv1394.cpp v4l2util.cpp fileSource.cpp \
				   sharedVideoSink.cpp busMsgHandler.cpp videoFlip.cpp videoScale.cpp videoFactory.cpp textOverlay.cpp \
                   audioFactory.cpp mediaBase.h messageDispatcher.h messageHandler.h busMsgHandler.h portOffsets.h \
				   audioSender.h audioReceiver.h \
				   videoConfig.h rtpBin.h pipeline.h \
				   rtpSender.h rtpReceiver.h audioSource.h \
				   gstLinkable.h videoSink.h audioSink.h jackUtils.h \
				   codec.h rtpPay.h audioConfig.h remoteConfig.h \
				   videoReceiver.h videoSender.h videoSource.h raw1394Util.h \
				   audioLevel.h $(top_srcdir)/src/include/playback.h dv1394.h v4l2util.h \
				   fileSource.h sharedVideoSink.h \
				   dc1394.cpp dc1394.h videoScale.h localAudio.h localVideo.h \
				   $(top_srcdir)/include/noncopyable.h videoFlip.h videoFactory.h audioFactory.h textOverlay.h
		
libgst_la_CPPFLAGS=$(GLIB_CFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(GLU_CFLAGS) $(BOOST_CPPFLAGS) $(DC1394_CFLAGS) -DPIXMAPS_DIR=\"$(PIXMAPS_DIR)\" -I$(top_srcdir)/src/include -I$(top_srcdir)/src
