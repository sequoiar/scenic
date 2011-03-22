SUBDIRS=examples

pkglib_LTLIBRARIES = librtsp.la
librtsp_la_SOURCES = gst-rtsp-cam-media-factory.c gst-rtsp-cam-media-factory.h RTSPServer.cpp RTSPServer.h RTSPClient.cpp RTSPClient.h
AM_CXXFLAGS = $(GST_CFLAGS) $(GST_RTSP_CFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/src/include $(GTK_CFLAGS)
AM_CFLAGS = $(GST_CFLAGS) $(GST_RTSP_CFLAGS)
