noinst_LTLIBRARIES = libgst.la

libgst_la_SOURCES = media_base.cpp \
				   audio_sender.cpp audio_receiver.cpp \
				   video_config.cpp rtp_bin.cpp pipeline.cpp \
				   rtp_sender.cpp rtp_receiver.cpp audio_source.cpp \
				   gst_linkable.cpp video_sink.cpp audio_sink.cpp jack_util.cpp \
				   codec.cpp rtp_pay.cpp audio_config.cpp remote_config.cpp \
				   video_receiver.cpp video_sender.cpp local_audio.cpp \
                   local_video.cpp video_source.cpp raw1394_util.cpp \
				   audio_level.cpp dv1394.cpp v4l2_util.cpp file_source.cpp \
				   shared_video_sink.cpp bus_msg_handler.cpp video_flip.cpp \
                   video_scale.cpp video_factory.cpp text_overlay.cpp \
                   audio_factory.cpp media_base.h bus_msg_handler.h port_offsets.h \
				   audio_sender.h audio_receiver.h \
				   video_config.h rtp_bin.h pipeline.h \
				   rtp_sender.h rtp_receiver.h audio_source.h \
				   gst_linkable.h video_sink.h audio_sink.h jack_util.h \
				   codec.h rtp_pay.h audio_config.h remote_config.h \
				   video_receiver.h video_sender.h video_source.h raw1394_util.h \
				   audio_level.h dv1394.h v4l2_util.h file_source.h shared_video_sink.h \
				   dc1394.cpp dc1394.h video_scale.h local_audio.h local_video.h \
				   $(top_srcdir)/src/include/noncopyable.h video_flip.h video_factory.h audio_factory.h text_overlay.h

libgst_la_CPPFLAGS=$(GLIB_CFLAGS) $(GST_CFLAGS) $(JACK_CFLAGS) $(GTK_CFLAGS) $(GLU_CFLAGS) $(BOOST_CPPFLAGS) $(DC1394_CFLAGS) -DPIXMAPS_DIR=\"$(PIXMAPS_DIR)\" -I$(top_srcdir)/src/include -I$(top_srcdir)/src
