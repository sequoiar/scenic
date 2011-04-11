lib_LTLIBRARIES = libshared_video.la
 # public header file 
shared_videodir = $(includedir)/shared-video-@SHAREDVIDEO_API_VERSION@
shared_video_HEADERS = \
    $(top_srcdir)/src/include/sharedVideoBuffer.h \
    $(top_srcdir)/src/include/videoSize.h

libshared_video_la_SOURCES = sharedVideoBuffer.cpp $(top_srcdir)/src/include/sharedVideoBuffer.h
libshared_video_la_CPPFLAGS = -I$(top_srcdir)/src/include

