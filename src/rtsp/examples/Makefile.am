noinst_PROGRAMS = gst-rtsp-cam test-client

gst_rtsp_cam_SOURCES = \
	$(top_srcdir)/src/rtsp/gst-rtsp-cam-media-factory.c \
	$(top_srcdir)/src/rtsp/gst-rtsp-cam-media-factory.h \
	gst-rtsp-cam.c

test_client_SOURCES = test-client.cpp
test_client_CXXFLAGS = $(GST_CFLAGS)
test_client_LDADD = $(GST_LIBS)

gst_rtsp_cam_CFLAGS = $(GST_CFLAGS) $(GST_RTSP_SERVER_CFLAGS)
gst_rtsp_cam_LDADD = $(GST_LIBS) $(GST_RTSP_SERVER_LIBS)
INCLUDES = -I$(top_srcdir)/src/rtsp
