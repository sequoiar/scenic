noinst_PROGRAMS = gst-rtsp-cam

gst_rtsp_cam_SOURCES = \
	$(top_srcdir)/src/rtsp/gst-rtsp-cam-media-factory.c \
	$(top_srcdir)/src/rtsp/gst-rtsp-cam-media-factory.h \
	gst-rtsp-cam.c

gst_rtsp_cam_CFLAGS = $(GST_CFLAGS) $(GST_RTSP_SERVER_CFLAGS)
gst_rtsp_cam_LDADD = $(GST_LIBS) $(GST_RTSP_SERVER_LIBS)
gst_rtsp_cam_LDFLAGS = -avoid-version -no-undefined -dynamic
INCLUDES = -I$(top_srcdir)/src/rtsp
