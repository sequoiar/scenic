SUBDIRS=examples

pkglib_LTLIBRARIES = librtsp.la
librtsp_la_SOURCES = gst-rtsp-cam-media-factory.c gst-rtsp-cam-media-factory.h rtsp_server.cpp rtsp_server.h rtsp_client.cpp rtsp_client.h
AM_CXXFLAGS = $(GST_CFLAGS) $(GST_RTSP_CFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/src/include $(GTK_CFLAGS)
AM_CFLAGS = $(GST_CFLAGS) $(GST_RTSP_CFLAGS)
